#include <cassert>
#include "../secp256k1/secp256k1.hpp"
#include "../common/random.hpp"
#include "../common/sha512.hpp"
#include "../common/hmac.hpp"
#include "../common/sha1.hpp"
#include "keys.hpp"
#include "ripemd160.h"
#include "builtins.hpp"

using namespace prologcoin::common;

namespace prologcoin { namespace ec {

secp256k1_ctx::secp256k1_ctx() : secp256k1_ctx(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY) { }
    
secp256k1_ctx::secp256k1_ctx(unsigned int flags) {
    ctx_ = secp256k1_context_create(flags);
    scratch_ = secp256k1_scratch_space_create(ctx_, 1024*1024);
}

secp256k1_ctx::~secp256k1_ctx() {
    secp256k1_scratch_space_destroy(ctx_, scratch_);
    secp256k1_context_destroy(ctx_);
}

void checksum(const uint8_t *in, size_t len, uint8_t out[4])
{
    builtins::get_checksum(in, len, out);
  
  /*
    sha1 hash;
    hash.update(in, len);
    uint8_t temp[sha1::HASH_SIZE];
    hash.finalize(temp);
    hash.init(nullptr, 0);
    hash.update(temp, sha1::HASH_SIZE);
    hash.finalize(temp);
    std::copy(temp, &temp[4], out);
  */
}
    
public_key::public_key()
{
    memset(&bytes_[0], 0, SIZE);
}

void public_key::compute_fingerprint(uint8_t fingerprint[4]) const {
    sha1 hash1;
    hash1.update(&bytes_[0], SIZE);

    uint8_t hash2[sha1::HASH_SIZE];
    hash1.finalize(hash2);

    uint8_t hash[20];
    ripemd160(hash2, sha1::HASH_SIZE, hash);

    std::copy(hash, &hash[4], fingerprint);
}
    
private_key::private_key(private_key::key_create_t k, secp256k1_ctx &ctx)
{
    if (k == NEW_KEY) {
        create_new(ctx);
    } else {
        assert(k == NO_KEY);
	memset(&bytes_[0], 0, SIZE);
    }
}

void private_key::create_new(secp256k1_ctx &ctx)
{
    bool ok = true;
    do {
        random::next_bytes(&bytes_[0], SIZE);
	ok = secp256k1_ec_seckey_verify(ctx, &bytes_[0]);
    } while (!ok);
}

bool private_key::compute_public_key(secp256k1_ctx &ctx, public_key &pubkey) const
{
    secp256k1_pubkey pubkey0;
    bool r = secp256k1_ec_pubkey_create(ctx, &pubkey0, &(*this)[0]);
    if (!r) {
	return false;
    }
    size_t pubkey_len = public_key::SIZE;
    r = secp256k1_ec_pubkey_serialize(ctx, &pubkey[0], &pubkey_len,
				      &pubkey0, SECP256K1_EC_COMPRESSED);
    if (!r) {
	return false;
    }
    if (pubkey_len != public_key::SIZE) {
	return false;
    }
    return true;
}    

void extended_public_key::write(uint8_t data[78]) const
{
    data[0] = 0x04;
    data[1] = 0x88;
    data[2] = 0xB2;
    data[3] = 0x1E;
    data[4] = static_cast<uint8_t>(level());
    std::copy(fingerprint(), fingerprint()+4, &data[5]);
    data[9] = static_cast<uint8_t>((child_number() >> 24) & 0xff);
    data[10] = static_cast<uint8_t>((child_number() >> 16) & 0xff);
    data[11] = static_cast<uint8_t>((child_number() >> 8) & 0xff);
    data[12] = static_cast<uint8_t>((child_number() >> 0) & 0xff);
    std::copy(&get_chain_code()[0], &get_chain_code()[32], &data[13]);
    std::copy(&(*this)[0], &(*this)[public_key::SIZE], &data[45]);

    std::cout << "WRITE EXT PUBKEY: " << hex::to_string(data, 78) << std::endl;
}

bool extended_public_key::read(uint8_t data[78])
{
    if (data[0] != 0x04) return false;
    if (data[1] != 0x88) return false;
    if (data[2] != 0xB2) return false;
    if (data[3] != 0x1E) return false;
    set_level(data[4]);
    set_fingerprint(&data[5]);
    set_child_number((static_cast<uint32_t>(data[9]) << 24) |
		     (static_cast<uint32_t>(data[10]) << 16) |
		     (static_cast<uint32_t>(data[11]) << 8) |
		     (static_cast<uint32_t>(data[12])));
    std::copy(&data[13], &data[13+32], &get_chain_code()[0]);
    std::copy(&data[45], &data[45+32], &(*this)[0]);
    return true;
}

std::string extended_public_key::to_string() const {
    uint8_t bytes[78+4];
    write(bytes);
    checksum(&bytes[0], 78, &bytes[78]);
    
    boost::multiprecision::cpp_int big;
    import_bits(big, bytes, bytes+78+4);
    return heap::big_to_string(big, 58, (78+3)*8);
}
    
void extended_private_key::set_from_hash(const uint8_t hash[64]) {
    std::copy(&hash[0], &hash[32], &(*this)[0]);
    std::copy(&hash[32], &hash[64], &get_chain_code()[0]);
}

void extended_private_key::write(uint8_t data[78]) const
{
    data[0] = 0x04;
    data[1] = 0x88;
    data[2] = 0xAD;
    data[3] = 0xE4;
    data[4] = static_cast<uint8_t>(level());
    std::copy(fingerprint(), fingerprint()+4, &data[5]);
    data[9] = static_cast<uint8_t>((child_number() >> 24) & 0xff);
    data[10] = static_cast<uint8_t>((child_number() >> 16) & 0xff);
    data[11] = static_cast<uint8_t>((child_number() >> 8) & 0xff);
    data[12] = static_cast<uint8_t>((child_number() >> 0) & 0xff);
    std::copy(&get_chain_code()[0], &get_chain_code()[32], &data[13]);
    data[45] = 0;
    std::copy(&(*this)[0], &(*this)[private_key::SIZE], &data[46]);

    std::cout << "WRITE EXT PRIVKEY: " << hex::to_string(data, 78) << std::endl;    
}

std::string extended_private_key::to_string() const {
    uint8_t bytes[78+4];
    write(bytes);

    std::cout << "CHECKSUM: " << hex::to_string(&bytes[78], 4) << std::endl;
    
    checksum(&bytes[0], 78, &bytes[78]);

    std::cout << "CHECKSUM: " << hex::to_string(&bytes[78], 4) << std::endl;
    
    boost::multiprecision::cpp_int big;
    import_bits(big, bytes, bytes+(78+4), 8);
    return heap::big_to_string(big, 58, (78+3)*8);
}

hd_keys::hd_keys(secp256k1_ctx &ctx, const uint8_t *seed, size_t seed_len)
  : ctx_(ctx), master_(ctx)
{
    const char *key = "Bitcoin seed";
  
    hmac<sha512> hasher;
    hasher.init(key, strlen(key));
    hasher.update(seed, seed_len);

    uint8_t hash[64];
    hasher.finalize(hash);

    master_.set_from_hash(hash);

    public_key pubkey;
    master_.compute_public_key(ctx, pubkey);
    master_public_.set_public_key(pubkey);
    master_public_.set_chain_code(master_.get_chain_code());
    master_public_.set_fingerprint(master_.fingerprint());
    master_public_.set_child_number(0);
    master_public_.set_level(0);
}

bool hd_keys::generate_child(const extended_public_key &parent, size_t index, extended_public_key &out)
{
    assert(index < extended_key::HARDENED_CHILD);
    out = parent;
    out.set_level(parent.level()+1);
    uint8_t fingerprint[4];
    parent.compute_fingerprint(fingerprint);
    out.set_fingerprint(fingerprint);
    out.set_child_number(index);

    hmac<sha512> hasher;
    hasher.init(&parent.get_chain_code()[0], 32);
    hasher.update(&parent[0], public_key::SIZE);

    uint8_t i32[4];
    i32[0] = static_cast<uint8_t>((index >> 24) & 0xff);
    i32[1] = static_cast<uint8_t>((index >> 16) & 0xff);
    i32[2] = static_cast<uint8_t>((index >> 8) & 0xff);
    i32[3] = static_cast<uint8_t>((index >> 0) & 0xff);    
    
    hasher.update(i32, 4);

    uint8_t I[64];
    hasher.finalize(I);

    uint8_t *IL = &I[0];
    uint8_t *IR = &I[32];

    // First get the parent key public key
    secp256k1_pubkey pubkey;
    if (secp256k1_ec_pubkey_parse(ctx_, &pubkey, &parent[0], 33) != 1) {
	return false;
    }

    // Add IL*G to this public key to get the child public key
    if (secp256k1_ec_pubkey_tweak_add(ctx_, &pubkey, IL) != 1) {
        return false;
    }

    size_t pubkey_len = public_key::SIZE;
    int r = secp256k1_ec_pubkey_serialize(ctx_, &out[0], &pubkey_len, &pubkey,
					  SECP256K1_EC_COMPRESSED);
    if (r != 1 || pubkey_len != public_key::SIZE) {
        return false;
    }

    out.set_chain_code(IR);

    return true;
}

}}
