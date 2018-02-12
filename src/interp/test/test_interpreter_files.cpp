#include "../../common/test/test_home_dir.hpp"
#include "../../common/term_tools.hpp"
#include "../../common/term_parser.hpp"
#include "../interpreter.hpp"
#include <fstream>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

using namespace prologcoin::common;
using namespace prologcoin::interp;

static bool do_compile = true;

static void header( const std::string &str )
{
    std::cout << "\n";
    std::cout << "--- [" + str + "] " + std::string(60 - str.length(), '-') << "\n";
    std::cout << "\n";
}

static std::vector<std::string> parse_x(const std::string &key, std::string &comments)
{
    std::vector<std::string> matched;
    size_t i = 0;
    while (i != std::string::npos) {
	size_t i1 = comments.find(key, i);
	if (i1 == std::string::npos) {
	    break;
	}
	size_t i2 = comments.find(key, i1+key.size());
	std::string exp;
	if (i2 == std::string::npos) {
  	    exp = comments.substr(i1+key.size());
	} else {
	    exp = comments.substr(i1+key.size(),(i2-i1));
	}
	// Remove anything from %
	exp = exp.substr(0, exp.find_first_of("%"));
	// Remove trailing whitespace
	boost::trim(exp);
	matched.push_back(exp);
	i = i2;
    }
    return matched;
}

static std::vector<std::string> parse_expected(std::string &comments)
{
    return parse_x("Expect:", comments);
}

static std::vector<std::string> parse_meta(std::string &comments)
{
    return parse_x("Meta:", comments);
}

static void process_meta(interpreter &interp, std::string &comments,
			 std::unordered_map<std::string, int> &opt)
{
    auto meta = parse_meta(comments);
    for (auto cmd : meta) {
        if (cmd == "debug on") {
  	    interp.set_debug(true);
        } else if (cmd == "debug off") {
	    interp.set_debug(false);
	} else if (cmd == "debug enabled") {
	    interp.set_debug_enabled();
	} else if (cmd == "fileio on") {
	    interp.enable_file_io();
	} else if (cmd == "WAM-only") {
            opt["WAM-only"] = 1;
	} else if (boost::algorithm::starts_with(cmd, "dont-compile ")) {
	    opt[cmd] = 1;
	} else {
	    std::cout << "Error. Unknown command/option: " << cmd << "\n";
	    assert("Unknown command/option" == "");
	}
    }
}

static bool match_strings(const std::string &actual,
			  const std::string &expect)
{
    // Compare at token level
    std::stringstream in_actual(actual);
    std::stringstream in_expect(expect);

    term_token_diff diff(in_actual, in_expect);
    if (!diff.check()) {
	std::cout << "Error. Difference spotted." << std::endl;
	return false;
    }
    return true;
}

static bool test_run_once(interpreter &interp,
			  size_t iteration,
			  const term &query,
			  std::vector<std::string> &expected)
{
    std::string actual;
    bool r = false;
    try {
	if (iteration == 0) {
	    r = interp.execute(query);
	} else {
	    r = interp.next();
	}
	if (!r) {
	    if (iteration == expected.size()) {
		return true;
	    }
	    if (iteration > 0) {
		actual = "end";
	    } else {
		actual = "fail";
	    }
	} else {
	    actual = interp.get_result(false);
	}
    } catch (interpreter_exception &ex) {
	actual = ex.what();
	r = false;
    }	
    std::cout << "Actual: " << actual << std::endl;
    std::cout << "Expect: " << expected[iteration] << std::endl;
    assert(match_strings(actual, expected[iteration]));
    return r;
}

