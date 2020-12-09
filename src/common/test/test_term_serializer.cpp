#include <iostream>
#include <iomanip>
#include <assert.h>
#include <common/term_env.hpp>
#include <common/term_serializer.hpp>

using namespace prologcoin::common;

static void header( const std::string &str )
{
    std::cout << "\n";
    std::cout << "--- [" + str + "] " + std::string(60 - str.length(), '-') << "\n";
    std::cout << "\n";
}

static void test_term_serializer_simple()
{
    header( "test_term_serializer_simple()" );

    term_env env;
    term t = env.parse("foo(1, bar(kallekula, [1,2,baz]), Foo, kallekula, world, test4711, Foo, Bar, Bar, Bar, Bar).");

    auto str1 = env.to_string(t);

    std::cout << "WRITE TERM: " << str1 << "\n";

    term_serializer ser(env);
    term_serializer::buffer_t buf;
    ser.write(buf, t);

    ser.print_buffer(buf, buf.size());

    term_env env2;
    term_serializer ser2(env2);

    term t2;
    try {
	t2 = ser2.read(buf);
    } catch (serializer_exception &ex) {
	std::cout << "EXCEPTION WHEN READING: " << ex.what() << "\n";
	std::cout << "Here's the data...\n";
	ser.print_buffer(buf, buf.size());
	assert("No exception expected" == nullptr);
    }
    auto str2 = env2.to_string(t2);

    std::cout << "READ TERM:  " << str2 << "\n";
    
    assert(str1 == str2);
}


static void test_term_serializer_bignum()
{
    header( "test_term_serializer_bignum()" );

    term_env env;
    term t = env.parse("foo(1, bar(58'4atLG7Hb9u2NH7HrRBedKHJ5hQ3z4QQcEWA3b8ACU), baz(16'110022003300440055006600770088009900AA00BB00CC00DD00EE00FF), Var).");
    auto str1 = env.to_string(t);

    std::cout << "WRITE TERM: " << str1 << "\n";

    term_serializer ser(env);
    term_serializer::buffer_t buf;
    ser.write(buf, t);

    ser.print_buffer(buf, buf.size());

    term_env env2;
    term_serializer ser2(env2);

    term t2;
    try {
	t2 = ser2.read(buf);
    } catch (serializer_exception &ex) {
	std::cout << "EXCEPTION WHEN READING: " << ex.what() << "\n";
	std::cout << "Here's the data...\n";
	ser.print_buffer(buf, buf.size());
	assert("No exception expected" == nullptr);
    }
    auto str2 = env2.to_string(t2);

    std::cout << "READ TERM:  " << str2 << "\n";
    
    assert(str1 == str2);

    // Create a big num that span across heap blocks
    term_env env3;

    const size_t num_bits3 = 4096*64;
    const size_t num_bytes3 = num_bits3/8;
    size_t cnt = 0, p = 251;
    term big3 = env3.new_big(num_bits3);
    uint8_t bytes3[num_bytes3];
    for (size_t i = 0; i < num_bytes3; i++, cnt++) {
	if (cnt == p) cnt = 0;
	bytes3[i] = cnt;
    }
    env3.set_big(big3, bytes3, num_bytes3);
    const size_t num_bits4 = 8192*64;
    const size_t num_bytes4 = num_bits4/8;
    cnt = 0;
    term big4 = env3.new_big(num_bits4);
    uint8_t bytes4[num_bytes4];
    for (size_t i = 0; i < num_bytes4; i++, cnt++) {
	if (cnt == p) cnt = 0;
	bytes4[i] = cnt;
    }
    env3.set_big(big4, bytes4, num_bytes4);

    term_serializer::buffer_t buf3;
    term_serializer ser3(env3);
    term t3 = env3.new_term( con_cell("f", 2),
			      {big3, big4});
    ser3.write(buf3, t3);

    term_env env4;
    term_serializer ser4(env4);
    term t4;
    try {
	t4 = ser4.read(buf3);
    } catch (serializer_exception &ex) {
	std::cout << "EXCEPTION WHEN READING: " << ex.what() << "\n";
	std::cout << "Here's the data...\n";
	ser.print_buffer(buf3, buf3.size());
	assert("No exception expected" == nullptr);	
    }

    assert(t4.tag() == tag_t::STR);
    assert(env4.functor(t4) == con_cell("f",2));
    term big3_cmp = env4.arg(t4,0);
    term big4_cmp = env4.arg(t4,1);    
    assert(big3_cmp.tag() == tag_t::BIG);
    assert(big4_cmp.tag() == tag_t::BIG);
    uint8_t bytes3_cmp[num_bytes3];
    env4.get_big(big3_cmp, bytes3_cmp, num_bytes3);
    assert(memcmp(bytes3, bytes3_cmp, num_bytes3) == 0);
    uint8_t bytes4_cmp[num_bytes4];
    env4.get_big(big4_cmp, bytes4_cmp, num_bytes4);

    env4.get_heap().print(std::cout);
    
    assert(memcmp(bytes4, bytes4_cmp, num_bytes4) == 0);    
}

