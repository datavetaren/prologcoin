#include <fstream>
#include "file_stream.hpp"
#include "../common/term_tokenizer.hpp"
#include "../common/term_parser.hpp"

namespace prologcoin { namespace interp {

using namespace prologcoin::common;

file_stream::~file_stream()
{
    if (parser_) {
        delete parser_;
    }
    if (tokenizer_) {
        delete tokenizer_;
    }
    if (in_) {
        delete in_;
    }
}

size_t file_stream::get_id() const
{
    return id_;
}

void file_stream::open(mode_t mode)
{
    if (mode_ != NONE) {
        // File is already open...
        return;
    }

    assert(mode == READ);

    in_ = new std::ifstream(path_);
    mode_ = mode;
}

void file_stream::close()
{
    if (parser_) {
        delete parser_;
	parser_ = nullptr;
    }
    if (tokenizer_) {
        delete tokenizer_;
	tokenizer_ = nullptr;
    }
    if (in_) {
        delete in_;
	in_ = nullptr;
    }
}

term file_stream::read_term()
{
    ensure_parser();

    term r = parser_->parse();

    parser_->for_each_var_name( [&](const term  &ref,
				   const std::string &name)
				{ env_.set_name(ref,name); } );
    parser_->clear_var_names();

    return r;
}

void file_stream::ensure_parser()
{
    if (mode_ != READ) {
        return;
    }

    if (tokenizer_ == nullptr) {
        tokenizer_ = new term_tokenizer(*in_);
    }
    if (parser_ == nullptr) {
	parser_ = new term_parser(*tokenizer_, env_);
    }
}

}}