static bool test_interpreter_file(const std::string &filepath)
{
    std::cout << std::endl;
    std::cout << std::string(60, '=') << std::endl;
    std::cout << "Process file: " << filepath << std::endl;
    std::cout << std::string(60, '=') << std::endl;

    std::vector<term_parser *> files;

    interpreter interp;
    const std::string dir = boost::filesystem::path(filepath).parent_path().string();
    
    interp.set_current_directory(dir);

    // interp.set_debug(true);

    std::ifstream *infile = new std::ifstream(filepath);
    term_tokenizer *tokenizer = new term_tokenizer(*infile);
    term_parser *parser = new term_parser(*tokenizer, interp.get_heap(), interp.get_ops());

    con_cell query_op("?-", 1);
    con_cell action_op(":-", 1);

    try {
	bool first_clause = true;
	bool new_block = false;
	bool cont = false;

	std::vector<con_cell> predicates;

	std::unordered_map<std::string, int> opt;

	do {

	while (!infile->eof()) {
	    term t = parser->parse();

	    std::string comments = parser->get_comments_string();

	    // interp.sync_with_heap();

	    process_meta(interp, comments, opt);

	    bool is_query = interp.is_functor(t, query_op);
	    bool is_action = interp.is_functor(t, action_op);

	    // Once parsing is done we'll copy over the var-name bindings
	    // so we can pretty print the variable names.
	    parser->for_each_var_name( [&](const term  &ref,
					  const std::string &name)
				      { interp.set_name(ref,name); } );
	    parser->clear_var_names();

	    if (!is_query && new_block) {
		std::cout << std::endl;
		new_block = false;
	    }

	    if (is_query || first_clause) {
		std::cout << std::string(67, '-') << std::endl;
	    }

	    std::cout << interp.to_string(t,
		is_query ? term_emitter::STYLE_TERM : term_emitter::STYLE_PROGRAM)
		      << "\n";

	    std::vector<std::string> expected;

	    if (is_query) {
		expected = parse_expected(comments);
		if (expected.size() == 0) {
		    std::cout << "Error. Found no expected results for this query. "
		              << "At line " << parser->get_comments()[0].pos().line()
			      << "\n";
		    assert(false);
		}
	    }

	    if (!is_query && !is_action) {
		auto p = interp.clause_predicate(t);
		if (std::find(predicates.begin(), predicates.end(), p)
		    == predicates.end()) {
		    predicates.push_back(p);
		}
  	        try {
		    interp.load_clause(t);
		} catch (syntax_exception &ex) {
  		    std::cout << "Syntax error: " << ex.what()
			      << ": "
			      << interp.to_string(ex.get_term())
			      << std::endl;
	  	    throw ex;
		}

		first_clause = false;
	    }

	    if (is_action) {
		// Check if this is a consult operation
		term a = interp.arg(t, 0);
		if (!interp.is_list(a)) {
		    std::cout << "Unrecognized action. Only [...] is supported (to include other files)" << std::endl;
		    continue;
		}
		for (auto fileatom : interp.iterate_over(a)) {
		    if (!interp.is_list(fileatom)) {
			con_cell f = interp.functor(fileatom);
			if (f.arity() != 0) {
			    std::cout << "Unrecognized file name: " << interp.to_string(fileatom) << std::endl;
			    continue;
			}
			files.push_back(parser);
			std::string incfile = interp.atom_name(f) + ".pl";
			infile = new std::ifstream(interp.get_full_path(incfile));
			if (!infile->good()) {
			    delete infile;
			    std::cout << "File not found: " << incfile << std::endl;
			    continue;
			}
			tokenizer = new term_tokenizer(*infile);
			parser = new term_parser(*tokenizer, interp.get_heap(), interp.get_ops());
			continue;
		    }
		}
	    }

	    if (is_query) {
		std::cout << std::string(67, '-') << std::endl;

		first_clause = true;
		new_block = true;

		size_t tr_mark = interp.trail_size();
		term query = interp.arg(t, 0);

		// Skip the ordinary run if we have the option WAM-only
		if (opt.count("WAM-only") == 0) {
		    // Run this query first without WAM
		    interp.set_wam_enabled(false);
		    for (size_t i = 0; i < expected.size(); i++) {
			test_run_once(interp, i, query, expected);
		    }
		    interp.unwind(tr_mark);
		    interp.reset_files();
		}

		// Then run this query again to check that the result is
		// the same, when we compile using WAM the recent predicates.
		interp.set_wam_enabled(true);

		// Compile recent predicates
		if (do_compile) {
		    std::unordered_set<std::string> dont_compile_set;
		    for (auto p : predicates) {
			auto p_name = interp.to_string(p) + "/"
			       + boost::lexical_cast<std::string>(p.arity());
			if (opt.count("dont-compile " + p_name) > 0) {
	    	            std::cout << "[DONT compile]: "<< p_name << "\n";
			} else {
	    	            std::cout << "[Compile]: "<< p_name << "\n";
			    interp.compile(p);
			}
	   	    }
		    if (interp.is_debug()) {
			interp.print_code(std::cout);
		    }
		}
		predicates.clear();

		interp.set_register_hb(interp.heap_size());
		for (size_t i = 0; i < expected.size(); i++) {
		    test_run_once(interp, i, query, expected);
		}
		interp.unwind(tr_mark);
		interp.reset_files();
		interp.set_register_hb(interp.heap_size());
	    }
	}

	infile->close();
	
	cont = false;

	if (!files.empty()) {
	    cont = true;
	    delete parser;
	    delete tokenizer;
	    delete infile;
	    parser = files.back();
	    files.pop_back();
	    tokenizer = &parser->tokenizer();
	    infile = static_cast<std::ifstream *>(&tokenizer->in());
	}

	} while (cont);

	return true;

    } catch (token_exception &ex) {
	infile->close();

	std::cout << "Parse error at line " << ex.pos().line() << " and column " << ex.pos().column() << ": " << ex.what() << "\n";
	return false;
    } catch (term_parse_error_exception &ex) {
	infile->close();

	auto tok = ex.token();
	std::cout << "Parse error at line " << tok.pos().line() << " and column " << tok.pos().column() << ": " << ex.what() << "\n";
	return false;
    }
}

static void test_interpreter_files(const char *filter = nullptr)
{
    header( "test_interpreter_files" );

    const std::string &home_dir = find_home_dir();
    std::string files_dir = home_dir + "/src/interp/test/pl_files";

    boost::filesystem::directory_iterator it_end;

    std::vector<boost::filesystem::path> files;

    for (boost::filesystem::directory_iterator it(files_dir); it != it_end; ++it) {
        auto filepath = it->path().string();

	if (filter != nullptr && filepath.find(filter) == std::string::npos) {
	    continue;
	}

	if (filter == nullptr &&
	    it->path().filename().string() == "ex_99_bigone.pl") {
	    continue; // Skip it for now...
	}

	if (boost::starts_with(it->path().filename().string(), "ex_") &&
	    boost::ends_with(filepath, ".pl")) {
	    files.push_back(filepath);
	}
    }

    std::sort(files.begin(), files.end());

    for (auto &filepath : files) {
	bool r = test_interpreter_file(filepath.string());
	assert(r);
    }
}

int main( int argc, char *argv[] )
{
    find_home_dir(argv[0]);

    if (argc == 2) {
	test_interpreter_files(argv[1]);
    } else {
	test_interpreter_files();
    }

    return 0;
}

