#include <cassert>
#include "../common/random.hpp"
#include "../common/sha256.hpp"
#include "mnemonic.hpp"
#include "keys.hpp"

using namespace prologcoin::common;

namespace prologcoin { namespace ec {

#include "bip39_english_word_list.hpp"

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
    bytes_[ent_/8] = checksum[0];
}

term mnemonic::to_sentence()
{
    size_t bit_index = 0;
    size_t checksum_len = (ent_/32);    
    size_t num_bits = ent_+checksum_len;
    size_t start_bit_index = num_bits - 11;
    std::cout << "num_bits: " << num_bits << std::endl;
    std::cout << "checksum_len: " << checksum_len << std::endl;
    std::cout << "start index: " << start_bit_index << std::endl;
    return term_env::EMPTY_LIST;
}
    
}}
