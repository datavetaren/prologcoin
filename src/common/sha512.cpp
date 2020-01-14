#include <iostream>
#include <boost/endian/conversion.hpp>
#include "sha512.hpp"

namespace prologcoin { namespace common {

const uint8_t sha512::PADDING[128] = {
    0x80,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0
};

const uint64_t sha512::K[80] = {
  0x428A2F98D728AE22,0x7137449123EF65CD,0xB5C0FBCFEC4D3B2F,0xE9B5DBA58189DBBC,
  0x3956C25BF348B538,0x59F111F1B605D019,0x923F82A4AF194F9B,0xAB1C5ED5DA6D8118,
  0xD807AA98A3030242,0x12835B0145706FBE,0x243185BE4EE4B28C,0x550C7DC3D5FFB4E2,
  0x72BE5D74F27B896F,0x80DEB1FE3B1696B1,0x9BDC06A725C71235,0xC19BF174CF692694,
  0xE49B69C19EF14AD2,0xEFBE4786384F25E3,0x0FC19DC68B8CD5B5,0x240CA1CC77AC9C65,
  0x2DE92C6F592B0275,0x4A7484AA6EA6E483,0x5CB0A9DCBD41FBD4,0x76F988DA831153B5,
  0x983E5152EE66DFAB,0xA831C66D2DB43210,0xB00327C898FB213F,0xBF597FC7BEEF0EE4,
  0xC6E00BF33DA88FC2,0xD5A79147930AA725,0x06CA6351E003826F,0x142929670A0E6E70,
  0x27B70A8546D22FFC,0x2E1B21385C26C926,0x4D2C6DFC5AC42AED,0x53380D139D95B3DF,
  0x650A73548BAF63DE,0x766A0ABB3C77B2A8,0x81C2C92E47EDAEE6,0x92722C851482353B,
  0xA2BFE8A14CF10364,0xA81A664BBC423001,0xC24B8B70D0F89791,0xC76C51A30654BE30,
  0xD192E819D6EF5218,0xD69906245565A910,0xF40E35855771202A,0x106AA07032BBD1B8,
  0x19A4C116B8D2D0C8,0x1E376C085141AB53,0x2748774CDF8EEB99,0x34B0BCB5E19B48A8,
  0x391C0CB3C5C95A63,0x4ED8AA4AE3418ACB,0x5B9CCA4F7763E373,0x682E6FF3D6B2B8A3,
  0x748F82EE5DEFB2FC,0x78A5636F43172F60,0x84C87814A1F0AB72,0x8CC702081A6439EC,
  0x90BEFFFA23631E28,0xA4506CEBDE82BDE9,0xBEF9A3F7B2C67915,0xC67178F2E372532B,
  0xCA273ECEEA26619C,0xD186B8C721C0C207,0xEADA7DD6CDE0EB1E,0xF57D4F7FEE6ED178,
  0x06F067AA72176FBA,0x0A637DC5A2C898A6,0x113F9804BEF90DAE,0x1B710B35131C471B,
  0x28DB77F523047D84,0x32CAAB7B40C72493,0x3C9EBE0A15C9BEBC,0x431D67C49C100D4C,
  0x4CC5D4BECB3E42B6,0x597F299CFC657E2A,0x5FCB6FAB3AD6FAEC,0x6C44198C4A475817
 };    
   
void sha512::init() {
    h_[0] = 0x6A09E667F3BCC908;
    h_[1] = 0xBB67AE8584CAA73B;
    h_[2] = 0x3C6EF372FE94F82B;
    h_[3] = 0xA54FF53A5F1D36F1;
    h_[4] = 0x510E527FADE682D1;
    h_[5] = 0x9B05688C2B3E6C1F;
    h_[6] = 0x1F83D9ABFB41BD6B;
    h_[7] = 0x5BE0CD19137E2179;
    size_ = 0;
    total_size_ = 0;
}

void sha512::update(const void *p, size_t len)
{
    auto data = reinterpret_cast<const uint8_t *>(p);
    while (len > 0) {
        size_t n = std::min(len, 128 - size_);
	std::copy(data, data+n, &buffer_[size_]);
	size_ += n;
	total_size_ += n;
	data += n;
	len -= n;
	if (size_ == BLOCK_SIZE) {
	    block();
	    size_ = 0;
	}
    }
}

void sha512::finalize(uint8_t digest[HASH_SIZE])
{
    size_t pad_size = (size_ < 112) ? 112 - size_ : 128 + 112 - size_;
    uint64_t total_size_bits = total_size_ * 8; // Length of message in bits
    update(PADDING, pad_size);
    w_[14] = 0;
    w_[15] = boost::endian::native_to_big(total_size_bits);
    block();
    for (size_t i = 0; i < 8; i++) {
        h_[i] = boost::endian::native_to_big(h_[i]);
    }
    std::copy(&digest_[0], &digest_[HASH_SIZE], digest);
}

void sha512::block()
{
    uint64_t t1, t2;

    uint64_t a = h_[0], b = h_[1], c = h_[2], d = h_[3];
    uint64_t e = h_[4], f = h_[5], g = h_[6], h = h_[7];

    for (size_t i = 0; i < 16; i++) {
        w_[i] = boost::endian::big_to_native(w_[i]);
    }

#define W(i) w_[(i) & 0xf]
#define ROR64(a, n) (((a) >> (n)) | ((a) << (64 - (n))))
#define SHR64(a, n) ((a) >> (n))
#define CH(x,y,z) (((x) & (y)) | (~(x) & (z)))
#define MAJ(x,y,z) (((x) & (y)) | ((x) & (z)) | ((y) & (z)))
#define SIGMA1(x) (ROR64(x,28) ^ ROR64(x,34) ^ ROR64(x, 39))
#define SIGMA2(x) (ROR64(x,14) ^ ROR64(x,18) ^ ROR64(x, 41))
#define SIGMA3(x) (ROR64(x,1) ^ ROR64(x,8) ^ SHR64(x, 7))
#define SIGMA4(x) (ROR64(x,19) ^ ROR64(x,61) ^ SHR64(x, 6))
    
    for (size_t i = 0; i < 80; i++) {
        if (i >= 16) {
	    W(i) += SIGMA4(W(i+14)) + W(i+9) + SIGMA3(W(i+1));
	}
	t1 = h + SIGMA2(e) + CH(e,f,g) + K[i] + W(i);
	t2 = SIGMA1(a) + MAJ(a,b,c);

	h = g; g = f; f = e; e = d+t1; d = c; c = b; b = a; a = t1+t2;
    }
    h_[0] += a; h_[1] += b; h_[2] += c; h_[3] += d;
    h_[4] += e; h_[5] += f; h_[6] += g; h_[7] += h;
}
    
}}
