#pragma once

#ifndef _common_random_hpp
#define _common_random_hpp

#include <string>

namespace prologcoin { namespace common {

class random {
public:
    static std::string next(size_t entropy_bits = 128);
    static int next_int(int max);

    static void set_for_testing(bool for_testing);

private:
    static bool for_testing_;
};

}}

#endif
