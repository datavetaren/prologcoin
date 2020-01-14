#include "../../common/hex.hpp"
#include "../../common/term.hpp"
#include "../../common/sha1.hpp"
#include "../keys.hpp"

using namespace prologcoin::common;
using namespace prologcoin::ec;

int main(int argc, char *argv[])
{
    const uint8_t SEED[16] = {0x0,0x1,0x2,0x3,0x4,0x5,0x6,0x7,
  		  	      0x8,0x9,0xa,0xb,0xc,0xd,0xe,0xf};
  
    secp256k1_ctx ctx;
    hd_keys hd(ctx, SEED, sizeof(SEED));

    std::cout << "MASTER PRIVKEY: " << hd.master_private().to_string() << std::endl;

    return 0;
}
