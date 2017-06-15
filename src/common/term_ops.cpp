#include <vector>
#include <iomanip>
#include "term_ops.hpp"

term_ops::term_ops()
{
    static op_entry DEFAULT[] =
	  { { "-->",    1200,       XFX },
	    { ":-",     1200,       XFX },
	    { ":-",     1200,       FX },
	    { "?-",     1200,       FX },
	    { ";",      1100,       XFY },
	    { "|",      1100,       XFY },
	    { "->",     1050,       XFY },
	    { "*->",    1050,       XFY },
	    { ",",      1000,       XFY },
	    { ":=",     990,        XFX },
	    { "\\+",    900,        FY },
	    { "<",      700,        XFX },
	    { "=",      700,        XFX },
	    { "=..",    700,        XFX },
	    { "=@=",    700,        XFX },
	    { "\\=@=",  700,        XFX },
	    { "=:=",    700,        XFX },
	    { "=<",     700,        XFX },
	    { "==",     700,        XFX },
	    { "=\\=",   700,        XFX },
	    { ">",      700,        XFX },
	    { ">=",     700,        XFX },
	    { "@<",     700,        XFX },
	    { "@=<",    700,        XFX },
	    { "@>",     700,        XFX },
	    { "@>=",    700,        XFX },
	    { "\\=",    700,        XFX },
	    { "\\==",   700,        XFX },
	    { "as",     700,        XFX },
	    { "is",     700,        XFX },
	    { ">:<",    700,        XFX },
	    { ":<",     700,        XFX },
	    { ":",      600,        XFY },
	    { "+",      500,        YFX },
	    { "-",      500,        YFX },
	    { "/\\",    500,        YFX },
	    { "\\/",    500,        YFX },
	    { "xor",    500,        YFX },
	    { "?",      500,        FX },
	    { "*",      400,        YFX },
	    { "/",      400,        YFX },
	    { "//",     400,        YFX },
	    { "div",    400,        YFX },
	    { "rdiv",   400,        YFX },
	    { "<<",     400,        YFX },
	    { ">>",     400,        YFX },
	    { "mod",    400,        YFX },
	    { "rem",    400,        YFX },
	    { "**",     200,        XFX },
	    { "^",      200,        XFY },
	    { "+",      200,        FY },
	    { "-",      200,        FY },
	    { "\\",     200,        FY },
	    { ".",      100,        YFX },
	    { "$",      1,          FX }
	  };

    for (auto i = 0; i < sizeof(DEFAULT) / sizeof(op_entry); i++) {
	const std::string &name = DEFAULT[i].name;
	int priority = DEFAULT[i].priority;
	type_t type = DEFAULT[i].type;
	std::string namekey(name);
	op_pred_[namekey] = { name, priority, type };
    }
}

void term_ops::print( std::ostream &out )
{
    std::vector<op_entry> ops;
    for (auto e : op_pred_) {
	ops.push_back(e.second);
    }
    std::sort(ops.begin(), ops.end());
    std::reverse(ops.begin(), ops.end());
    for (auto e : ops) {
	out << std::setw(8) << e.name << " " << std::setw(4) << e.priority << " " << e.typestr() << "\n";
    }
}
