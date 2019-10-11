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
    std::cout << "Prolog engine up+down in " << dt.total_microseconds() << " microseconds\n";
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

static void test_interpreter_multi_instance()
{
    header("test_interpreter_multi_instance()");

    interpreter interp;

    interp.setup_standard_lib();

    const std::string expect[] = 
	{ "X = 1", "Y = a", "Y = b", "Y = c", "Y = d", "true", "Done.",
	  "X = 2", "Y = a", "Y = b", "Y = c", "Y = d", "true", "Done.",
	  "X = 3", "Y = a", "Y = b", "Y = c", "Y = d", "true", "Done.",
	  "X = 4", "Y = a", "Y = b", "Y = c", "Y = d", "true", "Done.",
	  "true",
	  "",
	};

    static size_t expect_count = 0;
    
    auto expect_check = [&](const std::string &actual)
	{
	    assert(!expect[expect_count].empty());
	    std::cout << "Actual: " << actual << std::endl;
	    std::cout << "Expect: " << expect[expect_count] << std::endl;
	    assert(expect[expect_count] == actual);
	    expect_count++;
	};

    term query1 = interp.parse("member(X, [1,2,3,4]).");
    term query2 = interp.parse("member(Y, [a,b,c,d]).");

    assert(interp.execute(query1));
    std::cout << "First answer: " << interp.get_result(false) << std::endl;
    
    expect_check(interp.get_result(false));
    
    while (interp.has_more()) {
	//
	// Start another instance of interpreter and apply query2
	//
	assert(interp.execute(query2));
	std::cout << "    [Nested] First answer: " << interp.get_result(false) << std::endl;
	expect_check(interp.get_result(false));
	while (interp.has_more()) {
	    interp.next();
	    std::cout << "    [Nested] Next answer:  " << interp.get_result(false) << std::endl;
	    expect_check(interp.get_result(false));
	}
	std::cout << "    [Nested] Done." << std::endl;
	expect_check("Done.");
	if (interp.is_instance()) {
	    std::cout << "    [Delete inner instance]." << std::endl;
	    interp.delete_instance();
	}

	//
	// Back at outer instance of interpreter
	//

	interp.next();
	std::cout << "Next answer:  " << interp.get_result(false) << std::endl;
	expect_check(interp.get_result(false));
    }
}

static void test_interpreter_freeze_preprocess()
{
    header("test_interpreter_freeze_preprocess()");

    interpreter interp;

    const std::string program = 
	R"PROGRAM(
           [foo(X,Y) :- (some(Y,Z), freeze(Z, (goal1(Y), goal2(X,Z))))].
          )PROGRAM";

    /*
    const std::string query = "build(some(term(42)),Q).";

    const std::string expected = "Q = foo(some(term(42)), some(term(42)), bar(some(term(42)), 42))";
    */

    term prog = interp.parse(program);

    interp.load_program(prog);

    std::cout << "Program --------------------------------------\n";
    interp.print_db(std::cout);
    std::cout << "----------------------------------------------\n";

    std::cout << "STATUS: " << interp.status() << std::endl;

    /*
    term qr = interp.parse(query);
    std::cout << "?- " << interp.to_string(qr) << ".\n";

    interp.execute(qr);

    std::string result = interp.to_string(qr);

    std::cout << "----------------------------------------------" << std::endl;
    interp.print_result(std::cout);
    std::cout << "----------------------------------------------" << std::endl;
    */
}

int main( int argc, char *argv[] )
{
    test_up_and_down();
    test_simple_interpreter();
    test_backtracking_interpreter();
    test_interpreter_serialize();
    test_interpreter_multi_instance();
    test_interpreter_freeze_preprocess();

    return 0;
}
