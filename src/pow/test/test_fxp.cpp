#include <iostream>
#include <sstream>
#include <assert.h>
#include <math.h>
#include <algorithm>
#include "../fxp.hpp"
#include "../vec3.hpp"

using namespace prologcoin::pow;

static void header( const std::string &str )
{
    std::cout << "\n";
    std::cout << "--- [" + str + "] " + std::string(60 - str.length(), '-') << "\n";
    std::cout << "\n";
}

static int slow_msb(uint64_t x) {
    int cnt = 0;
    if (x == 0) {
	return -1;
    }
    while ((x >> 63) == 0) {
	cnt++;
	x <<= 1;
    }
    return 63-cnt;
}

static bool tol_check(double d, fxp1648 v, double tol=0.00001)
{
    double vd = v.to_double();
    assert((d < 0) == (vd < 0));
    double rel = fabs(0.5 - fabs(vd) / (fabs(d)+fabs(vd)));
    assert(rel < tol);
    return true;
}

static void test_fxp_msb()
{
    header("test_fxp_msb");

    uint64_t v = 0x1122334455667788;
    for (size_t i = 0; i < 65; i++) {
	if (msb(v) != slow_msb(v)) {
	    std::cout << "ERROR at i: " << i << " MSB: " << msb(v) << " " << slow_msb(v) << std::endl;
	}
	assert( msb(v) == slow_msb(v) );
	v >>= 1;
    }
}

static void test_fxp_mul()
{
    header("test_fxp_mul");

    double d1 = 103.748728273;
    double d2 = 45.878723737219;

    fxp1648 f1(d1);
    fxp1648 f2(d2);

    std::cout.precision(17);
    std::cout << "d1: " << d1 << " f1: " << f1 << std::endl;
    std::cout << "d2: " << d2 << " f2: " << f2 << std::endl;

    std::cout << "d1*d2: " << (d1*d2) << " f(d1*d2): " << fxp1648(d1*d2) << " f1*f2: " << (f1*f2) << std::endl;
    tol_check(d1*d2, f1*f2);

    std::cout << "-0.5: " << fxp1648(-0.5) << std::endl;
    std::cout << "-0.5*0.5: " << fxp1648(-0.5) * fxp1648(0.5) << std::endl;
    tol_check(-0.25, fxp1648(-0.5) * fxp1648(0.5));
}

static void test_fxp_reciprocal()
{
    header("test_fxp_reciprocal");
    
    static const size_t N = 10;
    double ds[N] = { 103.748728273, 45.878723737219, -103.748728273, -45.878723737219, 
		     -0.5, 0.25, -0.1, 0.1, 1.0, -1.0 };

    std::cout.precision(17);

    for (size_t i = 0; i < N; i++) {
	double d = ds[i];
	fxp1648 f(d);
	double d_inv = 1/d;
	fxp1648 f_inv = f.reciprocal();

	std::cout << "I=" << i << " d=" << d << " 1/d=" << d_inv << std::endl;
	std::cout << "I=" << i << " f=" << f << " 1/f=" << f_inv << std::endl;

	tol_check(d_inv, f_inv);
    }
}

static void test_fxp_division()
{
    header("test_fcp_division");

    static const size_t N = 10;
    double qt[N] = { 103.12345678, 47.1287483, 0.17492731, -1.234567, -5.9823673, 0.01234567, -0.01234567, 10.0, 1.0, 42.0 };

    for (size_t i = 0; i < N; i++) {
	for (size_t j = 0; j < N; j++) {
	    double ad = qt[i], bd = qt[j];
	    fxp1648 af(qt[i]), bf(qt[j]);
	    std::cout << "(i=" << i << ",j=" << j << "): ";
	    std::cout << ad << "/" << bd << " = " << ad/bd << std::endl;
	    std::cout << "(i=" << i << ",j=" << j << "): ";
	    std::cout << af << "/" << bf << " = " << af/bf << std::endl;
	    tol_check(ad/bd, af/bf);
	}
    }
}

static void test_fxp_inv_sqrt()
{
    header("test_fxp_inv_sqrt");

    static const size_t N = 10;
    double qt[N] = { 103.12345678, 47.1287483, 0.17492731, 1.234567, 5.9823673, 0.01234567, 0.01234567, 10.0, 1.0, 42.0 };

    for (size_t i = 0; i < N; i++) {
	double d = qt[i];
	fxp1648 f(d);
	std::cout << "double: " << d << ": inv_sqrt()=" << 1.0/::sqrt(d) << std::endl;
        std::cout << "fixed : " << f << ": inv_sqrt()=" << f.inv_sqrt() << std::endl;
	tol_check(1.0/::sqrt(d), f.inv_sqrt());
    }

    std::cout << "Testing range of inv_sqrt([1..+100])" << std::endl;

    double max_err = 0.0;
    double acc_err = 0.0;
    for (size_t i = 1; i < 10000; i++) {
	double d = 100.0*i/10000.0;
	fxp1648 f(d);
	auto err = abs(1.0/::sqrt(d) - f.inv_sqrt().to_double()) / (1.0/::sqrt(d));
	// std::cout << "i=" << i << " err=" << err << std::endl;
	max_err = std::max(err, max_err);
	acc_err += err;
    }
    std::cout << "Maximum err    : " << max_err << std::endl;
    std::cout << "Accumulated err: " << acc_err << std::endl;
}

