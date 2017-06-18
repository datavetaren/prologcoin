#pragma once

#ifndef _common_term_ops_hpp
#define _common_term_ops_hpp

#include <unordered_map>
#include <string>
#include <iostream>
#include "term.hpp"

	
namespace std {

    template<> struct hash<::prologcoin::common::cell> {
        size_t operator()(const ::prologcoin::common::cell& k) const {
	    return hash<uint64_t>()(k.value());
	}
    };
}

namespace prologcoin { namespace common {

//
// term_ops
//
// This class defines the operator precendences.
//
class term_ops {
public:
    term_ops();

    void print(std::ostream &out);

    enum type_t { XF = 0, YF = 1, XFX = 2, XFY = 3,
		  YFX = 4, FY = 5, FX = 6 };

    struct op_entry {
	std::string name;
	size_t arity;
        size_t precedence;
	type_t type;

	std::string typestr() const {
	    switch (type) {
	    case XFX: return "xfx";
	    case FX: return "fx";
	    case FY: return "fy";
	    case XFY: return "xfy";
	    case YFX: return "yfx";
            default: return "?";
	    }
	}

	bool operator < (const op_entry &other) const {
	    return precedence < other.precedence;
	}
    };

    void put(const std::string &name, size_t arity, size_t precedence, type_t type)
    {
	con_cell c = con_cell( name, arity );
        op_prec_[c] = { name, arity, precedence, type };
    }

    const op_entry & none() const
    {
	return op_none_;
    }

    const op_entry & prec(cell c) const;

private:
    std::unordered_map<cell, op_entry> op_prec_;
    op_entry op_none_;
};

}}

#endif
