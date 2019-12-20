#include <boost/algorithm/string.hpp>
#include "../../common/term_tools.hpp"
#include "../interpreter.hpp"
#include "../wam_interpreter.hpp"
#include "../wam_compiler.hpp"

using namespace prologcoin::common;
using namespace prologcoin::interp;

namespace prologcoin { namespace interp {

static void header( const std::string &str )
{
    std::cout << "\n";
    std::cout << "--- [" + str + "] " + std::string(60 - str.length(), '-') << "\n";
    std::cout << "\n";
}

static void check_gold(std::string actual, std::string expect)
{
    std::stringstream ss(actual);
    std::stringstream goldss(expect);

    while (!ss.eof()) {
        std::string actual;
        std::string expected;
	std::getline(ss, actual);
	std::getline(goldss, expected);
	std::cout << actual << std::endl;
	boost::trim(actual);
	boost::trim(expected);
	if (actual != expected) {
	    std::cout << "ACTUAL: " << actual << std::endl;
	    std::cout << "EXPECT: " << expected << std::endl;
	    assert(actual == expected);
	}
    }

}

class test_wam_compiler
{
public:
    test_wam_compiler() : interp_(), comp_(interp_) { }

    void test_flatten();
    void test_compile();
    void test_partition();
    void test_compile2();
    void test_varset();
    void test_unsafe_set_unify();

private:
    interpreter interp_;
    wam_compiler comp_;
};

}}

void test_wam_compiler::test_flatten()
{
    term t = interp_.parse("p(f(X),h(Y,f(a)),Y).");
    auto fl = comp_.flatten(t, wam_compiler::COMPILE_PROGRAM, true);
    comp_.print_prims(fl);
}

static void test_flatten()
{
    header("test_flatten");

    test_wam_compiler test;
    test.test_flatten();
}

void test_wam_compiler::test_compile()
{
    term t = interp_.parse("p(Z,h(Z,W),f(W)).");

    std::cout << "Compile " << interp_.to_string(t) << " as QUERY:" << std::endl << std::endl;

    {
        wam_interim_code seq(interp_);
        comp_.compile_query_or_program(t,wam_compiler::COMPILE_QUERY,seq);
        seq.print(std::cout);
    }

    term t2 = interp_.parse("p(Z,h(Z,W),f(W)).");

    std::cout << std::endl << "Compile " << interp_.to_string(t2) << " as PROGRAM:" << std::endl << std::endl;

    {
        wam_interim_code seq(interp_);
	comp_.compile_query_or_program(t2,wam_compiler::COMPILE_PROGRAM,seq);
	seq.print(std::cout);
    }

    {
        std::string prog =
        R"PROG(
            append([X|Xs],Ys,[X|Zs]) :- append(Xs,Ys,Zs).
            append([],Ys,Ys).
            nrev([X|Xs],Ys) :- nrev(Xs,Ys0), append(Ys0,[X],Ys), Ys = 123.
            nrev([],[]).
          )PROG";

	std::cout << std::endl << "Compile append & nrev" << std::endl << std::endl;

	try {
	    interp_.load_program(prog);
	} catch (std::runtime_error &err) {
	    std::cout << "Syntax error: " << err.what() << std::endl;
	    throw;
	}

	auto &clauses = interp_.get_predicate(con_cell("nrev",2));

	wam_interim_code seq(interp_);
	comp_.compile_clause(clauses[0], seq);

	seq.print(std::cout);
    }
}

void test_wam_compiler::test_compile2()
{
    std::string prog2 =
        R"PROG(
          append([X|Xs],Ys,[X|Zs]) :- append(Xs,Ys,Zs).
          append([],Ys,Ys).
          nrev([X|Xs],Ys) :- nrev(Xs,Ys0), append(Ys0,[X],Ys), Ys = 123.
          nrev([],[]).

          call(or(X,Y)) :- call(X).
          call(trace) :- trace.
          call(or(X,Y)) :- call(Y).
          call(notrace) :- notrace.
          call(nl) :- nl.
          call(X) :- builtin(X).
          call(X) :- extern(X).
          call(call(X)) :- call(X).
          call(repeat).
          call(repeat) :- call(repeat).
          call(true).
        )PROG";

    // std::cout << std::endl << "Compile append & nrev" << std::endl << std::endl;
    try {
        interp_.load_program(prog2);
    } catch (std::runtime_error &err) {
        std::cout << "Syntax error: " << err.what() << std::endl;
        throw;
    }

    interp_.compile(con_cell("[]",0), con_cell("call",1));
    interp_.compile(con_cell("[]",0), con_cell("append",3));
    interp_.compile(con_cell("[]",0), con_cell("nrev",2));
    interp_.print_code(std::cout);
}

