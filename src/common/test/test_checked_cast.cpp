#include <iostream>
#include <iomanip>
#include <assert.h>
#include <common/checked_cast.hpp>
#include <boost/lexical_cast.hpp>

using namespace prologcoin::common;

static void header( const std::string &str )
{
    std::cout << "\n";
    std::cout << "--- [" + str + "] " + std::string(60 - str.length(), '-') << "\n";
    std::cout << "\n";
}

static void test_checked_cast()
{
    header("test_checked_cast()");

    int x = 100000;
    try {
	checked_cast<unsigned short>(x);
	assert("Checked cast was supposed to error" == nullptr);
    } catch (checked_cast_exception &ex) {
	std::string expect = "Value 100000 was greater than maximum 65535";
	std::cout << "Actual: " << ex.what() << std::endl;
	std::cout << "Expect: " << expect << std::endl;
	assert(ex.what() == expect);
    } catch (...) {
	assert("Unknown exception" == nullptr);
    }

    x = -1;
    try {
	checked_cast<unsigned short>(x);
	assert("Checked cast was supposed to error" == nullptr);
    } catch (checked_cast_exception &ex) {
	std::string expect = "Value -1 was less than minimum 0";
	std::cout << "Actual: " << ex.what() << std::endl;
	std::cout << "Expect: " << expect << std::endl;
	assert(ex.what() == expect);
    } catch (...) {
	assert("Unknown exception" == nullptr);
    }

    x = 1000;
    try {
	checked_cast<unsigned short>(x, 0, 100);
	assert("Checked cast was supposed to error" == nullptr);
    } catch (checked_cast_exception &ex) {
	std::string expect = "Value 1000 was greater than maximum 100";
	std::cout << "Actual: " << ex.what() << std::endl;
	std::cout << "Expect: " << expect << std::endl;
	assert(ex.what() == expect);
    } catch (...) {
	assert("Unknown exception" == nullptr);
    }

    x = 2;
    try {
	checked_cast<unsigned short>(x, 5, 100);
	assert("Checked cast was supposed to error" == nullptr);
    } catch (checked_cast_exception &ex) {
	std::string expect = "Value 2 was less than minimum 5";
	std::cout << "Actual: " << ex.what() << std::endl;
	std::cout << "Expect: " << expect << std::endl;
	assert(ex.what() == expect);
    } catch (...) {
	assert("Unknown exception" == nullptr);
    }
}

int main(int argc, char *argv[])
{
    test_checked_cast();

    return 0;
}
