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

void wam_code::remove_compiled(const qname &qn) 
{
    auto it = predicate_map_.find(qn);
    if (it != predicate_map_.end()) {
	auto meta_data = it->second;
	predicate_map_.erase(qn);
	predicate_rev_map_.erase(meta_data.code_offset);
	
	auto &offsets = calls_[qn];
	for (auto offset : offsets) {
	    auto *cp_instr = reinterpret_cast<wam_instruction_code_point *>(to_code(offset));
	    cp_instr->cp().set_wam_code(nullptr);
	    cp_instr->cp().set_qn(qn);
	}
    }
}

void wam_code::update(code_t *old_base, code_t *new_base)
{
    wam_instruction_base *instr = reinterpret_cast<wam_instruction_base *>(new_base);
    for (size_t i = 0; i < instrs_size_;) {
	instr->update(old_base, new_base);
	instr = interp_.next_instruction(instr);
	i = static_cast<size_t>(reinterpret_cast<code_t *>(instr) - new_base);
    }

    for (auto &p : interp_.code_db())
    {
	code_point &code_p = p.second;
	update_ptr(code_p, old_base, new_base);
    }
}

void wam_code::print_code(std::ostream &out)
{
    print_code(out, 0, instrs_size_);
}
	
void wam_code::print_code(std::ostream &out, size_t from, size_t to)
{
    static const common::con_cell default_module("[]",0);
    for (size_t i = from; i < to;) {
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

wam_interpreter::wam_interpreter(const std::string &name) : interpreter_base(name), wam_code(*this), auto_wam_(false), compiler_(nullptr)
{
    total_reset();
}

wam_interpreter::~wam_interpreter()
{
    delete compiler_;
    for (auto m : hash_maps_) {
	delete m;
    }
}

void wam_interpreter::update(code_t *old_base, code_t *new_base)
{
    wam_code::update(old_base, new_base);

    struct visit : public stack_frame_visitor {
	visit(wam_interpreter &interp, code_t *old_base0, code_t *new_base0) :
	    interp_(interp), old_base_(old_base0), new_base_(new_base0) { }
	
	virtual void visit_naive_environment(environment_naive_t *e) override {
	    visit_env(e);
	}
	virtual void visit_wam_environment(environment_t *e) override {
	    visit_env(e);
	}
	virtual void visit_frozen_environment(environment_frozen_t *e) override {
	    visit_env(e);
	    update(e->p);
	}
	virtual void visit_choice_point(choice_point_t *cp) override {
	    update(cp->cp);
	    update(cp->bp);
	}
	virtual void visit_meta_context(meta_context *m) override {
	    update(m->old_p);
	    update(m->old_cp);	    
	}
	inline void visit_env(environment_base_t *e) {
	    update(e->cp);
	}

	inline void update(code_point &cp) {
	    update_ptr(cp, old_base_, new_base_);
	}

	wam_interpreter &interp_;
	code_t *old_base_;
	code_t *new_base_;	
    };

    visit v(*this, old_base, new_base);
    foreach_stack_frame(v);

    update_ptr(p(), old_base, new_base);
    update_ptr(cp(), old_base, new_base);
    update_ptr(tmp_, old_base, new_base);
}
	
std::string wam_interpreter::to_string(const code_point &cp) const
{
    using namespace common;
	
    if (cp.has_wam_code()) {
	size_t offset = to_code_addr(cp.wam_code());
	std::string str = "[" + boost::lexical_cast<std::string>(offset) + "]";
	auto &t = cp.term_code();
	if (t.tag() == tag_t::CON || t.tag() == tag_t::STR) {
	    str += " (" + to_string(t);
	    size_t arity = 0;
	    if (t.tag() == tag_t::CON) {
		arity = reinterpret_cast<const con_cell &>(t).arity();
	    } else {
		assert(t.tag() == tag_t::STR);
		auto f = functor(t);
		if (f == COLON) {
		    term rh = arg(t, 1);
		    if (rh.tag() == tag_t::CON) {
			arity = reinterpret_cast<const con_cell &>(rh).arity();
		    }
		}
	    }
	    str += "/" + boost::lexical_cast<std::string>(arity) + ")";
	} else if (t.tag() == tag_t::INT) {
	    // Ignore integers. It can be used for certain WAM instructions
	    // to indicate label entry points.
	} else {
	    str += " ??? tag=" + boost::lexical_cast<std::string>(t.tag());
	    str += " (" + to_string(t) + ")";
	}
	return str;
    } else {
	return interpreter_base::to_string_cp(cp);
    }
}

void wam_interpreter::total_reset()
{
    interpreter_base::total_reset();
    wam_code::total_reset();

    fail_ = false;
    mode_ = READ;
    set_num_y_fn( &num_y );
    set_save_restore_state_fns( &save_state, &restore_state );
    register_s_ = 0;
    memset(register_xn_, 0, sizeof(register_xn_));
    if (compiler_) delete compiler_;
    for (auto *m : hash_maps_) delete m;
    hash_maps_.clear();
    compiler_ = new wam_compiler(*this);
}

bool wam_interpreter::cont_wam()
{
    fail_ = false;
    while (p().has_wam_code() && !is_top_fail()) {
	if (auto instr = p().wam_code()) {
	    if (is_debug()) {
		std::stringstream ss;
		ss << "[WAM debug]: tr=" << trail_size() << " [" << std::setw(5)
		   << instr << " " << to_code_addr(instr) << "]: e=" << e0() << " ";
		instr->print(ss, *this);
		std::cout << ss.str() << "\n";
	    }
	    instr->invoke(*this);
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

bool wam_interpreter::compile(const qname &qn)
{
    size_t heap_sz = heap_size();

    wam_interim_code instrs(*this);
    compiler_->clear();
    if (!compiler_->compile_predicate(qn, instrs)) {
	trim_heap_safe(heap_sz);
	return false;
    }
    size_t xn_size = compiler_->get_num_x_registers(instrs);
    size_t yn_size = compiler_->get_environment_size_of(instrs);    
    size_t first_offset = next_offset();
    
    load_code(instrs);

    auto *next_instr = to_code(first_offset);
    auto *end_instr = to_code(next_offset());
    set_wam_predicate(qn, next_instr, end_instr, xn_size, yn_size);
    code_point cp(next_instr);
    set_code(qn, cp);

    trim_heap_safe(heap_sz);

    get_predicate(qn).set_was_compiled(true);

    return true;
}

bool wam_interpreter::compile(common::con_cell module, common::con_cell name)
{
    return compile(std::make_pair(module, name));
}

bool wam_interpreter::compile(common::con_cell name)
{
    return compile(std::make_pair(current_module(), name));
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
        if (was_compiled(qn)) {
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
    if (!was_compiled(qn)) {
        return;
    }
    if (is_updated_predicate(qn)) {
        remove_compiled(qn);
	compile(qn);
	clear_updated_predicate(qn);
    }
}

void wam_interpreter::auto_compile(const qname &qn)
{
    auto &pred = get_predicate(qn);
    bool failed = false;
    if (pred.ok_to_compile()) {
	auto num_clauses = pred.num_clauses();
	if (num_clauses > 0 && num_clauses <= 10) {
	    if (!compile(qn)) {
		failed = true;
	    }
	} else {
	    failed = true;
	}
    }
    if (failed) {
	pred.set_ok_to_compile(false);
	remove_compiled(qn);
	clear_updated_predicate(qn);
    }
}

void wam_interpreter::print_code(std::ostream &out)
{
    wam_code::print_code(out);
}

void wam_interpreter::print_code(std::ostream &out, size_t from, size_t to)
{
    wam_code::print_code(out, from, to);
}

void wam_interpreter::print_code(std::ostream &out, const qname &qn)
{
    auto &meta_data = get_wam_predicate_meta_data(qn);
    print_code(out,meta_data.code_offset,meta_data.end_offset);
}
    
void wam_interpreter::bind_code_point(std::unordered_map<size_t, size_t> &label_map, code_point &cp)
{
    if (!cp.has_wam_code()) {
	auto term = cp.term_code();
	if (term.tag() == common::tag_t::INT) {
	    auto lbl_int = reinterpret_cast<const int_cell &>(term);
	    auto lbl = static_cast<size_t>(lbl_int.value());
	    if (label_map.count(lbl)) {
		size_t offset = label_map[lbl];
		auto *instr = reinterpret_cast<wam_instruction_code_point *>(to_code(offset));
		cp.set_wam_code(instr);
	    }
	} else if (term.tag() == common::tag_t::CON) {
	    auto lbl_con = reinterpret_cast<const con_cell &>(term);

	    qname qn{current_module(), lbl_con};
	    auto &code = get_code(qn);
	    if (code.has_wam_code()) {
		cp.set_wam_code(code.wam_code()); // Could be imported
	    } else if (auto instr = resolve_wam_predicate(current_module(), lbl_con)) {
		cp.set_wam_code(instr);
	    }
	} else if (term.tag() == common::tag_t::STR) {
	    static const con_cell functor_colon(":", 2);
	    if (functor(term) == functor_colon) {
		auto module = functor(arg(term, 0));
		auto f = functor(arg(term, 1));
		qname qn{module, f};
		auto &code = get_code(qn);
		if (code.has_wam_code()) {
		    cp.set_wam_code(code.wam_code()); // Could be imported
		} else if (auto instr = resolve_wam_predicate(module, f)) {
		    cp.set_wam_code(instr);
		}
	    }
	}
    }
}

void wam_interpreter::load_code(wam_interim_code &instrs)
{
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
