#pragma once

#ifndef _interp_file_stream_hpp
#define _interp_file_stream_hpp

#include <fstream>
#include "../common/term.hpp"
#include "../common/term_env.hpp"
#include "../common/term_tokenizer.hpp"
#include "../common/term_parser.hpp"

namespace prologcoin { namespace interp {

    class file_stream {
    public:
        enum mode_t { NONE, READ, WRITE }; // No support for READ+WRITE

        file_stream(common::term_env &env, size_t id, const std::string &path)
	  : env_(env), id_(id), path_(path), in_(nullptr), mode_(NONE),
            tokenizer_(nullptr), parser_(nullptr) { }

        ~file_stream();

        void open(mode_t mode = READ);
        void close();
        size_t get_id() const;
        bool is_eof();

        common::term read_term();

    private:
        void ensure_parser();

        common::term_env &env_;
        
        size_t id_;
        std::string path_;
        std::ifstream *in_;
        mode_t mode_;
        common::term_tokenizer *tokenizer_;
        common::term_parser *parser_;
    };

}}

#endif
