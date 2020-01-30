#include "wam_interpreter.hpp"
#include "wam_compiler.hpp"

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
    compiler_ = new wam_compiler(*this);
}

wam_interpreter::~wam_interpreter()
{
    delete compiler_;
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

void wam_interpreter::compile(const qname &qn)
{
    wam_interim_code instrs(*this);
    if (!compiler_->compile_predicate(qn, instrs)) {
        return;
    }
    size_t xn_size = compiler_->get_num_x_registers(instrs);
    size_t yn_size = compiler_->get_environment_size_of(instrs);    
    size_t first_offset = next_offset();
    load_code(instrs);

    // Update WAM code points if there was reallocation.
    if (reallocation_occurred()) {
	for (auto &entry : code_db_) {
	    auto &qn = entry.first;
	    auto &cp = entry.second;
	    if (cp.has_wam_code()) {
		cp.set_wam_code(resolve_wam_predicate(qn.first, qn.second));
	    }
	}
    }

    auto *next_instr = to_code(first_offset);
    set_wam_predicate(qn, next_instr, xn_size, yn_size);
    code_point cp(next_instr);
    set_code(qn, cp);
}

void wam_interpreter::compile(common::con_cell module, common::con_cell name)
{
    compile(std::make_pair(module, name));
}

void wam_interpreter::compile(common::con_cell name)
{
    compile(std::make_pair(current_module(), name));
}

void wam_interpreter::compile()
{
    for (auto &qn : get_predicates()) {
	if (is_updated_predicate(qn)) {
	    remove_compiled(qn);
	}
	if (!is_compiled(qn)) {
	    compile(qn);
	}
    }
    clear_updated_predicates();
}

void wam_interpreter::recompile()
{
    std::vector<qname> recompiled;
    for (auto &qn : get_updated_predicates()) {
        if (is_compiled(qn)) {
	    remove_compiled(qn);
	    compile(qn);
	    recompiled.push_back(qn);
	}
    }
    for (auto &qn : recompiled) {
        clear_updated_predicate(qn);
    }
}

void wam_interpreter::recompile_if_needed(const qname &qn)
{
    if (!is_compiled(qn)) {
        return;
    }
    if (is_updated_predicate(qn)) {
        remove_compiled(qn);
	compile(qn);
	clear_updated_predicate(qn);
    }
}
    
void wam_interpreter::bind_code_point(std::unordered_map<size_t, size_t> &label_map, code_point &cp)
{
    if (!cp.has_wam_code()) {
	auto term = cp.term_code();
	if (term.tag() == common::tag_t::INT) {
	    auto lbl_int = static_cast<const int_cell &>(term);
	    auto lbl = static_cast<size_t>(lbl_int.value());
	    if (label_map.count(lbl)) {
		size_t offset = label_map[lbl];
		auto *instr = reinterpret_cast<wam_instruction_code_point *>(to_code(offset));
		cp.set_wam_code(instr);
	    }
	} else if (term.tag() == common::tag_t::CON) {
	    auto lbl_con = static_cast<const con_cell &>(term);
	    if (auto instr = resolve_wam_predicate(current_module(), lbl_con)) {
		cp.set_wam_code(instr);
	    }
	} else if (term.tag() == common::tag_t::STR) {
	    static const con_cell functor_colon(":", 2);
	    if (functor(term) == functor_colon) {
		auto module = functor(arg(term, 0));
		auto f = functor(arg(term, 1));
		if (auto instr = resolve_wam_predicate(module, f)) {
		    cp.set_wam_code(instr);
		}
	    }
	}
    }
}

void wam_interpreter::load_code(wam_interim_code &instrs)
{
    // instrs.print(std::cout);

    std::unordered_map<size_t, size_t> label_map;
    size_t first_offset = next_offset();
    size_t offset = first_offset;
    // Collect labels
    for (auto *instr : instrs) {
	if (wam_compiler::is_label_instruction(instr)) {
	    auto *lbl_instr = static_cast<wam_interim_instruction<INTERIM_LABEL> *>(instr);
	    size_t lbl = static_cast<size_t>(lbl_instr->label().value());
	    label_map.insert(std::make_pair(lbl, offset));
	} else {
	    add(*instr);
	    size_t sz = instr->size();
	    offset += sz;
	}
    }

    // Update code points
    wam_instruction_base *instr = to_code(first_offset);
    for (size_t i = first_offset; i < offset;) {
	switch (instr->type()) {
	case TRY_ME_ELSE:
	case RETRY_ME_ELSE:
	case TRY:
	case RETRY:
	case TRUST:
	case CALL:
	case EXECUTE:
	case GOTO:
	    {
	    auto cp_instr = static_cast<wam_instruction_code_point *>(instr);
	    bind_code_point(label_map, cp_instr->cp());
	    break;
	    }
        case SWITCH_ON_TERM:
	    {
	    auto cp_instr = static_cast<wam_instruction<SWITCH_ON_TERM> *>(instr);
	    bind_code_point(label_map, cp_instr->pv());
	    bind_code_point(label_map, cp_instr->pc());
	    bind_code_point(label_map, cp_instr->pl());
	    bind_code_point(label_map, cp_instr->ps());
	    }
	    break;
        case SWITCH_ON_CONSTANT:
        case SWITCH_ON_STRUCTURE:
	    {
	    auto cp_instr = static_cast<wam_instruction_hash_map *>(instr);
	    for (auto &e : cp_instr->map()) {
		bind_code_point(label_map, e.second);
	    }
	    }
	    break;
	default:
	    break;
	}
	i += instr->size();
	instr = next_instruction(instr);
    }
}

}}
