#include "../interpreter.hpp"

using namespace prologcoin::common;
using namespace prologcoin::interp;

static void header( const std::string &str )
{
    std::cout << "\n";
    std::cout << "--- [" + str + "] " + std::string(60 - str.length(), '-') << "\n";
    std::cout << "\n";
}

static void eval_check_1(const std::string &program,
			 const std::string &query,
			 const std::vector<std::string> &expected)
{
    interpreter interp;

    term prog = interp.env().parse(program);

    interp.load_program(prog);

    std::cout << "Program --------------------------------------\n";
    interp.print_db(std::cout);
    std::cout << "----------------------------------------------\n";
    
    term qr = interp.env().parse(query);
    std::cout << "?- " << interp.env().to_string(qr) << ".\n";

    interp.execute(qr);

    std::string result = interp.env().to_string(qr);

    std::cout << "----------------------------------------------" << std::endl;
    interp.print_result(std::cout);
    std::cout << "----------------------------------------------" << std::endl;

    for (auto &e : expected) {
        std::cout << "Expected: " << e << std::endl;
    }

    size_t cnt = 0;
    for (auto &v : interp.get_result()) {
        assert(v == expected[cnt]);
        cnt++;
    }

    assert(cnt == expected.size());

    std::cout << std::endl;
}

static void test_simple_interpreter()
{
    header("test_simple_interpreter()");

    eval_check_1("[(append([X|Xs],Ys,[X|Zs]) :- append(Xs,Ys,Zs)), "
	          "  append([], Zs, Zs)].", 
	       "append([1,2,3],[4,5,6],Q).",
	       {"Q = [1,2,3,4,5,6]"} );

    eval_check_1("[(append([X|Xs],Ys,[X|Zs]) :- append(Xs,Ys,Zs)), "
	         "   append([], Zs, Zs),"
	         "(nrev([X|Xs],Ys) :- nrev(Xs,Rs), append(Rs,[X],Ys)),"
	         "   nrev([],[])"
	          "].",
	       "nrev([1,2,3],Q).",
	       {"Q = [3,2,1]"} );

    eval_check_1("[member(X,[X|_]), (member(X,[_|Xs]) :- member(X,Xs))].",
	       "member(A,Xs).",
	       {"A = G_0", "Xs = [G_0| _]"} );

    eval_check_1("[append([], Zs, Zs),"
	       "(append([X|Xs],Ys,[X|Zs]) :- append(Xs,Ys,Zs))].",
	       "append(A,B,C).",
	       {"A = []", "B = G_0", "C = G_0"});
}

int main( int argc, char *argv[] )
{
    test_simple_interpreter();

    return 0;
}