void test_wam_compiler::test_partition()
{
    std::string prog =
        R"PROG(
          call(or(X,Y)) :- call(X).
          call(trace) :- trace.
          call(or(X,Y)) :- call(Y).
          call(notrace) :- notrace.
          call(nl) :- nl.
          call(X) :- builtin(X).
          call(X) :- extern(X).
          call(call(X)) :- call(X).
          call(repeat).
          call(repeat) :- call(repeat).
          call(true).
       )PROG";

    std::string gold =
        R"GOLD(Section 0:
   call(or(X, Y)) :- call(X)
   call(trace) :- trace
   call(or(X, Y)) :- call(Y)
   call(notrace) :- notrace
   call(nl) :- nl
Section 1:
   call(X) :- builtin(X)
Section 2:
   call(X) :- extern(X)
Section 3:
   call(call(X)) :- call(X)
   call(repeat)
   call(repeat) :- call(repeat)
   call(true)
)GOLD";

    std::string gold2 =
        R"GOLD(--- SECTION 0 ---------------
Section 0:
   call(or(X, Y)) :- call(X)
   call(or(X, Y)) :- call(Y)
Section 1:
   call(trace) :- trace
Section 2:
   call(notrace) :- notrace
Section 3:
   call(nl) :- nl
--- SECTION 1 ---------------
Section 0:
   call(X) :- builtin(X)
--- SECTION 2 ---------------
Section 0:
   call(X) :- extern(X)
--- SECTION 3 ---------------
Section 0:
   call(call(X)) :- call(X)
Section 1:
   call(repeat)
   call(repeat) :- call(repeat)
Section 2:
   call(true)
)GOLD";

    try {
        interp_.load_program(prog);
    } catch (syntax_exception &ex) {
      std::cout << "Syntax exception: " << ex.what() << ": " << interp_.to_string(ex.get_term()) << std::endl;
	throw ex;
    }

    auto &clauses = interp_.get_predicate(con_cell("call",1));
    
    auto p = comp_.partition_clauses_nonvar(clauses);

    std::stringstream ss;
    comp_.print_partition(ss, p);

    check_gold(ss.str(), gold);

    std::stringstream ss2;
    size_t i = 0;
    for (auto &cs : p) {
        ss2 << "--- SECTION " << i << " ---------------" << std::endl;
        auto p1 = comp_.partition_clauses_first_arg(cs);
	comp_.print_partition(ss2, p1);
	i++;
    }

    check_gold(ss2.str(), gold2);
}

static void test_compile()
{
    header("test_compile");

    test_wam_compiler test;
    test.test_compile();
}

static void test_compile2()
{
    header("test_compile2");

    test_wam_compiler test;
    test.test_compile2();
}

