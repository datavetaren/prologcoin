#include "term_tools.hpp"
#include "term_tokenizer.hpp"

namespace prologcoin { namespace common {

term_token_diff::term_token_diff(std::istream &in1, std::istream &in2)
    : line1_no_(-1), line2_no_(-1), tokens1_(in1), tokens2_(in2) { }

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

bool term_token_diff::check()
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

}}

