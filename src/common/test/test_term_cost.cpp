#include <iostream>
#include <iomanip>
#include <assert.h>
#include <common/term_env.hpp>
#include <common/term_ops.hpp>

using namespace prologcoin::common;

static void header( const std::string &str )
{
    std::cout << "\n";
    std::cout << "--- [" + str + "] " + std::string(60 - str.length(), '-') << "\n";
    std::cout << "\n";
}

static void test_cost1()
{
    header( "test_cost1()" );

    term_env env;
    std::string str = "append([],Zs,Zs).";
    auto t = env.parse(str);
    uint64_t cost = env.cost(t);
    
    std::cout << "TERM: " << env.to_string(t) << "\n";
    std::cout << "COST: " << cost << "\n";
}

int main( int argc, char *argv[] )
{
    test_cost1();

    return 0;
}



