#include "wam_interpreter.hpp"

namespace prologcoin { namespace interp {

std::unordered_map<wam_instruction_base::fn_type, wam_instruction_base::print_fn_type> wam_instruction_base::print_fns_;
std::unordered_map<wam_instruction_base::fn_type, wam_instruction_base::updater_fn_type> wam_instruction_base::updater_fns_;

void wam_code::print_code(std::ostream &out)
{
    for (size_t i = 0; i < instrs_size_;) {
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
    mode_ = READ;
    set_num_y_fn( &num_y );
    register_p_.reset();
    register_s_ = 0;
    memset(register_xn_, 0, sizeof(register_xn_));
}

wam_interpreter::~wam_interpreter()
{
    for (auto m : hash_maps_) {
	delete m;
    }
}

}}
