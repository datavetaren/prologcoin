#include <iostream>
#include <iomanip>
#include <assert.h>
#include <common/term.hpp>
#include <common/term_ops.hpp>
#include <common/term_emitter.hpp>

using namespace prologcoin::common;

static void header( const std::string &str )
{
    std::cout << "\n";
    std::cout << "--- [" + str + "] " + std::string(60 - str.length(), '-') << "\n";
    std::cout << "\n";
}

static void test_simple_term()
{
    header("test_simple_term()");

    heap h;
    term_ops ops;
    std::stringstream ss;
    term_emitter emit(ss, h, ops);

    con_cell h_2("h", 2);
    con_cell f_1("f", 1);
    con_cell p_3("p", 3);

    auto h_2_str = h.new_str(h_2);
    auto Z = h.arg(h_2_str, 0);
    auto W = h.arg(h_2_str, 1);
    auto f_1_str = h.new_str(f_1);
    h.set_arg(f_1_str, 0, W);
    auto p_3_str = h.new_str(p_3);
    h.set_arg(p_3_str, 0, Z);
    h.set_arg(p_3_str, 1, h_2_str);
    h.set_arg(p_3_str, 2, f_1_str);

    emit.print(p_3_str);

    std::cout << ss.str() << "\n";

    assert(ss.str() == "p(C, h(C, D), f(D))");
}

static size_t my_rand(size_t bound)
{
    static uint64_t state = 4711;

    if (bound == 0) {
	state = 4711;
	return 0;
    }
    
    state = 13*state + 734672631;

    return state % bound;
}

static term new_term(heap &heap, size_t max_depth, size_t depth = 0)
{
    size_t arity = (depth >= max_depth) ? 0 : my_rand(6);
    char functorName[2];
    functorName[0] = 'a' + (char)arity;
    functorName[1] = '\0';
    auto str = heap.new_str(con_cell(functorName, arity));
    for (size_t j = 0; j < arity; j++) {
	auto arg = new_term(heap, max_depth, depth+1);
	heap.set_arg(str, j, arg);
    }
    return str;
}

const char *BIG_TERM_GOLD =
"e(b(e(b(e(a, a, a, a)), b(e(a, a, a, a)), b(e(a, a, a, a)), b(c(a, a)))), \n"
"  f(e(b(e(a, a, a, a)), f(a, f(a, a, a, a, a), a, f(a, a, a, a, a), a), \n"
"      b(e(a, a, a, a)), b(e(a, a, a, a))), \n"
"    d(c(b(a), c(a, a)), f(a, b(a), a, d(a, a, a), e(a, a, a, a)), \n"
"      f(c(a, a), b(a), c(a, a), f(a, a, a, a, a), a)), \n"
"    b(e(f(a, a, a, a, a), e(a, a, a, a), b(a), c(a, a))), \n"
"    f(e(f(a, a, a, a, a), c(a, a), b(a), e(a, a, a, a)), b(c(a, a)), \n"
"      f(c(a, a), f(a, a, a, a, a), a, b(a), e(a, a, a, a)), \n"
"      d(a, d(a, a, a), e(a, a, a, a)), b(e(a, a, a, a))), \n"
"    d(c(b(a), c(a, a)), b(e(a, a, a, a)), d(e(a, a, a, a), d(a, a, a), a))), \n"
"  b(e(d(e(a, a, a, a), f(a, a, a, a, a), c(a, a)), \n"
"      f(c(a, a), f(a, a, a, a, a), c(a, a), b(a), e(a, a, a, a)), \n"
"      b(e(a, a, a, a)), d(e(a, a, a, a), b(a), c(a, a)))), \n"
"  f(a, \n"
"    d(a, d(e(a, a, a, a), f(a, a, a, a, a), a), \n"
"      f(c(a, a), b(a), e(a, a, a, a), b(a), a)), \n"
"    d(a, d(c(a, a), b(a), c(a, a)), b(e(a, a, a, a))), b(a), \n"
"    d(a, b(a), \n"
"      f(e(a, a, a, a), d(a, a, a), c(a, a), d(a, a, a), e(a, a, a, a)))))\n";

static std::string cut(const char *from, const char *to)
{
    std::string r;
    for (const char *p = from; p < to; p++) {
	r += (char)*p;
    }
    return r;
}