static void test_fxp_sqrt()
{
    header("test_fxp_sqrt");

    static const size_t N = 10;
    double qt[N] = { 103.12345678, 47.1287483, 0.17492731, 1.234567, 5.9823673, 0.01234567, 0.01234567, 10.0, 1.0, 42.0 };

    for (size_t i = 0; i < N; i++) {

	double d = qt[i];
	fxp1648 f(d);
	std::cout << "double: " << d << ": sqrt()=" << ::sqrt(d) << std::endl;
	std::cout << "fixed : " << f << ": sqrt()=" << f.sqrt()
		  << " err=" << (abs(::sqrt(d) - f.sqrt().to_double())
				 / ::sqrt(d)) << std::endl;
	tol_check(::sqrt(d), f.sqrt());
    }

    std::cout << "Testing range of sqrt([1..+100])" << std::endl;

    double max_err = 0.0;
    double acc_err = 0.0;
    for (size_t i = 1; i < 10000; i++) {
	double d = 100.0*i/10000.0;
	fxp1648 f(d);
	auto err = abs(::sqrt(d) - f.sqrt().to_double()) / ::sqrt(d);
	// std::cout << "i=" << i << " err=" << err << std::endl;
	max_err = std::max(err, max_err);
	acc_err += err;
    }
    std::cout << "Maximum err    : " << max_err << std::endl;
    std::cout << "Accumulated err: " << acc_err << std::endl;
}

