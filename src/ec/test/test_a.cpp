#include "../../common/hex.hpp"
#include "../builtins.hpp"

using namespace prologcoin::common;
using namespace prologcoin::ec;

int main(int argc, char *argv[])
{
    uint8_t xxx[8] = { 1, 2, 10, 0x40, 0x50, 0x70, 0xf0, 0x67 };

    std::cout << "HELLO: " << hex::to_string(xxx,8) << std::endl;

    builtins::pedersen_test();

    return 0;
}
