#include <cassert>
#include "../common/random.hpp"
#include "../common/sha256.hpp"
#include "../common/sha512.hpp"
#include "../common/pbkdf2.hpp"
#include "../common/hmac.hpp"
#include "mnemonic.hpp"
#include "keys.hpp"
#include <unordered_map>

using namespace prologcoin::common;

namespace prologcoin { namespace ec {

#include "bip39_english_word_list.hpp"

static std::unordered_map<std::string, size_t> word_2_index;

bool mnemonic::is_valid_word(term t)
{
    ensure_reverse_word_map();
    if (t.tag() != tag_t::CON) {
        return false;
    }
    con_cell f = env_.functor(t);
    if (f.arity() != 0) {
        return false;
    }
    std::string word = env_.atom_name(f);
    if (word_2_index.find(word) == word_2_index.end()) {
        return false;
    }

    return true;
}
    
void mnemonic::generate_new(size_t ent)
{
    assert(ent % 32 == 0);
    assert(ent >= 128 && ent <= 256);
    ent_ = ent;
    random::next_bytes(bytes_, ent/8);
    add_checksum();
}

void mnemonic::set(const uint8_t *bytes, size_t n)
{
    size_t ent = n*8;
    assert(ent % 32 == 0);
    assert(ent >= 128 && ent <= 256);
    ent_ = ent;
    std::copy(bytes, &bytes[n], bytes_);
    add_checksum();
}

void mnemonic::add_checksum()
{
    uint8_t checksum[sha256::HASH_SIZE];
    sha256 hash;
    hash.update(bytes_, ent_/8);
    hash.finalize(checksum);
    size_t checksum_num_bits = ent_/32;
    bytes_[ent_/8] = checksum[0] & (0xff << (8 - checksum_num_bits));
}

bool mnemonic::check_checksum()
{
    uint8_t checksum[sha256::HASH_SIZE];
    sha256 hash;
    hash.update(bytes_, ent_/8);
    hash.finalize(checksum);
    size_t checksum_num_bits = ent_/32;
    uint8_t expect = checksum[0] & (0xff << (8 - checksum_num_bits));
    uint8_t actual = bytes_[ent_/8] & (0xff << (8 - checksum_num_bits));
    return expect == actual;
}

void mnemonic::ensure_reverse_word_map()
{
    if (word_2_index.empty()) {
	for (size_t i = 0; i < 2048; i++) {
	    word_2_index[BIP39_ENGLISH_WORD_LIST[i]] = i;
	}
    }
}

bool mnemonic::from_sentence(term words)
{
    ensure_reverse_word_map();
    size_t off = 0;
    memset(bytes_, 0, 32+1);
    ent_ = 0;
    while (words != term_env::EMPTY_LIST) {
	auto word = env_.arg(words, 0);
	std::string word_str = env_.to_string(word);
	if (word_2_index.find(word_str) == word_2_index.end()) {
	    return false;
	}
	size_t word_index = word_2_index[word_str];
	size_t n = 0;
	while (n < 11) {
	    size_t i1 = off % 8;
	    size_t n1 = 8 - i1;
	    if (n+n1 > 11) n1 = 11 - n;
	    bytes_[off / 8] |= (word_index >> (11 - n1)) << (8-i1-n1);
	    word_index <<= n1;
	    n += n1;
	    off += n1;
	}
	ent_ += 11;
	
	words = env_.arg(words, 1);
    }

    ent_ -= (ent_ / 11) / 3;

    if (!check_checksum()) {
	return false;
    }

    return true;
}

void mnemonic::to_integers(std::vector<size_t> &ints)
{
    size_t checksum_len = (ent_/32);    
    size_t num_bits = ent_+checksum_len;
    for (size_t i = 0; i < num_bits; i += 11) {
	size_t off = i;
	size_t n = 0;
	size_t word_index = 0;
	while (n < 11) {
	    size_t i1 = off % 8;
	    size_t n1 = 8 - i1;
	    if (n + n1 > 11) n1 = 11 - n;
	    word_index <<= n1;
	    word_index |= ((bytes_[off / 8] & (0xff >> i1)) >> (8 - i1 - n1));

	    n += n1;
	    off += n1;
	}
	ints.push_back(word_index);
    }
}

term mnemonic::to_sentence()
{
    std::vector<size_t> ints;
    to_integers(ints);
    term lst = term_env::EMPTY_LIST;
    for (auto word_index : boost::adaptors::reverse(ints)) {
	lst = env_.new_dotted_pair(env_.functor(BIP39_ENGLISH_WORD_LIST[word_index],0),lst);
    }
    return lst;
}

std::string mnemonic::to_sentence_string()
{
    std::string str;
    std::vector<size_t> ints;
    to_integers(ints);
    for (auto word_index : ints) {
	if (!str.empty()) str += " ";
	str += BIP39_ENGLISH_WORD_LIST[word_index];
    }
    return str;
}

void mnemonic::compute_key(hd_keys &hd, const std::string &passphrase)
{
    uint8_t seed[64];
    compute_seed(seed, passphrase);
    hd.set_seed(seed, 64);
}

void mnemonic::compute_seed(uint8_t seed[64], const std::string &passphrase)
{
    std::string salt = "mnemonic";
    salt += passphrase;
    pbkdf2_t<hmac<sha512> > pbk(salt.c_str(), salt.size(), 2048, 64);
    std::string passwd = to_sentence_string();
    pbk.set_password(passwd.c_str(), passwd.size());
    const uint8_t *seed1 = pbk.get_key();
    std::copy(seed1, &seed1[64], seed);
}
    
}}
