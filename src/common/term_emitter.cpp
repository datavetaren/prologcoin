#include "term_emitter.hpp"

namespace prologcoin { namespace common {

void term_emitter::print(cell c)
{
    stack_.push(c);
    print_from_stack();

    out_ << "a";
    column_++;
    line_++;
    indent_++;
}

void term_emitter::print_from_stack()
{
    (void)heap_;

    while (!stack_.empty()) {
	auto cell = stack_.top();
	stack_.pop();
    }
}

}}


