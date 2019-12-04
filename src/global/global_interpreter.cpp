#include "global_interpreter.hpp"
#include "builtins.hpp"

using namespace prologcoin::common;

namespace prologcoin { namespace global {

global_interpreter::global_interpreter() {
    builtins::load(*this);
    setup_standard_lib();
}

bool global_interpreter::execute_goal(term t) {
    return execute(t);
}

bool global_interpreter::execute_goal(buffer_t &serialized)
{
    term_serializer ser(*this);
    try {
        term goal = ser.read(serialized);
	if (!execute(goal)) {
	    return false;
	}
	bool do_fail = false;
	if (naming_) {
	    std::vector<binding> memorize;
	    const std::vector<binding> &vars = const_cast<const global_interpreter &>(*this).query_vars();
	    for (auto &binding : vars) {
	        if (name_to_term_.count(binding.name())) {
		    auto current_term = name_to_term_[binding.name()];
		    if (!unify(current_term, binding.value())) {
		        do_fail = true;
			break;
		    }
		} else {
		    memorize.push_back(binding);
		}
	    }
	    for (auto &binding : memorize) {
	        name_to_term_[binding.name()] = binding.value();
	    }
	    if (do_fail) {
	        unwind_to_top_choice_point();
		return false;
	    }
	}
	
	serialized.clear();
	ser.write(serialized, goal);
	return true;
    } catch (serializer_exception &ex) {
        return false;
    } catch (interpreter_exception &ex) {
        return false;
    }
}

void global_interpreter::execute_cut() {
    interpreter_base::cut();
    interpreter_base::clear_trail();
}
    

}}
