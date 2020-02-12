#include <cassert>
#include "../secp256k1/secp256k1.hpp"
#include "../common/random.hpp"
#include "../common/sha256.hpp"
#include "../common/sha512.hpp"
#include "../common/hmac.hpp"
#include "../common/sha1.hpp"
#include "../common/ripemd160.hpp"
#include "keys.hpp"
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
    sha256 hash;
    hash.update(in, len);
    uint8_t temp[sha256::HASH_SIZE];
    hash.finalize(temp);
    hash.init();
    hash.update(temp, sha256::HASH_SIZE);
    hash.finalize(temp);
    std::copy(temp, &temp[4], out);
}
    
public_key::public_key()
{
    memset(&bytes_[0], 0, SIZE);
}

void public_key::compute_fingerprint(uint8_t out[4]) const {
    sha256 sha;
    sha.update(&bytes_[0], SIZE);

    uint8_t hash[sha256::HASH_SIZE];
    sha.finalize(hash);
    uint8_t hash2[32];
    ripemd160(hash, sha256::HASH_SIZE, hash2);

    std::copy(hash2, &hash2[4], out);
}

private_key::private_key(private_key::key_create_t k)
{
    assert(k == NO_KEY);
    memset(&bytes_[0], 0, SIZE);
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
    std::copy(&data[45], &data[45+33], &(*this)[0]);
    return true;
}

std::string extended_public_key::to_string() const {
    uint8_t bytes[78+4];
    write(bytes);

    checksum(&bytes[0], 78, &bytes[78]);
    
    boost::multiprecision::cpp_int big;
    import_bits(big, bytes, bytes+(78+4), 8);
    return heap::big_to_string(big, 58, (78+4)*8);
}

term extended_public_key::to_term(term_env &env) const {
    static const size_t N = 78+4;
    uint8_t bytes[N];
    write(bytes);
    checksum(&bytes[0], 78, &bytes[78]);
    term big = env.new_big(N*8);
    env.set_big(big, &bytes[0], N);
    return big;
}
    
void extended_private_key::set_from_hash(const uint8_t hash[64]) {
    std::copy(&hash[0], &hash[32], &(*this)[0]);
    std::copy(&hash[32], &hash[64], &get_chain_code()[0]);
}

void extended_private_key::compute_extended_public_key(secp256k1_ctx &ctx, extended_public_key &extpub){
    public_key pubkey;
    compute_public_key(ctx, pubkey);
    extpub.set_public_key(pubkey);
    extpub.set_chain_code(get_chain_code());
    extpub.set_fingerprint(fingerprint());
    extpub.set_child_number(child_number());
    extpub.set_level(level());
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
}

bool extended_private_key::read(uint8_t data[78])
{
    if (data[0] != 0x04) return false;
    if (data[1] != 0x88) return false;
    if (data[2] != 0xAD) return false;
    if (data[3] != 0xE4) return false;
    set_level(data[4]);
    set_fingerprint(&data[5]);
    set_child_number((static_cast<uint32_t>(data[9]) << 24) |
		     (static_cast<uint32_t>(data[10]) << 16) |
		     (static_cast<uint32_t>(data[11]) << 8) |
		     (static_cast<uint32_t>(data[12])));
    std::copy(&data[13], &data[13+32], &get_chain_code()[0]);
    if (data[45] != 0) return false;
    std::copy(&data[46], &data[46+32], &(*this)[0]);
    return true;
}

std::string extended_private_key::to_string() const {
    uint8_t bytes[78+4];
    write(bytes);

    checksum(&bytes[0], 78, &bytes[78]);
    
    boost::multiprecision::cpp_int big;
    import_bits(big, bytes, bytes+(78+4), 8);
    return heap::big_to_string(big, 58, (78+4)*8);
}

term extended_private_key::to_term(term_env &env) const {
    static const size_t N = 78+4;
    uint8_t bytes[N];
    write(bytes);
    checksum(&bytes[0], 78, &bytes[78]);
    term big = env.new_big(N*8);
    env.set_big(big, &bytes[0], N);
    return big;
}    

hd_keys::hd_keys(secp256k1_ctx &ctx)
  : ctx_(ctx), master_(ctx) {
}
    
hd_keys::hd_keys(secp256k1_ctx &ctx, const uint8_t *seed, size_t seed_len)
  : ctx_(ctx), master_(ctx)
{
    set_seed(seed, seed_len);
}

void hd_keys::set_seed(const uint8_t *seed, size_t seed_len)
{
    const char *key = "Bitcoin seed";
  
    hmac<sha512> hasher;
    hasher.init(key, strlen(key));
    hasher.update(seed, seed_len);

    uint8_t hash[64];
    hasher.finalize(hash);

    master_.set_from_hash(hash);

    master_.compute_extended_public_key(ctx_, master_public_);
}

bool hd_keys::generate_child(const extended_public_key &parent, uint32_t index, extended_public_key &out)
{
    assert(index < extended_key::HARDENED_KEY);
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

bool hd_keys::generate_child(const extended_private_key &parent, uint32_t index, extended_private_key &out)
{
    out.set_level(parent.level()+1);
    uint8_t fingerprint[4];
    public_key parent_pubkey;
    parent.compute_public_key(ctx_, parent_pubkey);
    parent_pubkey.compute_fingerprint(fingerprint);
    out.set_fingerprint(fingerprint);
    out.set_child_number(index);

    hmac<sha512> hasher;
    hasher.init(&parent.get_chain_code()[0], 32);
    if (index >= extended_key::HARDENED_KEY) {
	uint8_t zero = 0;
	hasher.update(&zero, 1);
	hasher.update(&parent[0], private_key::SIZE);
    } else {
	hasher.update(&parent_pubkey[0], public_key::SIZE);
    }
    
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

    // Add IL to this public key to get the child public key
    if (secp256k1_ec_privkey_tweak_add(ctx_, IL, &parent[0]) != 1) {
        return false;
    }

    std::copy(IL, IL+32, &out[0]);

    out.set_chain_code(IR);

    return true;
}

bool hd_keys::generate_child(const extended_private_key &parent, uint32_t index, extended_public_key &out)
{
    extended_private_key priv_child;
    generate_child(parent, index, priv_child);
    
    public_key pub_child;
    priv_child.compute_public_key(ctx_, pub_child);
    out.set_level(priv_child.level());
    uint8_t fingerprint[4];
    public_key pub_parent;
    parent.compute_public_key(ctx_, pub_parent);
    pub_parent.compute_fingerprint(fingerprint);
    out.set_fingerprint(fingerprint);
    out.set_chain_code(priv_child.get_chain_code());
    out.set_child_number(priv_child.child_number());
    out.set_public_key(pub_child);

    return true;
}


}}
