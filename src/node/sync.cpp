#include "self_node.hpp"
#include "local_interpreter.hpp"
#include "sync.hpp"
#include "session.hpp"
#include "self_node.hpp"

using namespace prologcoin::common;
using namespace prologcoin::interp;

namespace prologcoin { namespace node {

sync::sync(self_node *self)
    : self_(self),
      sync_file_("sync.pl"),
      session_(self->new_in_session(nullptr, true)),
      interp_(session_->interp()) {
    interp_.dont_load_startup_file();
    interp_.set_current_directory(self->data_directory()),
    interp_.set_debug_enabled();
    interp_.ensure_initialized();
    setup_sync_impl();
}

void sync::start()
{
    thread_ = boost::thread([&](){ run(); });
}

void sync::join()
{
    stop_ = true;
    thread_.join();
}

void sync::load()
{
    auto fullpath = interp_.get_full_path(sync_file_);
    if (!fullpath.empty() && boost::filesystem::exists(fullpath)) {
        std::ifstream ifs(fullpath);
	auto old_module = interp_.current_module();
	interp_.set_current_module(con_cell("sync",0));
	interp_.load_program(ifs);
	interp_.set_current_module(old_module);
    }
}

void sync::save()
{
    auto fullpath = interp_.get_full_path(sync_file_);
    // Check if wallet is non empty first
    auto &mod = interp_.get_module(con_cell("sync",0));
    if (mod.empty()) {
	// No need to save something empty
	return;
    }
    std::ofstream ofs(fullpath);
    interp_.save_program(con_cell("sync",0),ofs);
}

void sync::check_dirty()
{
    auto &meta = interp_.get_module_meta(con_cell("sync",0));
    if (meta.has_changed()) {
        save();
    }
}

void sync::update_sync_mode()
{
    auto &pred = interp_.get_predicate(con_cell("sync",0), con_cell("mode",1));
    if (pred.num_clauses() == 1) {
	for (auto &c : pred.get_clauses()) {
	    if (!c.is_erased()) {
		auto arg = interp_.clause_first_arg(c.clause());
		self_->set_sync_mode(interp_.to_string(arg));
		if (arg == con_cell("done",0)) {
		    self_->set_sync_complete(true);
		    stop();
		}
	    }
	}
    }
}

void sync::update_sync_meta_block()
{
    auto &pred = interp_.get_predicate(con_cell("sync",0), con_cell("low",1));
    if (pred.num_clauses() == 1) {
	for (auto &c : pred.get_clauses()) {
	    if (!c.is_erased()) {
		auto arg = interp_.clause_first_arg(c.clause());
		if (arg.tag() == tag_t::INT) {
		    auto v = reinterpret_cast<int_cell &>(arg).value();
		    if (v > 0) {
			self_->set_syncing_meta_block(static_cast<size_t>(v));
		    }
		}
	    }
	}
    }
}
	
void sync::run()
{
    auto mod = interp_.functor("sync_impl",0);
    interp_.set_current_module(mod);
    auto main_qr = interp_.new_term(con_cell(":",2),
				    {mod, con_cell("sync",0)});

    auto max = std::numeric_limits<uint64_t>::max();    
    while (!stop_) {
	interp_.session().set_available_funds( max );
	interp_.session().execute(main_qr);
	std::cout << interp_.get_text_out();
	check_dirty();

	// Check if sync is done
	update_sync_mode();
	update_sync_meta_block();
	if (!stop_) {
	    utime::sleep(utime::us(self_->get_fast_timer_interval_microseconds()));
	}
    }
}
	
void sync::setup_sync_impl()
{
    std::string template_source = R"PROG(    

sync :- 
    sync_setup,
    sync:mode(Mode), 
    sync_run(Mode), !.

sync_setup :-
    (\+ current_predicate(sync:mode/1) -> assert(sync:mode(meta)) ; true),
    (\+ current_predicate(sync:step/1) -> assert(sync:step(1000)) ; true),
    (\+ current_predicate(sync:'timeout'/1) -> assert(sync:'timeout'(100000)) ; true),
    (\+ current_predicate(sync:lookahead/1) -> assert(sync:lookahead(10)) ; true),
    (\+ current_predicate(sync:low/1) -> assert(sync:low(0)) ; true).

%

sync_run(meta) :-
    !,
    sync_run_roots,
    (sync_process_delayed_results ; true),
    (sync_meta_blocks(10) ; true),
    (current_predicate(sync:root/4) -> true ;
     retract(sync:mode(_)),
     assert(sync:mode(done)),
     write('Sync complete'), nl).

print_roots(Label) :-
    (current_predicate(sync:root/4) -> 
    findall(XX-YY-ZZ, sync:root(YY,XX,ZZ,_), LLL),
    write('----- roots '), write(Label), nl,
    write(LLL), nl,
    write('------------'), nl ; true).

sync_meta_blocks(0) :- !.
sync_meta_blocks(N) :-
    current_predicate(sync:root/4),
    (\+ current_predicate(tmp:run/1) -> assert(tmp:run(dummy)) ; true),
    % Process lowest height first
    (sync_meta_next(Height,Id,Span) ->
        assert(tmp:run(Id)),
        (ready(Connection) -> true ; retract(tmp:run(Id)), fail),
        sync:'timeout'(Timeout),
        write('Get '), write(Height), write(' '), write(Id), write(' '), write(Span), nl,
        (metas(Id, Span, Result) @= (Connection else
            (
%            write('failed connection '), write(Id), nl,
             retract(tmp:run(Id)),
             meta_fail_root(Height, Id)) timeout Timeout)),
        freeze(Result, (delay_put_metas1(Result) ->
                      sync_delay_result(Result,Height,Id,Span)
                    ; sync_add_result(Result,Height,Id,Span)
                    ; retract(tmp:run(Id)), meta_fail_root(Height, Id))),
        N1 is N - 1,
        !,
        sync_meta_blocks(N1) 
    ; true).

sync_meta_next(Height,Id,Span) :-
    findall(root(H,I,S,F), sync:root(H,I,S,F), L),
    sort(L, SortedL),
    member(root(Height,Id,Span,_), SortedL),
    \+ tmp:run(Id).

sync_delay_result(Result, Height, Id, Span) :-
    write('Delay '), write(Height), write(' '), write(Id), write(' '), write(Span), nl,
    assert(tmp:delay(Height, Id, Span, Result)).

sync_process_delayed_results :-
    current_predicate(tmp:delay/4),
    findall(delay(H,I,S,F), tmp:delay(H,I,S,F),L),
    sort(L, Sorted),
    [delay(Height,Id,Span,Result)|_] = Sorted,
    write('Delayed '), write(Height), write(' '), write(Id), write(' '), write(Span), nl,
    (delay_put_metas1(Result) -> true ; 
        retract(tmp:delay(Height,Id,Span,_)),
        sync_add_result(Result, Height, Id, Span)).

delay_put_metas1(Result) :-
%    (\+ current_predicate(tmp:mycnt/1) -> assert(tmp:mycnt(0)) ; true),
%    tmp:mycnt(Cnt), Cnt1 is Cnt + 1,
%    retract(tmp:mycnt(_)),
%    assert(tmp:mycnt(Cnt1)),
%    (0 is mod(Cnt1,5) -> true ; delay_put_metas(Result)).
    delay_put_metas(Result).

sync_add_result(Result, Height, Id, Span) :-
    write('Add '), write(Height), write(' '), write(Id), write(' '), write(Span), nl,
    put_metas(Result),
    (retract(sync:root(Height,Id,Span,_)) ; true),
    !,
    retract(tmp:run(Id)),
    last(Result, MetaRecord),
    meta_height(MetaRecord, LastH),
    meta_id(MetaRecord, LastId),
    % Need continuation sync?
    length(Result, Len),
    Remaining is max(Height+Span-1 - LastH, 2),
    (Len > 1, Remaining > 0
        -> (current_predicate(sync:root/4), sync:root(LastH,LastId,_,_) ->
             true ; assert(sync:root(LastH,LastId,Remaining,0)))
        ; true),
    % Add branches
    branches(Result, Branches),
    add_branches(Branches),
    % Update low
    (current_predicate(sync:root/4) ->
        findall(H, sync:root(H,_,_,_), Hs) ; Hs = []),
    sort(Hs, HsSorted),
    (current_predicate(sync:root/4) ->
        findall(HH-AA-BB, sync:root(HH,AA,BB,_), HHs) ; HHs = []),
    retract(sync:low(_)),
    ([HLow|_] = HsSorted ->
        assert(sync:low(HLow)) ;
        assert(sync:low(LastH))).
%    sync:low(LLL),
%    write('Updated low '), write(LLL), nl.

meta_fail_root(Height, Id) :-
    write('Fail '), write(Id), write(' '), write(Height), nl,
    sync:root(Height, Id, Span, FailCount),
    retract(sync:root(Height, Id, Span, FailCount)),
    FailCount1 is FailCount + 1,
    (FailCount1 < 5 -> assert(sync:root(Height, Id, Span, FailCount1)) ; true).
                    
add_branches([]).
add_branches([branch(Height,Id)|Branches]) :-
    (current_predicate(sync:root/4), sync:root(Height,Id,_,_) -> true
            ; sync:step(Span), assert(sync:root(Height,Id,Span,0))),
    add_branches(Branches).

meta_height(meta(PropList), Height) :- meta_height1(PropList, Height).
meta_height1([height(H)|_], H) :- !.
meta_height1([_|Xs], H) :- meta_height1(Xs, H).

meta_id(meta(PropList), Id) :- meta_id1(PropList, Id).
meta_id1([id(Id)|_], Id) :- !.
meta_id1([_|Xs], Id) :- meta_id1(Xs, Id).
     

%
% sync:root(Height, Id, Span, FailCount)
%

sync_run_roots :-
    (sync:low(0), \+ current_predicate(sync:root/4) 
         -> sync:step(Span), assert(sync:root(0, [], Span, 0)) ; true),
    (\+ current_predicate(tmp:rooting/1) -> assert(tmp:rooting(false)) ; true),
    (\+ current_predicate(tmp:last_rooting/1) -> assert(tmp:last_rooting(0)) ; true),
    sync_rooting_check.

sync_rooting_check :-
    (tmp:rooting(false),
     tmp:last_rooting(T),
     now(seconds, T0), T0 > T + 10 ->
        retract(tmp:last_rooting(_)),
        assert(tmp:last_rooting(T0)),
        sync:lookahead(LA),
        (current_predicate(sync:root/4) ->
            findall(H, sync:root(H,_,_,_), L) ; L = []),
        length(L,N),
        N < LA, sync_rooting ; true).

add_roots([], _, _).
add_roots([Id|Roots], Height, Step) :-
    (current_predicate(sync:root/4), sync:root(Height, Id, _, _) 
      ; assert(sync:root(Height, Id, Step, 0))),
    Height1 is Height + Step,
    add_roots(Roots, Height1, Step).

sync_rooting :-
    retract(tmp:rooting(_)),
    assert(tmp:rooting(true)),
    (sync_rooting1 ; retract(tmp:rooting(_)), assert(tmp:rooting(false))), !.

sync_rooting1 :-
    current_predicate(sync:root/4),
    findall(H, sync:root(H,_,_,_), L),
    max(L, MaxHeight),
    sync:root(MaxHeight, Id, _, _),
    !,
    sync:step(Step),
    sync:'timeout'(Timeout),
    sync:lookahead(N),
    ready(Connection),
    meta_roots(Id, Step, N, Roots) @= (Connection else
          (retract(tmp:rooting(_)),
          assert(tmp:rooting(false))) timeout Timeout),
    freeze(Roots, (add_roots(Roots, MaxHeight, Step),
                   retract(tmp:rooting(_)),
                   assert(tmp:rooting(false)))).

max([], 0).
max([X|Xs], M) :- max(Xs, M1), (X > M1 -> M = M1 ; M = X).

min([X|Xs], M) :- (Xs = [] -> M = X ; min(Xs, M1), (X < M1 -> M = M1 ; M = X)).

)PROG";

    con_cell old_module = interp_.current_module();
    interp_.set_current_module(interp_.functor("sync_impl",0));
    interp_.use_module(interp_.ME);
    self_->set_sync_complete(false);
    try {
	// First load init
	std::string init_str = self_->get_sync_init();
	if (!init_str.empty()) {
	    init_str += ".";
	    interp_.load_program(init_str);
        }

	// Load template code above
        interp_.load_program(template_source);
    } catch (interpreter_exception &ex) {
        std::cout << "Error while loading internal sync_impl source:" << std::endl;
        std::cout << ex.what() << std::endl;
    } catch (term_parse_exception &ex) {
        std::cout << "Error while loading internal sync_impl source:" << std::endl;      
        std::cout << term_parser::report_string(interp_, ex) << std::endl;
    } catch (token_exception &ex) {
        std::cout << "Error while loading internal sync_impl source:" << std::endl;      
        std::cout << term_parser::report_string(interp_, ex) << std::endl;
    }
    interp_.compile();

    auto sync_pl = interp_.get_full_path(sync_file_);
    if (boost::filesystem::exists(sync_pl)) {
	interp_.set_current_module(con_cell("sync",0));
	interp_.load_file(sync_file_);
    }

    interp_.set_current_module(old_module);
}

}}
