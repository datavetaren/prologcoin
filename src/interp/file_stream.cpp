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
    if (emitter_) {
	delete emitter_;
    }
    if (in_ && in_owner_) {
        delete in_;
    }
    if (out_ && out_owner_) {
	delete out_;
    }
}

size_t file_stream::get_id() const
{
    return id_;
}

bool file_stream::is_eof()
{
    if (in_) {
	ensure_parser();
	return parser_->is_eof();
    } else {
	return false;
    }
}

void file_stream::open(mode_t mode)
{
    if (mode_ != NONE) {
        // File is already open...
        return;
    }

    mode_ = mode;
    switch (mode) {
    case READ:
	in_ = new std::ifstream(path_); in_owner_ = true; break;
    case WRITE:
	out_ = new std::ofstream(path_); out_owner_ = true; break;
    default:
	assert(mode == READ || mode == WRITE);
    }
}

void file_stream::open(std::ostream &out)
{
    mode_ = WRITE;
    out_ = &out;
    out_owner_ = false;
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
    if (emitter_) {
	delete emitter_;
	emitter_ = nullptr;
    }
    if (in_ && in_owner_) {
        delete in_;
    }
    in_ = nullptr;
    if (out_ && out_owner_) {
	delete out_;
    }
    out_ = nullptr;
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

void file_stream::ensure_emitter()
{
    if (mode_ != WRITE) {
	return;
    }

    if (emitter_ == nullptr) {
	emitter_ = new term_emitter(*out_, env_, env_);
	emitter_->options().clear(emitter_option::EMIT_QUOTED);
	emitter_->options().clear(emitter_option::EMIT_NEWLINE);
        emitter_->set_var_naming(env_.var_naming());
    }
}

void file_stream::write_term(const term t)
{
    ensure_emitter();

    emitter_->reset();
    emitter_->print(t);
}

void file_stream::write(const std::string &s)
{
    *out_ << s;
}

void file_stream::nl()
{
    *out_ << std::endl;
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
