#include "term_parser.hpp"

namespace prologcoin { namespace common {

//
// The parsing algorithm is similar to what happens in a
// pushdown automata.
//

term_parser::term_parser(std::istream &in, heap &h, term_ops &ops)
        : in_(in),
	  heap_(h),
	  ops_(ops),
          column_(0),
          line_(0)
{
    (void)in_;
    (void)heap_;
    (void)ops_;
    (void)column_;
    (void)line_;
}

}}


