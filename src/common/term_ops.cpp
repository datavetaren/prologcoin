#include <vector>
#include <iomanip>
#include "term_ops.hpp"

namespace prologcoin { namespace common {

term_ops::term_ops()
{
    static op_entry DEFAULT[] =
	  { { "-->",    2, 1200,       XFX },
	    { ":-",     2, 1200,       XFX },
	    { ":-",     2, 1200,       FX },
	    { "?-",     2, 1200,       FX },
	    { ";",      2, 1100,       XFY },
	    { "|",      2, 1100,       XFY },
	    { "->",     2, 1050,       XFY },
	    { "*->",    2, 1050,       XFY },
	    { ",",      2, 1000,       XFY },
	    { ":=",     2, 990,        XFX },
	    { "\\+",    1, 900,        FY },
	    { "<",      2, 700,        XFX },
	    { "=",      2, 700,        XFX },
	    { "=..",    2, 700,        XFX },
	    { "=@=",    2, 700,        XFX },
	    { "\\=@=",  2, 700,        XFX },
	    { "=:=",    2, 700,        XFX },
	    { "=<",     2, 700,        XFX },
	    { "==",     2, 700,        XFX },
	    { "=\\=",   2, 700,        XFX },
	    { ">",      2, 700,        XFX },
	    { ">=",     2, 700,        XFX },
	    { "@<",     2, 700,        XFX },
	    { "@=<",    2, 700,        XFX },
	    { "@>",     2, 700,        XFX },
	    { "@>=",    2, 700,        XFX },
	    { "\\=",    2, 700,        XFX },
	    { "\\==",   2, 700,        XFX },
	    { "as",     2, 700,        XFX },
	    { "is",     2, 700,        XFX },
	    { ">:<",    2, 700,        XFX },
	    { ":<",     2, 700,        XFX },
	    { ":",      2, 600,        XFY },
	    { "+",      2, 500,        YFX },
	    { "-",      2, 500,        YFX },
	    { "/\\",    2, 500,        YFX },
	    { "\\/",    2, 500,        YFX },
	    { "xor",    2, 500,        YFX },
	    { "?",      1, 500,        FX },
	    { "*",      2, 400,        YFX },
	    { "/",      2, 400,        YFX },
	    { "//",     2, 400,        YFX },
	    { "div",    2, 400,        YFX },
	    { "rdiv",   2, 400,        YFX },
	    { "<<",     2, 400,        YFX },
	    { ">>",     2, 400,        YFX },
	    { "mod",    2, 400,        YFX },
	    { "rem",    2, 400,        YFX },
	    { "**",     2, 200,        XFX },
	    { "^",      2, 200,        XFY },
	    { "+",      1, 200,        FY },
	    { "-",      1, 200,        FY },
	    { "\\",     1, 200,        FY },
	    { ".",      2, 100,        YFX },
	    { "$",      1, 1,          FX }
	  };

    op_none_.arity = 0;
    op_none_.precedence = 0;
    op_none_.type = FX;

    for (auto i = 0; i < sizeof(DEFAULT) / sizeof(op_entry); i++) {
	const std::string &name = DEFAULT[i].name;
	size_t precedence = DEFAULT[i].precedence;
	size_t arity = DEFAULT[i].arity;
	type_t type = DEFAULT[i].type;
	std::string namekey(name);
	put( namekey, arity, precedence, type );
    }
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