static void test_instruction_sequence()
{
    header("test_instruction_sequence");

    wam_interpreter interp;
    interp.add(wam_instruction<PUT_VARIABLE_X>(1, 2));
    interp.add(wam_instruction<PUT_VARIABLE_X>(3, 4));
    interp.add(wam_instruction<PUT_VARIABLE_Y>(5, 6));
    interp.add(wam_instruction<PUT_VALUE_X>(6, 7));
    interp.add(wam_instruction<PUT_VALUE_Y>(8, 9));
    interp.add(wam_instruction<PUT_UNSAFE_VALUE_Y>(10, 11));
    interp.add(wam_instruction<PUT_STRUCTURE_A>(con_cell("f",2), 12));
    interp.add(wam_instruction<PUT_STRUCTURE_X>(con_cell("f",2), 11));
    interp.add(wam_instruction<PUT_STRUCTURE_Y>(con_cell("f",2), 10));
    interp.add(wam_instruction<PUT_LIST_A>(13));
    interp.add(wam_instruction<PUT_LIST_X>(14));
    interp.add(wam_instruction<PUT_LIST_Y>(15));
    interp.add(wam_instruction<PUT_CONSTANT>(con_cell("foo",0), 14));
    interp.add(wam_instruction<PUT_CONSTANT>(int_cell(4711), 15));
    interp.add(wam_instruction<GET_VARIABLE_X>(16,17));
    interp.add(wam_instruction<GET_VARIABLE_Y>(16,17));
    interp.add(wam_instruction<GET_VALUE_X>(18,19));
    interp.add(wam_instruction<GET_VALUE_Y>(20,21));
    interp.add(wam_instruction<GET_STRUCTURE_A>(con_cell("ga",2), 22));
    interp.add(wam_instruction<GET_STRUCTURE_X>(con_cell("gx",2), 21));
    interp.add(wam_instruction<GET_STRUCTURE_Y>(con_cell("gy",2), 20));
    interp.add(wam_instruction<GET_LIST_A>(23));
    interp.add(wam_instruction<GET_LIST_X>(24));
    interp.add(wam_instruction<GET_LIST_Y>(25));
    interp.add(wam_instruction<GET_CONSTANT>(con_cell("bar",0), 24));
    interp.add(wam_instruction<GET_CONSTANT>(int_cell(-123), 25));
    interp.add(wam_instruction<SET_VARIABLE_X>(26));
    interp.add(wam_instruction<SET_VARIABLE_Y>(27));
    interp.add(wam_instruction<SET_VALUE_X>(28));
    interp.add(wam_instruction<SET_VALUE_Y>(29));
    interp.add(wam_instruction<SET_LOCAL_VALUE_X>(30));
    interp.add(wam_instruction<SET_LOCAL_VALUE_Y>(31));
    interp.add(wam_instruction<SET_CONSTANT>(con_cell("xyz",0)));
    interp.add(wam_instruction<SET_CONSTANT>(int_cell(456)));
    interp.add(wam_instruction<SET_VOID>(8));
    interp.add(wam_instruction<UNIFY_VARIABLE_X>(32));
    interp.add(wam_instruction<UNIFY_VARIABLE_Y>(33));
    interp.add(wam_instruction<UNIFY_VALUE_X>(34));
    interp.add(wam_instruction<UNIFY_VALUE_Y>(35));
    interp.add(wam_instruction<UNIFY_LOCAL_VALUE_X>(36));
    interp.add(wam_instruction<UNIFY_LOCAL_VALUE_Y>(37));
    interp.add(wam_instruction<UNIFY_CONSTANT>(con_cell("abc",0)));
    interp.add(wam_instruction<UNIFY_CONSTANT>(int_cell(-789)));
    interp.add(wam_instruction<UNIFY_VOID>(9));
    interp.add(wam_instruction<ALLOCATE>());
    interp.add(wam_instruction<DEALLOCATE>());
    interp.add(wam_instruction<CALL>(con_cell("[]",0), con_cell("f",3), 10));
    interp.add(wam_instruction<EXECUTE>(con_cell("g",4)));
    interp.add(wam_instruction<PROCEED>());
    interp.add(wam_instruction<TRY_ME_ELSE>(int_cell(1)));
    interp.add(wam_instruction<RETRY_ME_ELSE>(int_cell(2)));
    interp.add(wam_instruction<TRUST_ME>());
    interp.add(wam_instruction<TRY>(int_cell(3)));
    interp.add(wam_instruction<RETRY>(int_cell(4)));
    interp.add(wam_instruction<TRUST>(int_cell(5)));
    interp.add(wam_instruction<SWITCH_ON_TERM>(int_cell(6),
					       int_cell(7),
					       code_point::fail(),
					       int_cell(8)));

    auto *hm1 = interp.new_hash_map();
    hm1->insert(std::make_pair(con_cell("f", 2), int_cell(126)));
    hm1->insert(std::make_pair(int_cell(1234), int_cell(207)));
    interp.add(wam_instruction<SWITCH_ON_CONSTANT>(hm1));

    auto *hm2 = interp.new_hash_map();
    hm2->insert(std::make_pair(con_cell("f", 2), int_cell(221)));
    hm2->insert(std::make_pair(con_cell("g", 3), int_cell(235)));
    interp.add(wam_instruction<SWITCH_ON_STRUCTURE>(hm2));

    interp.add(wam_instruction<NECK_CUT>());
    interp.add(wam_instruction<GET_LEVEL>(111));
    interp.add(wam_instruction<CUT>(112));

    interp.print_code(std::cout);
}

