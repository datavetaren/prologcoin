#include <iostream>
#include <iomanip>
#include <assert.h>
#include <vector>
#include <math.h>
#include <common/fast_hash.hpp>

using namespace prologcoin::common;

static void header( const std::string &str )
{
    std::cout << "\n";
    std::cout << "--- [" + str + "] " + std::string(60 - str.length(), '-') << "\n";
    std::cout << "\n";
}

static double igf(double s, double z)
{
    if (z < 0.0) {
	return 0.0;
    }

    double sc = (1.0/s)*pow(z,s)*exp(-z);
    double sum = 1.0;
    double q = 1.0;

    for (size_t i = 0; i < 200; i++) {
	q *= (z/(s+1));
	sum += q;
	s++;
    }
    return sum * sc;
}

static double p_value(int dof, double chi2)
{
    double k = 0.5*static_cast<double>(dof);
    double x = 0.5*chi2;
    double p = igf(k, x);
    if (isnan(p) || isinf(p) || p < 1e-8) {
	return 1e-14;
    }
    return 1.0 - (p / tgamma(k));
}

static void test_fast_hash()
{
    header( "test_fast_hash" );

    static const size_t B = 16;
    static const size_t N = 100;
 
    std::vector<size_t> buckets(B);

    // Put B*N values in B buckets. Expected number of
    // values in each bucket should be ~N.

    for (size_t i = 0; i < B*N; i++) {
	fast_hash h;
	uint32_t v = h << 123 << "foo" << "bar" << i;
	buckets[v % B]++;
    }

    // Compute the chi2 variance. E=N, Oi = number of elements in bucket.
    double E = N;
    double chi2 = 0;
    for (size_t i = 0; i < B; i++) {
	double Oi = buckets[i];
	chi2 += (Oi-E)*(Oi-E)/E;
    }

    double pv = p_value(B-1, chi2);
    std::cout << "p-value: " << pv << " (must be >= 0.2) \n";

    // It should be a relatively high probability that you'll get
    // the expected value...
    assert(pv >= 0.2);
}

int main(int argc, char *argv[])
{
    test_fast_hash();

    return 0;
}
