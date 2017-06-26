#include "term_parser.hpp"

namespace prologcoin { namespace common {

//
// This is a hand coded shift/reduce parser.
//

term_parser::term_parser(std::istream &in, heap &h, term_ops &ops)
        : tokenizer_(in),
	  heap_(h),
	  ops_(ops)
{
    (void)tokenizer_;
    (void)heap_;
    (void)ops_;
}



}}


