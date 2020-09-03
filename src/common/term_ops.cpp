#include <vector>
#include <iomanip>
#include "term_ops.hpp"

namespace prologcoin { namespace common {

term_ops::term_ops()
{
    reset();
}

void term_ops::reset()
{
    static op_entry DEFAULT[] =
	{
	    { "-->",    2, 1200,       XFX, SPACE_XFX},
	    { ":-",     2, 1200,       XFX, SPACE_XFX },
	    { ":-",     1, 1200,       FX,  SPACE_XF },
	    { "?-",     1, 1200,       FX,  SPACE_XF },

	    { ";",      2, 1100,       XFY, SPACE_XFX },
	    { "|",      2, 1100,       XFY, SPACE_XFX },
	    { "->",     2, 1050,       XFY, SPACE_XFX },
	    { "*->",    2, 1050,       XFY, SPACE_XFX },
	    { ",",      2, 1000,       XFY, SPACE_FX },

	    // Non-standard Prolog operator; for Prologcoin only
	    { "@",      2, 995,       XFY, SPACE_XFX },

	    { ":=",     2, 990,        XFX, SPACE_XFX },
	    { "\\+",    1, 900,        FY, SPACE_FX },
	    { "<",      2, 700,        XFX, SPACE_XFX },
	    { "=",      2, 700,        XFX, SPACE_XFX },
	    { "=..",    2, 700,        XFX, SPACE_XFX },
	    { "=@=",    2, 700,        XFX, SPACE_XFX },
	    { "\\=@=",  2, 700,        XFX, SPACE_XFX },
	    { "=:=",    2, 700,        XFX, SPACE_XFX },
	    { "=<",     2, 700,        XFX, SPACE_XFX },
	    { "==",     2, 700,        XFX, SPACE_XFX },
	    { "=\\=",   2, 700,        XFX, SPACE_XFX },
	    { ">",      2, 700,        XFX, SPACE_XFX },
	    { ">=",     2, 700,        XFX, SPACE_XFX },
	    { "@<",     2, 700,        XFX, SPACE_XFX },
	    { "@=<",    2, 700,        XFX, SPACE_XFX },
	    { "@>",     2, 700,        XFX, SPACE_XFX },
	    { "@>=",    2, 700,        XFX, SPACE_XFX },
	    { "\\=",    2, 700,        XFX, SPACE_XFX },
	    { "\\==",   2, 700,        XFX, SPACE_XFX },
	    { "as",     2, 700,        XFX, SPACE_XFX },
	    { "is",     2, 700,        XFX, SPACE_XFX },
	    { ">:<",    2, 700,        XFX, SPACE_XFX },
	    { ":<",     2, 700,        XFX, SPACE_XFX },
	    { ":",      2, 600,        XFY, SPACE_XFX },
	    { "+",      2, 500,        YFX, SPACE_F },
	    { "-",      2, 500,        YFX, SPACE_F },
	    { "/\\",    2, 500,        YFX, SPACE_XFX },
	    { "\\/",    2, 500,        YFX, SPACE_XFX },
	    { "xor",    2, 500,        YFX, SPACE_XFX },
	    { "?",      1, 500,        FX, SPACE_F },
	    { "*",      2, 400,        YFX, SPACE_F },
	    { "/",      2, 400,        YFX, SPACE_F },
	    { "//",     2, 400,        YFX, SPACE_XFX },
	    { "div",    2, 400,        YFX, SPACE_XFX },
	    { "rdiv",   2, 400,        YFX, SPACE_XFX },
	    { "<<",     2, 400,        YFX, SPACE_F },
	    { ">>",     2, 400,        YFX, SPACE_F },
	    { "mod",    2, 400,        YFX, SPACE_XFX },
	    { "rem",    2, 400,        YFX, SPACE_XFX },
	    { "**",     2, 200,        XFX, SPACE_F },
	    { "^",      2, 200,        XFY, SPACE_F },
	    { "+",      1, 200,        FY, SPACE_FX },
	    { "-",      1, 200,        FY, SPACE_FX },
	    { "\\",     1, 200,        FY, SPACE_F },

	    { ".",      2, 100,        YFX, SPACE_F },
	    { "$",      1, 1,          FX, SPACE_F }
	  };

    op_none_.arity = 0;
    op_none_.precedence = 0;
    op_none_.type = FX;
    op_none_.space = SPACE_F;

    op_prec_.clear();
    name_prec_.clear();
    op_empty_list_.clear();

    for (auto i = std::size_t(0); i < sizeof(DEFAULT) / sizeof(op_entry); i++) {
	const std::string &name = DEFAULT[i].name;
	size_t precedence = DEFAULT[i].precedence;
	size_t arity = DEFAULT[i].arity;
	type_t type = DEFAULT[i].type;
	space_t space = DEFAULT[i].space;
	std::string namekey(name);
	put( namekey, arity, precedence, type, space );
    }
}

std::string term_ops::op_entry::str() const
{
    std::stringstream ss;
    ss << "op(" << name << "," << precedence << "," << typestr() << ")";
    return ss.str();
}

void term_ops::put(const std::string &name, size_t arity, size_t precedence, term_ops::type_t type, term_ops::space_t space)
{
    con_cell c = con_cell( name, arity );
    op_entry newE = {name, arity, precedence, type, space};
    op_prec_[c] = newE;

    auto &ops = name_prec_[name];
    ops.insert( std::upper_bound( ops.begin(), ops.end(), newE), newE );
}

const term_ops::op_entry & term_ops::prec(cell c) const
{
    auto it = op_prec_.find(c);
    if (it == op_prec_.end()) {
	return op_none_;
    } else {
	return it->second;
    }
}

const std::vector<term_ops::op_entry> & term_ops::prec(const std::string &name) const
{
    auto it = name_prec_.find(name);
    if (it == name_prec_.end()) {
	return op_empty_list_;
    }
    return it->second;
}

void term_ops::print( std::ostream &out )
{
    std::vector<op_entry> ops;
    for (auto e : op_prec_) {
	ops.push_back(e.second);
    }
    std::sort(ops.begin(), ops.end());
    std::reverse(ops.begin(), ops.end());
    for (auto e : ops) {
	out << std::setw(8) << e.name << " " << std::setw(4) << e.precedence << " " << e.typestr() << "\n";
    }
}

}}