static void test_big_term()
{
    header("test_big_term()");

    heap h;

    const size_t DEPTH = 5;

    cell term = new_term(h, DEPTH);

    std::stringstream ss;
    term_ops ops;
    term_emitter emitter(ss, h, ops);
    emitter.set_max_column(78);
    emitter.print(term);
    ss << "\n";

    std::string str = ss.str();

    const char *goldScan = BIG_TERM_GOLD;
    const char *actScan = str.c_str();

    size_t lineCnt = 0;
    bool err = false;

    while (goldScan[0] != '\0') {
	const char *goldNextLine = strchr(goldScan, '\n');
	const char *actNextLine = strchr(actScan, '\n');

	if (goldNextLine == NULL && actNextLine != NULL) {
	    std::cout << "There was no next line in gold template.\n";
	    std::cout << "Actual: " << cut(actScan, actNextLine) << "\n";
	    err = true;
	    break;
	}
	if (goldNextLine != NULL && actNextLine == NULL) {
	    std::cout << "Actual feed terminated to early.\n";
	    std::cout << "Expect: " << cut(goldScan, goldNextLine) << "\n";
	    err = true;
	    break;
	}
	size_t goldNextLineLen = goldNextLine - goldScan;
	size_t actNextLineLen = actNextLine - actScan;
	if (goldNextLineLen != actNextLineLen) {
	    std::cout << "Difference at line " << lineCnt << ":\n";
	    std::cout << "(Due to different lengths: ExpectLen: " << goldNextLineLen << " ActualLen: "<< actNextLineLen << ")\n";
	    std::cout << "Actual: " << cut(actScan, actNextLine) << "\n";
	    std::cout << "Expect: " << cut(goldScan, goldNextLine) << "\n";
	    err = true;
	    break;
	}
	std::string actual = cut(actScan, actNextLine);
	std::string expect = cut(goldScan, goldNextLine);
	if (actual != expect) {
	    std::cout << "Difference at line " << lineCnt << ":\n";
	    for (size_t i = 0; i < actual.length(); i++) {
		if (actual[i] != expect[i]) {
		    std::cout << "(at column " << i << ")\n";
		    break;
		}
	    }
	    std::cout << "Actual: " << actual << "\n";
	    std::cout << "Expect: " << expect << "\n";
	    err = true;
	    break;
	}
	goldScan = &goldNextLine[1];
	actScan = &actNextLine[1];
	lineCnt++;
    }

    if (!err) {
	std::cout << str;
	std::cout << "OK\n";
    }
    std::cout << "\n";

    assert(!err);
}

static void test_ops()
{
    header("test_ops()");

    heap h;
    term_ops ops;
    std::stringstream ss;
    term_emitter emit(ss, h, ops);

    con_cell plus_2("+", 2);
    con_cell times_2("*", 2);
    con_cell mod_2("mod", 2);
    con_cell pow_2("^", 2);
    con_cell minus_1("-", 1);

    auto plus_expr = h.new_str(plus_2);
    auto plus_expr1 = h.new_str(plus_2);
    h.set_arg(plus_expr1, 0, int_cell(1));
    h.set_arg(plus_expr1, 1, int_cell(2));
    h.set_arg(plus_expr, 0, plus_expr1);
    h.set_arg(plus_expr, 1, int_cell(3));

    auto times_expr = h.new_str(times_2);
    h.set_arg(times_expr, 0, int_cell(42));
    h.set_arg(times_expr, 1, plus_expr);

    auto mod_expr = h.new_str(mod_2);
    h.set_arg(mod_expr, 0, times_expr);
    h.set_arg(mod_expr, 1, int_cell(100));

    auto pow_expr = h.new_str(pow_2);
    h.set_arg(pow_expr, 0, mod_expr);
    h.set_arg(pow_expr, 1, int_cell(123));

    auto minus1_expr = h.new_str(minus_1);
    h.set_arg(minus1_expr, 0, pow_expr);

    auto minus1_expr1 = h.new_str(minus_1);
    h.set_arg(minus1_expr1, 0, minus1_expr);

    emit.print(minus1_expr1);
    std::cout << "Expr: " << ss.str() << "\n";
    assert( ss.str() == "- - (42*(1+2+3) mod 100)^123" );
}

static void test_bignum()
{
    using namespace boost::multiprecision;

    header("test_bignum()");

    cpp_int i("123456789123456789123456789123456789123456789123456789123456789123456789");

    // Construct foo(...) with ... as a bignum
    heap h;
    big_cell big = h.new_big(i);

    h.print(std::cout);

    con_cell foo_1("foo", 1);
    auto foo_str = h.new_str(foo_1);
    h.set_arg(foo_str, 0, big);

    // Check that it emits in base 58 if it cannot be fit within an int
    term_ops ops;
    std::stringstream ss;
    term_emitter emit(ss, h, ops);
    emit.print(foo_str);

    std::string expect = "foo(58'4atLG7Hb9u2NH7HrRBedKHJ5hQ3z4QQcEWA3b8ACU)";
    std::string actual = ss.str();
    
    std::cout << "Actual: " << actual << std::endl;
    std::cout << "Expect: " << expect << std::endl;

    assert(actual == expect);

    // Check that a small integer is still displayed in base 10

    // Construct foo(...) with ... as a bignum
    cpp_int i2("123456789");
    big_cell big2 = h.new_big(i2);
    auto foo_str2 = h.new_str(foo_1);
    h.set_arg(foo_str2, 0, big2);

    // Check that it emits in base 58 if it cannot be fit within an int
    std::stringstream ss2;
    term_emitter emit2(ss2, h, ops);
    emit2.print(foo_str2);

    std::string expect2 = "foo(123456789)";
    std::string actual2 = ss2.str();
    
    std::cout << "Actual: " << actual2 << std::endl;
    std::cout << "Expect: " << expect2 << std::endl;

    assert(actual2 == expect2);

}

int main(int argc, char *argv[])
{
    test_simple_term();
    test_big_term();
    test_ops();
    test_bignum();

    return 0;
}
