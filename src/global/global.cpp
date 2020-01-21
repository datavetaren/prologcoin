#include "global.hpp"

using namespace prologcoin::common;

namespace prologcoin { namespace global {

global::global() : interp_(*this), current_height_(0) {
}

}}

