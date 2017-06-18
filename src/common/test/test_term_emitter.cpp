#include <iostream>
#include <iomanip>
#include <assert.h>
#include <common/term.hpp>
#include <common/term_ops.hpp>
#include <common/term_emitter.hpp>

using namespace prologcoin::common;

void header( const std::string &str )
{
    std::cout << "\n";
    std::cout << "--- [" + str + "] " + std::string(60 - str.length(), '-') << "\n";
    std::cout << "\n";
}

void test_simple_term()
{
    header("test_simple_term()");

    heap h;
    term_ops ops;
    std::stringstream ss;
    term_emitter emit(ss, h, ops);

    con_cell h_2("h", 2);
    con_cell f_1("f", 1);
    con_cell p_3("p", 3);

    ref_cell Z, W;

    str_cell h_2_str = h.new_str(h_2);
    h.set_arg(h_2_str, 0, Z = h.new_ref());
    h.set_arg(h_2_str, 1, W = h.new_ref());
    str_cell f_1_str = h.new_str(f_1);
    h.set_arg(f_1_str, 0, W);
    str_cell p_3_str = h.new_str(p_3);
    h.set_arg(p_3_str, 0, Z);
    h.set_arg(p_3_str, 1, h_2_str);
    h.set_arg(p_3_str, 2, f_1_str);

    emit.print(p_3_str);

    std::cout << ss.str() << "\n";

    assert(ss.str() == "p(E, h(E, F), f(F))");
}

int main(int argc, char *argv[])
{
    test_simple_term();
}
