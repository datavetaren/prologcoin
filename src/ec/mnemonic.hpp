#pragma once

#ifndef _ec_mnemonic_hpp
#define _ec_mnemonic_hpp

// Implements BIP39

#include "../common/term_env.hpp"
#include "keys.hpp"

namespace prologcoin { namespace ec {

class mnemonic {
public:
    inline mnemonic(common::term_env &env) : env_(env) { }

    void generate_new(size_t ent);
    void set(const uint8_t *bytes, size_t n);
    void to_integers(std::vector<size_t> &ints);
    common::term to_sentence();
    std::string to_sentence_string();
    bool from_sentence(common::term atoms);
    common::term atom_list();
    void compute_key(hd_keys &key, const std::string &passphrase = "");
    void compute_seed(uint8_t seed[64], const std::string &passphrase = "");
    bool is_valid_word(common::term word);
  
private:
    void ensure_reverse_word_map();
    bool check_checksum();
    void add_checksum();
    common::term_env &env_;
    uint8_t bytes_[32+1];
    size_t ent_;
};

}}

#endif