static void test_term_serializer_clause()
{
    header( "test_term_serializer_clause()" );

    term_env env;
    term t = env.parse("setup_numbers(N, M) :- T is N*M, write(generate_numbers), nl, generate_numbers(T, Xs), write(split_numbers), nl, split_numbers(Xs, M, Ys), write('sort chunks'), nl, findall(Y, (member(X, Ys), sort(X, Y)), Cs), write('store numbers'), nl, store_numbers(Cs, 0).");
    auto str1 = env.to_string(t);

    std::cout << "WRITE TERM: " << str1 << "\n";

    term_serializer ser(env);
    term_serializer::buffer_t buf;
    ser.write(buf, t);

    ser.print_buffer(buf, buf.size());

    term_env env2;
    term_serializer ser2(env2);

    term t2;
    try {
	t2 = ser2.read(buf);
    } catch (serializer_exception &ex) {
	std::cout << "EXCEPTION WHEN READING: " << ex.what() << "\n";
	std::cout << "Here's the data...\n";
	ser.print_buffer(buf, buf.size());
	assert("No exception expected" == nullptr);
    }
    auto str2 = env2.to_string(t2);

    std::cout << "READ TERM:  " << str2 << "\n";
    
    assert(str1 == str2);
}


namespace prologcoin { namespace common { namespace test {

class test_term_serializer {
public:

    static void test_exception(const std::string &label,
			       std::initializer_list<cell> init,
			       const std::string &expect_str)
    {
	term_env env;

	term_serializer::buffer_t buffer;
	term_serializer ser(env);
	size_t offset = 0;
	for (auto c : init) {
	    ser.write_cell(buffer, offset, c);
	    offset += sizeof(cell);
	}

	try {
	    static_cast<void>(ser.read(buffer));

	    std::cout << label << ": actual: no exception; expected: " << expect_str << "\n";
	    std::cout << "Here's the data:\n";
	    ser.print_buffer(buffer, buffer.size());

	    assert("No exception as expected" == nullptr);

	} catch (serializer_exception &ex) {
	    std::string actual_str = ex.what();
	    std::cout << label << ": actual: " << actual_str << "; expected: " << expect_str << "\n";
	    bool ok = actual_str.find(expect_str) != std::string::npos;
	    if (!ok) {
		std::cout << "Here's the data:\n";
		ser.print_buffer(buffer, buffer.size());
	    }
	    assert(ok);
	}
    }
};

}}}

using namespace prologcoin::common::test;

