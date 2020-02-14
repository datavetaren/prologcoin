#include "meta_data.hpp"

namespace prologcoin { namespace statedb {

void meta_entry::print(std::ostream &out) const
{
    out << "{index=" << index_ << ",height=" << height_ << ",offset=" << offset_ << ",size=" << size_ << "}";
}
    
void bucket::print(std::ostream &out) const
{
    size_t num_entries = 0;
    for (auto &v : entries_) {
        num_entries += v.size();
    }
    out << "bucket{num_entries=" << num_entries << ",[";
    bool first = true;
    for (auto &v : entries_) {
        for (auto &e : v) {
	    if (!first) out << ", ";
	    e.print(out);
	    first = false;
        }
    }
    out << "]}";
}
    
}}
