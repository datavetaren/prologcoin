#include "wam_interpreter.hpp"

namespace prologcoin { namespace interp {

std::unordered_map<wam_instruction_base::fn_type, wam_instruction_base::print_fn_type> wam_instruction_base::print_fns_;
std::unordered_map<wam_instruction_base::fn_type, wam_instruction_base::updater_fn_type> wam_instruction_base::updater_fns_;

size_t wam_code::add(const wam_instruction_base &i)
{
    size_t offset = next_offset();
    size_t sz = i.size();
    code_t *old_base = instrs_;
    code_t *data = ensure_fit(sz);
    code_t *new_base = instrs_;
    auto p = reinterpret_cast<wam_instruction_base *>(data);
    memcpy(p, &i, sz*sizeof(code_t));

    if (old_base != new_base) {
	p->update(old_base, new_base);
    }

    if (i.type() == EXECUTE || i.type() == CALL) {
	auto *cp_instr = reinterpret_cast<wam_instruction_code_point *>(p);
	auto module = cp_instr->cp().module();
	auto f = cp_instr->cp().name();
	calls_[std::make_pair(module,f)].push_back(offset);
    }
    
    return sz;
}

void wam_code::print_code(std::ostream &out)
{
    static const common::con_cell default_module("[]",0);
    for (size_t i = 0; i < instrs_size_;) {
	if (predicate_rev_map_.count(i)) {
	    auto name = predicate_rev_map_[i];
	    if (name.first == default_module) {
		out << interp_.to_string(name.second);
	    } else {
		out << interp_.to_string(name.first) << ":"
		    << interp_.to_string(name.second);
	    }
	    out << "/" << name.second.arity() << ": ";
	    if (predicate_map_.count(name)) {
	      auto meta_data = predicate_map_[name];
	      out << "(num_x=" << meta_data.num_x_registers << ")";
	    }
	    out << std::endl;
	}

	wam_instruction_base *instr
	  = reinterpret_cast<wam_instruction_base *>(&instrs_[i]);
	out << "[" << std::setw(5) << i << "]: ";
	instr->print(out, interp_);
	out << std::endl;
	
	i += instr->size();
    }
}

wam_interpreter::wam_interpreter() : wam_code(*this)
{
    fail_ = false;
    mode_ = READ;
    set_num_y_fn( &num_y );
    set_save_restore_state_fns( &save_state, &restore_state );
    register_s_ = 0;
    memset(register_xn_, 0, sizeof(register_xn_));
}

wam_interpreter::~wam_interpreter()
{
    for (auto m : hash_maps_) {
	delete m;
    }
}

bool wam_interpreter::cont_wam()
{
    fail_ = false;
    while (p().has_wam_code() && !is_top_fail()) {
	if (auto instr = p().wam_code()) {
	    if (is_debug()) {
		std::cout << "[WAM debug]: tr=" << trail_size() << " [" << std::setw(5)
			  << to_code_addr(instr) << "]: e=" << e0() << " ";
		instr->print(std::cout, *this);
		std::cout << "\n";
	    }
	    instr->invoke(*this);
	    cnt++;
	}
    }
    if (is_debug()) {
	if (fail_) {
	    std::cout << "[WAM debug]: fail\n";
	} else {
	    std::cout << "[WAM debug]: exit\n";
	}
    }
    return !fail_;
}

}}