static void test_partition()
{
    header("test_partition");

    test_wam_compiler test;
    test.test_partition();
}

static int find_index(const std::string src[], size_t n,
		      const std::string &search)
{
    for (size_t i = 0; i < n; i += 2) {
	std::stringstream in1(src[i]);
	std::stringstream in2(search);
	term_token_diff d(in1, in2);
	if (d.check()) {
	    return static_cast<int>(i);
	}
    }
    return -1;
}

void test_wam_compiler::test_varset()
{
    static const size_t N = 26;
    static std::string EXPECT[N] =
	{ "foo(X, Y) :- A = 1 ; B = X, C = Y ; foo(X, A, B), bar(B, Q, W) ; baz(boo(W, D, E))", "[C,Y,X,A,B,Q,W,D,E]",
	  "(B = X, C = Y)", "[C,Y,X,B]",
	  "foo(X, A, B), bar(B, Q, W) ; baz(boo(W, D, E))", "[X,A,B,Q,W,D,E]",
	  "C = Y", "[C,Y]",
	  "(foo(X, A, B), bar(B, Q, W))", "[X,A,B,Q,W]",
	  "B = X", "[X,B]",
	  "bar(B, Q, W)", "[B,Q,W]",
	  "baz(boo(W, D, E))", "[W,D,E]",
	  "foo(X, A, B)", "[X,A,B]",
	  "B = X, C = Y ; foo(X, A, B), bar(B, Q, W) ; baz(boo(W, D, E))", "[C,Y,X,A,B,Q,W,D,E]",
	  "A = 1 ; B = X, C = Y ; foo(X, A, B), bar(B, Q, W) ; baz(boo(W, D, E))", "[C,Y,X,A,B,Q,W,D,E]",
	  "A = 1", "[A]",
	  "foo(X, Y)", "[Y,X]"
	};

    std::vector<size_t> match;

    auto t = interp_.parse("foo(X,Y) :- (A = 1 ; (B = X, C = Y) ; (foo(X,A,B), bar(B,Q,W) ; baz(boo(W,D,E)))).");
    comp_.compute_varsets(t);

    for (auto ts : comp_.varsets_) {
	auto t = ts.first;
	auto s = ts.second;

	auto actual = interp_.to_string(t);
	auto index = find_index(EXPECT, N, actual);

	if (index == -1) {
	    std::cout << "Error: Did not find this in expected table:\n";
	    std::cout << "    " << actual << "\n";
	    assert(index != -1);
	}

	auto actual_set = comp_.varset_to_string(s);

	if (actual_set != EXPECT[index+1]) {
	    std::cout << "The term: " << actual << "\n";
	    std::cout << "Error: Did not match the expected set:\n";
	    std::cout << "   ACTUAL: " << actual_set << "\n";
	    std::cout << "   EXPECT: " << EXPECT[index+1] << "\n";
	    assert(actual_set == EXPECT[index+1]);
	}

	std::cout << std::setw(32) << std::setiosflags(std::ios::left) << interp_.to_string(t) << ": " << comp_.varset_to_string(s) << "\n";
	match.push_back(index);
    }

    for (size_t i = 0; i < N; i += 2) {
	if (std::find(match.begin(), match.end(), i) == match.end()) {
	    std::cout << "Error: Did not find entry:\n";
	    std::cout << EXPECT[i] << ": " << EXPECT[i+1] << "\n";
	    assert(std::find(match.begin(), match.end(), i) != match.end());
	}
    }
}

static void test_varset()
{
    header("test_varset");
    
    test_wam_compiler test;
    test.test_varset();
}

void test_wam_compiler::test_unsafe_set_unify()
{
    term t = interp_.parse("a(X) :- b(f(X)).");
    wam_interim_code seq(interp_);
    comp_.compile_clause(t, seq);

    seq.print(std::cout);
}

static void test_unsafe_set_unify()
{
    header("test_unsafe_set_unify");

    test_wam_compiler test;
    test.test_unsafe_set_unify();
}

int main( int argc, char *argv[] )
{
    test_flatten();
    test_instruction_sequence();
    test_partition();
    test_compile();
    test_compile2();
    test_varset();
    test_unsafe_set_unify();

    return 0;
}