#ifndef USE_PROLOGCOIN_FXP_ASSERT
static void test_fxp_saturation()
{
    header("test_fxp_saturation");

    std::cout << "Check min max eps..." << std::endl;

    std::cout << "min: " << fxp1648::min() << std::endl;
    assert(fxp1648::min() == fxp1648(-32768));
    std::cout << "max: " << fxp1648::max() << std::endl;
    assert(fxp1648::min() == fxp1648::raw(fxp1648::max().raw_value() + 1));
    std::cout << "eps: " << fxp1648::eps(true) << std::endl;
    assert(fxp1648::eps(true).to_double() < 4e-15);
    std::cout << "negative eps: " << fxp1648::eps(false) << std::endl;
    assert(fxp1648::eps(false).to_double() < -3e-15);

    fxp1648 a(20000), b(10000);

    std::cout << "mul3: " << a << "*3=" << a.mul3() << std::endl;
    assert(a.mul3() == fxp1648::max());
    std::cout << "mul3: " << -a << "*3=" << (-a).mul3() << std::endl;
    assert((-a).mul3() == fxp1648::min());

    fxp1648 x(100);
    for (size_t i = 0; i < 20; i++) {
	x = x >> 5;
    }
    std::cout << "100 >> (5*20): " << x << std::endl;
    assert(x == fxp1648::eps(true));
    x = -100;
    for (size_t i = 0; i < 20; i++) {
	x = x >> 5;
    }
    std::cout << "(-100) >> (5*20): " << x << std::endl;
    assert(x == fxp1648::eps(false));

    fxp1648 x_start = fxp1648(1) / 100 / 100 / 100;
    x = x_start;
    for (size_t i = 0; i < 20; i++) {
	x = x << 5;
    }
    std::cout << x_start << " << (5*20): " << x << std::endl;
    assert(x == fxp1648::max());

    x_start = fxp1648(-1) / 100 / 100 / 100;
    x = x_start;
    for (size_t i = 0; i < 20; i++) {
	x = x << 5;
    }
    std::cout << x_start << " << (5*20): " << x << std::endl;
    assert(x == fxp1648::min());

    {
	fxp1648 a(-32768), b(1), c(32767), d = fxp1648::max();
	std::cout << a << "+" << b << "=" << (a+b) << std::endl;
	assert((a+b) == -32767);
	std::cout << a << "+(" << (-b) << ")=" << (a+(-b)) << std::endl;
	assert((a+(-b)).is_min());
	std::cout << c << "+" << b << "=" << (c+b) << std::endl;
	assert((c+b).is_max());
	std::cout << d << "+" << b << "=" << (d+b) << std::endl;
	assert((d+b).is_max());
	std::cout << d << "+" << d << "=" << (d+d) << std::endl;
	assert((d+d).is_max());
	std::cout << a << "+" << a << "=" << (a+a) << std::endl;
	assert((a+a).is_min());
    }

    {
	fxp1648 a(-32768), b(1), c(32767), d = fxp1648::max();
	std::cout << a << "-" << b << "=" << (a-b) << std::endl;
	assert((a-b).is_min());
	std::cout << a << "-(" << (-b) << ")=" << (a-(-b)) << std::endl;
	assert((a-(-b)) == -32767);
	std::cout << c << "-" << b << "=" << (c-b) << std::endl;
	assert((c-b) == 32766);
	std::cout << d << "-(" << -b << ")=" << (d-(-b)) << std::endl;
	assert((d-(-b)).is_max());
	std::cout << d << "-(" << (-d) << ")=" << (d-(-d)) << std::endl;
	assert((d-(-d)).is_max());
	std::cout << a << "-(" << a << ")=" << (a-a) << std::endl;
	assert((a-a).is_zero());
    }

    {
	fxp1648 a = fxp1648(1000) / 3;
	fxp1648 b = fxp1648(10000) * 3 / 7;

	double ad = 1000.0/3;
	double bd = 10000.0*3/7;

	std::cout << a << "*" << b << "=" << (a*b) << std::endl;
	assert((a*b).is_max());
	std::cout << (-a) << "*" << b << "=" << ((-a)*b) << std::endl;
	assert(((-a)*b).is_min());
	std::cout << a << "*" << (-b) << "=" << (a*(-b)) << std::endl;
	assert((a*(-b)).is_min());
	std::cout << (-a) << "*" << (-b) << "=" << ((-a)*(-b)) << std::endl;
	assert(((-a)*(-b)).is_max());

	a /= 10;
	b /= 10;
	ad /= 10;
	bd /= 10;

	std::cout << a << "*" << b << "=" << (a*b) << std::endl;
	tol_check(ad*bd, a*b);
	std::cout << (-a) << "*" << b << "=" << ((-a)*b) << std::endl;
	tol_check((-ad)*bd, (-a)*b);
	std::cout << a << "*" << (-b) << "=" << (a*(-b)) << std::endl;
	tol_check(ad*(-bd), a*(-b));
	std::cout << (-a) << "*" << (-b) << "=" << ((-a)*(-b)) << std::endl;
	tol_check((-ad)*(-bd), (-a)*(-b));
    }

    {
	fxp1648 a = fxp1648(10) / 100 / 100 / 100;

	std::cout << "1/" << a << "=" << a.reciprocal() << std::endl;
	assert(a.reciprocal().is_max());
	std::cout << "1/" << (-a) << "=" << (-a).reciprocal() << std::endl;
	assert((-a).reciprocal().is_min());
    }

    {
	fxp1648 r(10000);

	for (size_t i = 0; i < 20; i++, r = r / 10) {
	    std::cout << "sqrt(" << r << ")=" << r.sqrt() << std::endl;
	    if (i < 10) {
		tol_check(::sqrt(r.to_double()), r.sqrt());
	    }
	}
	assert(fxp1648::eps(true).sqrt()  == r.sqrt());
    }

    {
	fxp1648 r(10000);

	for (size_t i = 0; i < 20; i++, r = r / 10) {
	    std::cout << "inv_sqrt(" << r << ")=" << r.inv_sqrt() << std::endl;
	    if (i < 10) {
		tol_check(::sqrt(1.0/r.to_double()), r.inv_sqrt());
	    }
	}
	assert(fxp1648::eps(true).inv_sqrt()  == fxp1648::max());
    }

    {
	// Test some edge cases

	std::cout << "0/0: " << fxp1648(0) / fxp1648(0) << std::endl;
	assert(fxp1648(0)/fxp1648(0) == 0);
	std::cout << "1/0: " << fxp1648(1) / fxp1648(0) << std::endl;
	assert(fxp1648(1)/fxp1648(0) == fxp1648::max());
	std::cout << "-1/0: " << fxp1648(-1) / fxp1648(0) << std::endl;
	assert(fxp1648(-1)/fxp1648(0) == fxp1648::min());
    }

}
#endif

int main(int argc, char *argv[])
{
    test_fxp_msb();
    test_fxp_mul();
    test_fxp_reciprocal();
    test_fxp_division();
    test_fxp_inv_sqrt();
    test_fxp_sqrt();

#ifndef USE_PROLOGCOIN_FXP_ASSERT
    test_fxp_saturation();
#endif

    return 0;
}

