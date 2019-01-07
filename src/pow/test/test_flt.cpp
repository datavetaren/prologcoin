#include <iostream>
#include <sstream>
#include <assert.h>
#include <math.h>
#include <cmath>
#include <iomanip>
#include "../flt.hpp"
#include <boost/algorithm/string.hpp>

using namespace prologcoin::pow;

static void header( const std::string &str )
{
    std::cout << "\n";
    std::cout << "--- [" + str + "] " + std::string(60 - str.length(), '-') << "\n";
    std::cout << "\n";
}

static bool tol_check(double d, flt1648 v, double tol=0.00001)
{
    double vd = v.to_double();
    assert((d < 0) == (vd < 0));
    if (vd == d) {
	return true;
    }
    double rel = fabs(0.5 - fabs(vd) / (fabs(d)+fabs(vd)));
    assert(rel < tol);
    return true;
}

const size_t N = 10;
const double db[N] = { 2.3e-7, 2.134e+10, -5.91352e+5, 2.12345e-3, 2e+256,
		       123.456e-127, 42e-42, -99e+42, -5e-17, -4.321e+19 };
flt1648 fl[N];

class next_count {
public:
    friend std::ostream & operator << (std::ostream &out, const next_count &nc);
};

static int cnt = 0;

inline std::ostream & operator << (std::ostream &out, const next_count &nc) {
    cnt++;
    out << "[" << cnt << "]: ";
    return out;
}

static void test_flt_init()
{
    std::cout.precision(17);

    for (size_t i = 0; i < N; i++) {
	int exp = 0;
	double frac = frexp(db[i], &exp);
	fl[i] = flt1648( exp, fxp1648(frac));
	std::cout << next_count() << "db[" << i << "]=" << db[i] << std::endl;
	std::cout << next_count() << "fl[" << i << "]=" << fl[i] << std::endl;
	tol_check(db[i], fl[i]);
    }
}

static void test_flt_add()
{
    header("test_flt_add");

    for (size_t i = 0; i < N; i++) {
	for (size_t j = 0; j < N; j++) {
	    double ad = db[i], bd = db[j];
	    flt1648 af = fl[i], bf = fl[j];
	    std::cout << next_count() << ad << "+" << bd << "=" << (ad+bd) << std::endl;
	    std::cout << next_count() << af << "+" << bf << "=" << (af+bf) << std::endl;
	    tol_check((ad+bd), (af+bf));
	}
    }
}


static void test_flt_sub()
{
    header("test_flt_sub");

    for (size_t i = 0; i < N; i++) {
	for (size_t j = 0; j < N; j++) {
	    double ad = db[i], bd = db[j];
	    flt1648 af = fl[i], bf = fl[j];
	    std::cout << next_count() << ad << "-" << bd << "=" << (ad-bd) << std::endl;
	    std::cout << next_count() << af << "-" << bf << "=" << (af-bf) << std::endl;
	    tol_check((ad-bd), (af-bf));
	}
    }
}

static void test_flt_mul()
{
    header("test_flt_mul");

    for (size_t i = 0; i < N; i++) {
	for (size_t j = 0; j < N; j++) {
	    double ad = db[i], bd = db[j];
	    flt1648 af = fl[i], bf = fl[j];
	    std::cout << next_count() << ad << "*" << bd << "=" << (ad*bd) << std::endl;
	    std::cout << next_count() << af << "*" << bf << "=" << (af*bf) << std::endl;
	    tol_check((ad*bd), (af*bf));
	}
    }
}

static void test_flt_div()
{
    header("test_flt_div");

    for (size_t i = 0; i < N; i++) {
	for (size_t j = 0; j < N; j++) {
	    double ad = db[i], bd = db[j];
	    flt1648 af = fl[i], bf = fl[j];
	    std::cout << next_count() << ad << "/" << bd << "=" << (ad/bd) << std::endl;
	    std::cout << next_count() << af << "/" << bf << "=" << (af/bf) << std::endl;
	    tol_check((ad/bd), (af/bf));
	}
    }
}

static void test_flt_reciprocal()
{
    header("test_flt_reciprocal");

    for (size_t i = 0; i < N; i++) {
	double ad = db[i];
	flt1648 af = fl[i];
	double ad_reciprocal = 1.0 / ad;
	flt1648 af_reciprocal = af.reciprocal();
	std::cout << next_count() << ad << " reciprocal=" << ad_reciprocal << std::endl;
	std::cout << next_count() << af << " reciprocal=" << af_reciprocal << std::endl;
	tol_check(ad_reciprocal, af_reciprocal);
    }
}

static void test_flt_values()
{
    header("test_flt_values");

    auto v = flt1648::from(12, 1234567);

    std::cout << "VALUE: " << v.to_double() << std::endl;

    tol_check(12.1234567, v);
}

static void test_bitcoin_difficulty()
{
    header("test_bitcoin_difficulty");

    auto base_value = flt1648(0x0404cb);
    auto mult_value = flt1648(1) << (8*(0x1b - 3));
    auto target_value = base_value * mult_value;
    std::string expect = "0x00000000000404CB000000000000000000000000000000000000000000000000";
    auto target = "0x" + boost::to_upper_copy(target_value.to_integer_string(32));
    std::cout << "TARGET: " << target << std::endl;
    std::cout << "EXPECT: " << expect << std::endl;
    assert( expect == target );

    auto max_value = flt1648(0x00ffff) * (flt1648(1) << (8*(0x1d - 3)));
    auto difficulty = max_value / target_value;
    std::cout << "Difficulty: " << difficulty.to_double() << std::endl;
    std::cout << "Expect    : 16307.42..." << std::endl;

    assert(abs(difficulty.to_double() - 16307.42) < 0.01);
}

static void test_our_difficulty()
{
    header("test_out_difficulty");

    auto base_value = flt1648(16307) << 32;
    auto target_value = flt1648(1) / base_value;
    auto difficulty = flt1648(1) - target_value;

    std::cout << "DIFFICULTY: " << difficulty.to_double() << std::endl;

    auto rel_target = flt1648(1) - difficulty;

    std::cout << "REL TARGET: " << rel_target.to_double() << std::endl;
    
    auto max_target = flt1648(1) << 256;
    auto target = rel_target * max_target;

    std::cout << "TARGET    : " << target.to_double() << std::endl;

    std::cout << "TARGET INT: 0x" << target.to_integer_string(32) << std::endl;
}

int main(int argc, char *argv[])
{
    test_flt_init();
    test_flt_add();
    test_flt_sub();
    test_flt_mul();
    test_flt_div();
    test_flt_reciprocal();
    test_flt_values();
    test_bitcoin_difficulty();
    test_our_difficulty();

    return 0;
}

