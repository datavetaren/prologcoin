#include <boost/date_time/posix_time/posix_time.hpp>
#include "../../common/term_tools.hpp"
#include "../../common/term_serializer.hpp"
#include "../interpreter.hpp"

using namespace prologcoin::common;
using namespace prologcoin::interp;

static void header( const std::string &str )
{
    std::cout << "\n";
    std::cout << "--- [" + str + "] " + std::string(60 - str.length(), '-') << "\n";
    std::cout << "\n";
}

static void test_up_and_down()
{
    header("test_up_and_down()");

    auto start = boost::posix_time::microsec_clock::local_time();
    {
	interpreter interp;
	interp.setup_standard_lib();
    }
    auto stop = boost::posix_time::microsec_clock::local_time();
    auto dt = stop - start;
    std::cout << "Prolog engine up+down in " << dt.total_milliseconds() << " milliseconds\n";
}

static bool check_terms(const std::string &actual,
			const std::string &expected)
{
    std::stringstream in_actual(actual);
    std::stringstream in_expect(expected);

    term_token_diff diff(in_actual, in_expect);
    if (!diff.check()) {
	std::cout << "Actual is not expected." << std::endl;
	std::cout << "Difference at line";
	if (diff.line1_no() != diff.line2_no()) {
	    std::cout << "s " << diff.line1_no() << " and "
		      << diff.line2_no() << ":" << std::endl;
	} else {
	    std::cout << " " << diff.line1_no() << ":" << std::endl;
	}
	std::cout << "  Actual: " << diff.line1() << "\n";
	std::cout << "  Expect: " << diff.line2() << "\n";
	return false;
    }
    return true;
}

static void eval_check_1(const std::string &program,
			 const std::string &query,
			 const std::string &expected)
{
    interpreter interp;

    term prog = interp.parse(program);

    // interp.set_debug(true);
    interp.load_program(prog);

    std::cout << "Program --------------------------------------\n";
    interp.print_db(std::cout);
    std::cout << "----------------------------------------------\n";
    
    term qr = interp.parse(query);
    std::cout << "?- " << interp.to_string(qr) << ".\n";

    interp.execute(qr);

    std::string result = interp.to_string(qr);

    std::cout << "----------------------------------------------" << std::endl;
    interp.print_result(std::cout);
    std::cout << "----------------------------------------------" << std::endl;

    assert(check_terms( interp.get_result(), expected ));

    std::cout << std::endl;
}

static void test_simple_interpreter()
{
    header("test_simple_interpreter()");

    eval_check_1("[(append([X|Xs],Ys,[X|Zs]) :- append(Xs,Ys,Zs)), "
	          "  append([], Zs, Zs)].", 
	       "append([1,2,3],[4,5,6],Q).",
	       "Q = [1,2,3,4,5,6]" );

    eval_check_1("[(append([X|Xs],Ys,[X|Zs]) :- append(Xs,Ys,Zs)), "
	         "   append([], Zs, Zs),"
	         "(nrev([X|Xs],Ys) :- nrev(Xs,Rs), append(Rs,[X],Ys)),"
	         "   nrev([],[])"
	          "].",
	       "nrev([1,2,3],Q).",
	       "Q = [3,2,1]" );

    eval_check_1("[member(X,[X|_]), (member(X,[_|Xs]) :- member(X,Xs))].",
	       "member(A,Xs).",
	       "Xs = [A| _]" );

    eval_check_1("[append([], Zs, Zs),"
	       "(append([X|Xs],Ys,[X|Zs]) :- append(Xs,Ys,Zs))].",
	       "append(A,B,C).",
	       "A = [], C = B");
}

static void eval_check_n(const std::string &program,
			 const std::string &query,
			 size_t count,
			 const std::string &expected)
{
    interpreter interp;

    // interp.set_debug(true);

    term prog = interp.parse(program);

    // interp.set_debug(true);
    interp.load_program(prog);

    std::cout << "Program --------------------------------------\n";
    interp.print_db(std::cout);
    std::cout << "----------------------------------------------\n";
    
    term qr = interp.parse(query);
    std::cout << "?- " << interp.to_string(qr) << ".\n";

    std::cout << "----------------------------------------------" << std::endl;

    std::string result;

    bool ok = interp.execute(qr);
    if (!ok) {
	std::cout << "Execution failed of query." << std::endl;
	assert(ok);
    }

    // interp.set_debug(true);

    size_t i = 0;
    while (ok) {
	std::string one_result = interp.get_result(false);
	std::cout << "Result: " << one_result << std::endl;
	result += one_result + "\n";
	ok = interp.next();
        if (++i == count) {
	    break;
	}
    }
}

static void test_backtracking_interpreter()
{
    header("test_backtracking_interpreter()");

    eval_check_n("[member(X,[X|_]), (member(X,[_|Xs]) :- member(X,Xs))].",
		 "member(A,B).", 5,
		 "blaha");

    eval_check_n("[append([], Zs, Zs),"
                 "   (append([X|Xs],Ys,[X|Zs]) :- append(Xs,Ys,Zs)),"
	         "nrev([],[]),"
		 "   (nrev([X|Xs],Ys) :- nrev(Xs,Rs), append(Rs,[X],Ys))"
	         "].",
	       "nrev(A,B).", 5,
	       "blaha" );

}

static void test_interpreter_serialize()
{
    header("test_interpreter_serialize()");

    interpreter interp;

    const std::string program = 
	R"PROGRAM(
           [build(X,Y) :- Y = foo(X,X,bar(X,42))].
          )PROGRAM";

    const std::string query = "build(some(term(42)),Q).";

    const std::string expected = "Q = foo(some(term(42)), some(term(42)), bar(some(term(42)), 42))";

    term prog = interp.parse(program);

    interp.load_program(prog);

    std::cout << "Program --------------------------------------\n";
    interp.print_db(std::cout);
    std::cout << "----------------------------------------------\n";
    
    term qr = interp.parse(query);
    std::cout << "?- " << interp.to_string(qr) << ".\n";

    interp.execute(qr);

    std::string result = interp.to_string(qr);

    std::cout << "----------------------------------------------" << std::endl;
    interp.print_result(std::cout);
    std::cout << "----------------------------------------------" << std::endl;

    assert(check_terms( interp.get_result(), expected ));

    std::cout << " Checking serialization; print cells." << std::endl;

    term t = interp.get_result_term("Q");
    term_serializer ser(interp);
    term_serializer::buffer_t buffer;
    ser.write(buffer, t);

    ser.print_buffer(buffer, buffer.size());

    // Lock down number of cells written
    size_t expected_cells = 15;
    size_t actual_cells = buffer.size() / sizeof(cell);
    std::cout << "Expected number of cells=" << expected_cells << ": actual=" << buffer.size() / sizeof(cell) << std::endl;
    assert(actual_cells == expected_cells);

    // Read back serialized data
    term t2 = ser.read(buffer);
    std::cout << "Read serialized buffer: " << interp.to_string(t2) << "\n";

    assert(interp.to_string(t) == interp.to_string(t2));
}

int main( int argc, char *argv[] )
{
    test_up_and_down();
    test_simple_interpreter();
    test_backtracking_interpreter();
    test_interpreter_serialize();

    return 0;
}
