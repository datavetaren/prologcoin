#pragma once

#ifndef _ec_mnemonic_hpp
#define _ec_mnemonic_hpp

// Implements BIP39

#include "../common/term_env.hpp"

namespace prologcoin { namespace ec {

class mnemonic {
public:
    inline mnemonic(common::term_env &env) : env_(env) { }

    void generate_new(size_t ent);
    void set(const uint8_t *bytes, size_t n);
    common::term to_sentence();
    void from_sentence(common::term atoms);
    common::term atom_list();
  
private:
    void add_checksum();
    common::term_env &env_;
    uint8_t bytes_[32+1];
    size_t ent_;
};

}}

#endif