static void test_term_serializer_exceptions()
{
    header( "test_term_serializer_exceptions()" );

    test_term_serializer::test_exception("UNSUPPORTED VERSION",
					 {con_cell("ver0",0)},
					 "Unsupported version");

    test_term_serializer::test_exception("WRONG REMAP",
					 {con_cell("ver1",0),
					  con_cell("haha",1)},
					 "remap section");

    test_term_serializer::test_exception("MISSING PAMER",
					 {con_cell("ver1",0),
					  con_cell("remap",0)},
					 "Unexpected end");

    test_term_serializer::test_exception("INDEX1",
					 {con_cell("ver1",0),
					  con_cell("remap",0),
					  str_cell(123)},
					 "ref/con in remap section");

    test_term_serializer::test_exception("INDEX2",
					 {con_cell("ver1",0),
					  con_cell("remap",0),
					  ref_cell(123),
			  		  int(4711)},
					 "expected encoded string");

    test_term_serializer::test_exception("INDEX3",
					 {con_cell("ver1",0),
					  con_cell("remap",0),
					  ref_cell(123),
				 	  int_cell::encode_str("frotzba",true),
					  int_cell::encode_str("f",false),
					  str_cell(456)
					  },
					 "ref/con in remap section");

    test_term_serializer::test_exception("INDEX4",
					 {con_cell("ver1",0),
					  con_cell("remap",0),
					  con_cell("pamer",0),
			 	 	  ref_cell(0),
					  },
					 "Missing index entry for 0:REF");

    test_term_serializer::test_exception("INDEX5",
					 {con_cell("ver1",0),
					  con_cell("remap",0),
					  con_cell("pamer",0),
				 	  con_cell(123,2),
					  },
					 "Missing index entry");

    test_term_serializer::test_exception("DANGLING1",
					 {con_cell("ver1",0),
					  con_cell("remap",0),
					  con_cell("pamer",0),
				 	  str_cell(123),
					  },
					 "Dangling pointer");

    test_term_serializer::test_exception("FUNCTORERR1",
					 {con_cell("ver1",0),
					  con_cell("remap",0),
					  con_cell("pamer",0),
				 	  str_cell(4),
				 	  int_cell(123)
					  },
					 "Illegal functor 123:INT");


    test_term_serializer::test_exception("ARGERR1",
					 {con_cell("ver1",0),
					  con_cell("remap",0),
					  con_cell("pamer",0),
				 	  str_cell(5),
				 	  str_cell(6),
					  con_cell("f",1),
					  con_cell("g",1)
					  },
					 "Erroneous argument g/1:CON");

    test_term_serializer::test_exception("ARGERR2",
					 {con_cell("ver1",0),
					  con_cell("remap",0),
					  con_cell("pamer",0),
				 	  str_cell(4),
					  con_cell("f",1)
					  },
					 "Missing argument for f/1:CON");

    test_term_serializer::test_exception("DANGLING2",
					 {con_cell("ver1",0),
					  con_cell("remap",0),
					  con_cell("pamer",0),
				 	  str_cell(5),
				 	  str_cell(7),
					  con_cell("f",1),
					  con_cell("g",0)
					  },
					 "Dangling pointer for 7:STR");

    test_term_serializer::test_exception("SELFERR1",
					 {con_cell("ver1",0),
					  con_cell("remap",0),
					  con_cell("pamer",0),
				 	  str_cell(3),
					  con_cell("f",1)
					  },
					 "Illegal functor 3:STR");

    test_term_serializer::test_exception("CYCLIC1",
					 {con_cell("ver1",0),
					  con_cell("remap",0),
					  con_cell("pamer",0),
				 	  ref_cell(4),
					  ref_cell(3)
					  },
					 "Cyclic reference for 4:REF");

    test_term_serializer::test_exception("BIGNUM1",
					 {con_cell("ver1",0),
					  con_cell("remap",0),
					  con_cell("pamer",0),
				 	  big_cell(4),
					  ref_cell(3)
					  },
					 "Illegal data 3:REF");


    test_term_serializer::test_exception("BIGNUM2",
					 {con_cell("ver1",0),
					  con_cell("remap",0),
					  con_cell("pamer",0),
				 	  big_cell(4),
					  dat_cell(0)
					  },
					 "is too small");

    test_term_serializer::test_exception("BIGNUM3",
					 {con_cell("ver1",0),
					  con_cell("remap",0),
					  con_cell("pamer",0),
				 	  big_cell(4),
				 	  dat_cell(256),
					  int_cell(0)
					  },
					 "exceeds length of serialized data");


}

int main( int argc, char *argv[] )
{
    test_term_serializer_simple();
    test_term_serializer_bignum();
    test_term_serializer_clause();
    test_term_serializer_exceptions();

    return 0;
}
