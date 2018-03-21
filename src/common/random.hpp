#pragma once

#ifndef _common_random_hpp
#define _common_random_hpp

#include <string>

namespace prologcoin { namespace common {

class random {
public:
    static std::string next(size_t entropy_bits = 128);
    static int next_int(int max);
};

}}

#endif
