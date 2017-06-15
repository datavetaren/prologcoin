#pragma once

#ifndef _common_term_ops_hpp
#define _common_term_ops_hpp

#include <unordered_map>
#include <string>
#include <iostream>

//
// term_ops
//
// This class defines the operator precendences.
//
class term_ops {
public:
    term_ops();

    void print(std::ostream &out);

    enum type_t { XFX = 0, FX = 1, FY = 2, XFY = 3, YFX = 4 } type;

    struct op_entry {
	std::string name;
	int priority;
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
	    return priority < other.priority;
	}
    };

    void put(const std::string &name, int priority, type_t type)
    {
	op_pred_[name] = { name, priority, type };
    }

private:

    std::unordered_map<std::string, op_entry> op_pred_;
};

#endif
