#include "../../common/term_tools.hpp"
#include "../wam_interpreter.hpp"

using namespace prologcoin::common;
using namespace prologcoin::interp;

static void header( const std::string &str )
{
    std::cout << "\n";
    std::cout << "--- [" + str + "] " + std::string(60 - str.length(), '-') << "\n";
    std::cout << "\n";
}

namespace prologcoin { namespace interp {

class test_wam_interpreter
{
public:
    test_wam_interpreter() { }

    void test_flatten();

private:
    wam_interpreter interp;
};

}}

void test_wam_interpreter::test_flatten()
{
    term t = interp.parse("f(g(y,12,h(k),i(2)),m(X)).");
    auto fl = interp.flatten(t);
    interp.print_prims(fl);
}

static void test_flatten()
{
    header("test_flatten");

    test_wam_interpreter wam;
    wam.test_flatten();
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
    interp.add(wam_instruction<PUT_STRUCTURE>(con_cell("f",2), 12));
    interp.add(wam_instruction<PUT_LIST>(13));
    interp.add(wam_instruction<PUT_CONSTANT>(con_cell("foo",0), 14));
    interp.add(wam_instruction<PUT_CONSTANT>(int_cell(4711), 15));
    interp.add(wam_instruction<GET_VARIABLE_X>(16,17));
    interp.add(wam_instruction<GET_VARIABLE_Y>(16,17));
    interp.add(wam_instruction<GET_VALUE_X>(18,19));
    interp.add(wam_instruction<GET_VALUE_Y>(20,21));
    interp.add(wam_instruction<GET_STRUCTURE>(con_cell("g",2), 22));
    interp.add(wam_instruction<GET_LIST>(23));
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
    interp.add(wam_instruction<CALL>(con_cell("f",3), nullptr, 10));
    interp.add(wam_instruction<EXECUTE>(con_cell("g",4), nullptr));
    interp.add(wam_instruction<PROCEED>());
    interp.add(wam_instruction<TRY_ME_ELSE>(interp.to_code(41)));
    interp.add(wam_instruction<RETRY_ME_ELSE>(interp.to_code(47)));
    interp.add(wam_instruction<TRUST_ME>());
    interp.add(wam_instruction<TRY>(interp.to_code(53)));
    interp.add(wam_instruction<RETRY>(interp.to_code(58)));
    interp.add(wam_instruction<TRUST>(interp.to_code(63)));
    interp.add(wam_instruction<SWITCH_ON_TERM>(interp.to_code(0),
					       interp.to_code(5),
					       nullptr,
					       interp.to_code(10)));

    auto *hm1 = interp.new_hash_map();
    hm1->insert(std::make_pair(con_cell("f", 2), interp.to_code(126)));
    hm1->insert(std::make_pair(int_cell(1234), interp.to_code(207)));
    interp.add(wam_instruction<SWITCH_ON_CONSTANT>(hm1));

    auto *hm2 = interp.new_hash_map();
    hm2->insert(std::make_pair(con_cell("f", 2), interp.to_code(221)));
    hm2->insert(std::make_pair(con_cell("g", 3), interp.to_code(235)));
    interp.add(wam_instruction<SWITCH_ON_STRUCTURE>(hm2));

    interp.add(wam_instruction<NECK_CUT>());
    interp.add(wam_instruction<GET_LEVEL>(111));
    interp.add(wam_instruction<CUT>(112));

    interp.print_code(std::cout);
}

int main( int argc, char *argv[] )
{
    test_flatten();

    test_instruction_sequence();

    return 0;
}
