#include "term_tools.hpp"
#include "term_tokenizer.hpp"

namespace prologcoin { namespace common {

term_token_diff::term_token_diff(std::istream &in1, std::istream &in2)
    : line1_no_(-1), line2_no_(-1), tokens1_(in1), tokens2_(in2),
      done_(false), failed_(false) { }

void term_token_diff::skip_whitespace(term_tokenizer &tokens,
				      int &line_no,
				      std::string &line)
{
    // Skip white spaces on stream 1
    while (tokens.has_more_tokens() &&
	   tokens.peek_token().type() ==
	   term_tokenizer::TOKEN_LAYOUT_TEXT) {
	const term_tokenizer::token &token = tokens.peek_token();
	if (token.pos().line() != line_no) {
	    line_no = token.pos().line();
	    line.clear();
	}
	line += token.lexeme();
	tokens.consume_token();
    }
}

bool term_token_diff::check(const std::string &s1, const std::string &s2)
{
    std::stringstream in1(s1);
    std::stringstream in2(s2);
    term_token_diff diff(in1, in2);
    return diff.check();
}

bool term_token_diff::check()
{
    if (done_) {
	return !failed_;
    }
    bool r = check_impl();
    done_ = true;
    if (!r) {
	failed_ = true;
    }
    return r;
}

bool term_token_diff::check_impl()
{
    while (tokens1_.has_more_tokens() && tokens2_.has_more_tokens()) {
	
	skip_whitespace(tokens1_, line1_no_, line1_);
	skip_whitespace(tokens2_, line2_no_, line2_);

	const term_tokenizer::token &token1 = tokens1_.peek_token();
	const term_tokenizer::token &token2 = tokens2_.peek_token();

	if (token1.pos().line() != line1_no_) {
	    line1_no_ = token1.pos().line();
	    line1_.clear();
	}

	if (token2.pos().line() != line2_no_) {
	    line2_no_ = token2.pos().line();
	    line2_.clear();
	}

	line1_ += token1.lexeme();
	line2_ += token2.lexeme();

	if (token1.type() != token2.type() || token1.lexeme() != token2.lexeme()) {
	    return false;
	}

	tokens1_.consume_token();
	tokens2_.consume_token();
    }

    skip_whitespace(tokens1_, line1_no_, line1_);
    skip_whitespace(tokens2_, line2_no_, line2_);

    if (tokens1_.has_more_tokens() || tokens2_.has_more_tokens()) {
	return false;
    }

    return true;
}

void term_token_diff::report()
{
    if (!done_) {
	std::cout << "Nothing to report." << std::endl;
	return;
    }
    if (!failed_) {
	std::cout << "Nothing to report. Terms are equal." << std::endl;
	return;
    }

    if (line1_no_ != line2_no_) {
	std::cout << "Difference at lines " << line1_no_
		  << " and " << line2_no_ << std::endl;
    } else {
	std::cout << "Difference at line " << line1_no_ << std::endl;
    }

    std::cout << "Source  : " << line1_ << std::endl;
    std::cout << "Compare : " << line2_ << std::endl;
}

void term_token_diff::assert_equal(const std::string &s1,const std::string &s2,
				   const std::string &comment)
{
    std::stringstream in1(s1), in2(s2);
    term_token_diff diff(in1, in2);
    bool r = diff.check();
    if (!r) {
	if (!comment.empty()) {
	    std::cout << comment << std::endl;
	}
	diff.report();
    }
    assert(r);
}

}}

