#include "global.hpp"

using namespace prologcoin::common;

namespace prologcoin { namespace global {

global::global(const std::string &data_dir) : data_dir_(data_dir), interp_(*this), current_height_(0) {
}

void global::erase_db()
{
}

}}

