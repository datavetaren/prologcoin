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
    if (!started_) {
	started_ = true;
	thread_ = boost::thread([&](){ run(); });
    }
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
		self_->syncer().set_mode(interp_.to_string(arg));
		if (arg == con_cell("done",0)) {
		    set_complete(true);
		    stop();
		}
		if (arg == con_cell("wait",0) && !sync_root_.is_zero()) {
		    auto id_term = sync_root_.to_term(interp_);
		    auto rootid_fact = interp_.new_term( con_cell(":",2),
		        { con_cell("sync",0),
		          interp_.new_term( con_cell("rootid", 1),
			                    { id_term } ) });
		    interp_.remove_clauses(qname(con_cell("sync",0),
						 con_cell("rootid",1)));
		    interp_.load_clause(rootid_fact, interp::LAST_CLAUSE);
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
			set_at_meta_block(static_cast<size_t>(v));
		    }
		}
	    }
	}
    }
}

void sync::update_sync_root_id()
{
    auto &pred = interp_.get_predicate(con_cell("sync",0), con_cell("rootid",1));
    if (pred.num_clauses() == 1) {
	for (auto &c : pred.get_clauses()) {
	    if (!c.is_erased()) {
		auto arg = interp_.clause_first_arg(c.clause());
		auto id = me_builtins::get_meta_id(interp_, "sync::update_sync_root_id", arg);
		set_root(id);
	    }
	}
    }
}

void sync::update_sync_progress()
{
    auto &pred = interp_.get_predicate(con_cell("sync",0), interp_.functor("progress",1));
    if (pred.num_clauses() == 1) {
	for (auto &c : pred.get_clauses()) {
	    if (!c.is_erased()) {
		auto arg = interp_.clause_first_arg(c.clause());
		if (arg.tag() == tag_t::INT) {
		    auto v = reinterpret_cast<int_cell &>(arg).value();
		    if (v > 0) {
			set_progress(static_cast<size_t>(v));
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

    auto setup_qr = interp_.new_term(con_cell(":",2),
				     {mod, interp_.functor("sync_setup",0)});
    
    auto run_qr = interp_.new_term(con_cell(":",2),
				    {mod, con_cell("sync",0)});

    auto max = std::numeric_limits<uint64_t>::max();

    interp_.session().set_available_funds( max );

    try {
	interp_.session().execute(setup_qr);	
    } catch (std::runtime_error &ex) {
	std::cout << "sync::run(): ERROR: " << ex.what() << std::endl;
	stop_ = true;
    }

    update_sync_root_id();
    
    while (!stop_) {
	interp_.session().set_available_funds( max );
	try {
	    interp_.session().execute(run_qr);
	} catch (std::runtime_error &ex) {
	    std::cout << "sync::run(): ERROR: " << ex.what() << std::endl;
	    stop_ = true;
	}
	if (!stop_) {
	    std::cout << interp_.get_text_out();
	    check_dirty();

	    // Check if sync is done
	    update_sync_mode();
	    update_sync_meta_block();
	    update_sync_progress();
	    if (!stop_) {
		utime::sleep(utime::us(self_->get_fast_timer_interval_microseconds()));
	    }
	}
    }
    std::cout << "syncing stopped!" << std::endl;
}
	
void sync::setup_sync_impl()
{
    std::string template_source_1 = R"PROG(

sync :- 
    critical_section((sync:mode(Mode), sync_run(Mode))).

sync_setup :-
    (\+ current_predicate(sync:mode/1) -> assert(sync:mode(meta)) ; true),
    (\+ current_predicate(sync:step/1) -> assert(sync:step(1000)) ; true),
    (\+ current_predicate(sync:'timeout'/1) -> assert(sync:'timeout'(100000)) ; true),
    (\+ current_predicate(sync:lookahead/1) -> assert(sync:lookahead(10)) ; true),
    (\+ current_predicate(sync:low/1) -> assert(sync:low(0)) ; true),
    (\+ current_predicate(tmp:run/1) -> assert(tmp:run(dummy)) ; true),
    (current_predicate(sync:low_db/2), sync:low_db(symbols,_) -> true ; assert(sync:low_db(symbols,0))),
    (current_predicate(sync:low_db/2), sync:low_db(program,_) -> true ; assert(sync:low_db(program,0))),
    (current_predicate(sync:low_db/2), sync:low_db(closure,_) -> true ; assert(sync:low_db(closure,0))),
    (current_predicate(sync:low_db/2), sync:low_db(heap,_) -> true ; assert(sync:low_db(heap,0))).

%
% ------
%

% debug_on.

debug(X) :- (current_predicate(sync_impl:debug_on/0) -> call(X) ; true).

sync_run(meta) :-
    !,
    sync_run_roots,
    (sync_process_delayed_results ; true),
    (sync_meta_blocks(10,[]) ; true),
    (current_predicate(sync:root/4) -> true ;
     retract(sync:mode(_)),
     assert(sync:mode(wait))).

sync_run(wait) :-
    !,
    current_predicate(sync:rootid/1),
    (current_predicate(tmp:runmore/0) -> true
    ; assert(tmp:runmore),
      sync:rootid(Root),
      sync:'timeout'(Timeout),
      ready(Connection),
      (meta_more(Root, Result) @= (Connection else (Result = fail) timeout Timeout)),
      freeze(Result, critical_section((
         retract(tmp:runmore),
         validate_meta(Result),
         (current_predicate(sync:db/3) -> retractall(db(_,_,_)) ; true),
         Result = meta(Info),
         extract_info(Info),
         retract(sync:mode(wait)),
         retract(sync:step(_)),
         sync_db_default_step(symbols, Step),
         assert(sync:step(Step)),
         assert(sync:mode(symbols)))))).

extract_info([]).
extract_info([db(DB, N, Root) | Rest]) :-
     !,
     assert(sync:db(DB, N, Root)),
     extract_info(Rest).
extract_info([_ | Rest]) :-
     extract_info(Rest).

sync_run(done).

sync_run(DB) :-
    current_predicate(sync:rootid/1),
    update_progress,
    sync:rootid(Root),
    (current_predicate(tmp:end/2), tmp:end(Root,DB) ->
      retract(tmp:end(Root,DB)),
      sync_db_done(DB, NextDB),
      retract(sync:mode(DB)),
      assert(sync:mode(NextDB)),
      retract(sync:step(_)),
      sync_db_default_step(NextDB, Step),
      assert(sync:step(Step))
    ; (sync_db_scan, ! ; true),
      % Do not attempt to schedule new downloads until scan
      % is complete
      (current_predicate(tmp:runscan/0) -> true ;
          (sync_db_schedule_gets(DB,Root), ! ; true),
          (sync_db_process_gets(10,[],DB,Root), ! ; true))).

update_progress :-
    sync:rootid(Root),
    progress([symbols, program, closure, heap], Root, 0, 0, Acc, Tot),
    Progress is 100*Acc // Tot,
    (current_predicate(sync:progress/1) -> retract(sync:progress(_)) ; true),
    assert(sync:progress(Progress)).

progress([], _, Acc, Tot, Acc, Tot).
progress([DB|Rest], Root, Acc0, Tot0, Acc, Tot) :-
    sync:db(DB, Num, _),
    db_num(Root, DB, Current),
    Acc1 is Acc0 + Current,
    Tot1 is Tot0 + Num,
    progress(Rest, Root, Acc1, Tot1, Acc, Tot).

sync_db_done(symbols, program).
sync_db_done(program, closure).
sync_db_done(closure, heap).
sync_db_done(heap, done).

sync_db_default_step(symbols, 10000).
sync_db_default_step(closure, 10000).
sync_db_default_step(program, 10000).
sync_db_default_step(heap, 16).
sync_db_default_step(done, 10000).

%
%
%

print_roots(Label) :-
    (current_predicate(sync:root/4) -> 
    findall(XX-YY-ZZ, sync:root(YY,XX,ZZ,_), LLL),
    write('----- roots '), write(Label), nl,
    write(LLL), nl,
    write('------------'), nl ; true).
)PROG";
    
    std::string template_source_2 = R"PROG(
%------------------------------
% Meta chain syncing
%------------------------------

write_short_id(Id) :-
   (Id = [] -> write(' Id=[]') ;
      sformat([A,B,C,D|_], "~16r", [Id]), write(' Id='), H = [A,B,C,D], format("~s", [H])).

sync_meta_blocks(0,_) :- !.
sync_meta_blocks(N,UsedConn) :-
    current_predicate(sync:root/4),
    % Process lowest height first
    (sync_meta_next(Height,Id,Span) ->
        assert(tmp:run(Id)),
        (ready(Connection,UsedConn) -> true ; retract(tmp:run(Id)), fail),
        NewUsedConn = [Connection|UsedConn],
        sync:'timeout'(Timeout),
        HeightEnd is Height + Span,
        % -- Debug>>> (scheduled get operation)
        debug((write('Get '), write(Height), write('..'), write(HeightEnd), write_short_id(Id), nl)),
        % -- Debug<<<
        (metas(Id, Span, Result) @= (Connection else
                                         (Result = fail) timeout Timeout)),
        freeze(Result, critical_section(
            (delay_put_metas1(Result) ->
                    sync_delay_result(Result,Height,Id,Span)
                    ; sync_add_result(Result,Height,Id,Span)
                    ; retract(tmp:run(Id)), meta_fail_root(Height,Span,Id)))),
        N1 is N - 1,
        !,
        (sync_meta_blocks(N1,NewUsedConn) -> true ; true)
    ; true).

sync_meta_next(Height,Id,Span) :-
    findall(root(H,I,S,F), sync:root(H,I,S,F), L),
    sort(L, SortedL),
    member(root(Height,Id,Span,_), SortedL),
    \+ tmp:run(Id).

sync_delay_result(Result, Height, Id, Span) :-
    HeightEnd is Height + Span,
    % --Debug>>> (Response out of order, put it on delay)
    debug((write('Delay '), write(Height), write('..'), write(HeightEnd), write_short_id(Id), nl)),
    % --Debug<<<
    assert(tmp:delay(Height, Id, Span, Result)).

sync_process_delayed_results :-
    current_predicate(tmp:delay/4),
    findall(delay(H,I,S,F), tmp:delay(H,I,S,F),L),
    sort(L, Sorted),
    [delay(Height,Id,Span,Result)|_] = Sorted,
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
    HeightEnd is Height + Span,
    % --Debug>>> (Adding response)
    debug((write('Add '), write(Height), write('..'), write(HeightEnd), write_short_id(Id), nl)),
    % --Debug<<<
    put_metas(Result), % Thus could fail if peer has lied about result
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

meta_fail_root(Height, Span, Id) :-
    HeightEnd is Height + Span,
    sync:root(Height, Id, Span, FailCount),
    FailCount1 is FailCount + 1,
    % --Debug>>> (We failed to add it, so reschedule and increment failcount)
    debug((write('Fail '), write(Height), write('..'), write(HeightEnd), write(' '), write('FailCount='), write(FailCount1), write_short_id(Id), nl)),
    % --Debug<<<
    retract(sync:root(Height, Id, Span, FailCount)),
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
    freeze(Roots, critical_section((add_roots(Roots, MaxHeight, Step),
                   retract(tmp:rooting(_)),
                   assert(tmp:rooting(false))))).

max([], 0).
max([X|Xs], M) :- max(Xs, M1), (X > M1 -> M = M1 ; M = X).

min([X|Xs], M) :- (Xs = [] -> M = X ; min(Xs, M1), (X < M1 -> M = M1 ; M = X)).
   )PROG";

    std::string template_source_3 = R"PROG(
%------------------------------
% Database/state syncing
%------------------------------

write_short_root(Root) :-
   sformat([A,B,C,D|_], "~16r", [Root]), write(' Root='), H = [A,B,C,D], format("~s", [H]).

sync_db_schedule_gets(DB,Root) :-
   sync:lookahead(LA),
   sync:low_db(DB,Low),
   sync:step(Step),
   (current_predicate(sync:get/4) ->
       findall(K, sync:get(K,Root,DB,_), L),
       length(L, N),
       (N < LA ->
           R is LA - N,
           add_gets(R, Low, Root, DB, Step)
       ; true)
    ; add_gets(LA, Low, Root, DB, Step)).

add_gets(0, _, _, _, _) :- !.
add_gets(N, Key, Root, DB, Step) :-
   Key1 is Key + Step,
   (\+ tmp:run(get(Key,Root,DB)), \+ (current_predicate(sync:get/4), sync:get(Key,Root,DB,_)) -> 
       assert(sync:get(Key,Root,DB,Step)) ; true),
   N1 is N - 1,
   add_gets(N1, Key1, Root, DB, Step).

sync_db_process_gets(0,_,_,_) :- !.
sync_db_process_gets(N,UsedConn,DB,Root) :-
   (sync_db_process_gets_one(DB,Root,UsedConn,Connection) -> NewUsedConn = [Connection|UsedConn], N1 is N - 1, sync_db_process_gets(N1,NewUsedConn,DB,Root) ; true).

sync_db_process_gets_one(DB,Root,UsedConn,Connection) :-
   ready(Connection,UsedConn),
   current_predicate(sync:get/4),
   sync_db_next(DB,Root,Key,Span),
   KeyEnd is Key + Span,
   % --Debug>>> (Schedule get operation)
   debug((write('Get '), write(Key), write('..'), write(KeyEnd), write(' DB='), write(DB), write_short_root(Root), nl)),
   % --Debug<<<
   assert(tmp:run(get(Key,Root,DB))),
   sync:'timeout'(Timeout),
   (db_get(Root,DB,Key,KeyEnd,Result) @= (Connection else
      (Result = fail) timeout Timeout)),
   %
   % Note that we can process out of order responses - doesn't affect
   % end result.
   %
   freeze(Result, critical_section((retract(tmp:run(get(Key,Root,DB))),
                   Result \= fail,
                   sync:db(DB, _, ExpectedDBRoot),
                   Result = branch(ActualDBRoot, _, _, _),
                   ActualDBRoot == ExpectedDBRoot,
                   (db_put(Root, DB, Key, KeyEnd, Result) ->
                      db_num(Result, Key, KeyEnd, NumKeys),
                      write('Add '), write(Key), write('..'), write(KeyEnd), write(' N='), write(NumKeys), write(' DB='), write(DB), write_short_root(Root), nl,
                      % --Debug>>> (Add response to database)
                      debug((write('Add '), write(Key), write('..'), write(KeyEnd), write(' N='), write(NumKeys), write(' DB='), write(DB), write_short_root(Root), nl)),
                      % --Debug<<<
                      (NumKeys = 0 ->
                           % If requested get was empty, we issue a scan
                           % operation to find the next viable key.
                           sync_db_push_scan(Root, DB, Key, KeyEnd) 
                         ; (current_predicate(tmp:numkeys/1) ->
                                retract(tmp:numkeys(_)) 
                              ; true),
                            assert(tmp:numkeys(NumKeys))),
                      retract(sync:get(Key,Root,DB,Span)),
                      sync_db_update_low(Root, DB, Key, KeyEnd)
                    ; sync_db_process_get_fail(DB,Root,Key,Span))))).

sync_db_update_low(Root, DB, Key, Last) :-
    sync:low_db(DB, Key1),
    (Key1 >= Key, Key1 < Last ->
        % --Debug>>>
        debug((write('Low '), write(Last), write(' '), write('DB='), write(DB), write_short_root(Root), nl)),
        % --Debug<<<
        retract(sync:low_db(DB, _)),
        assert(sync:low_db(DB, Last))
     ; true).

sync_db_process_get_fail(DB,Root,Key,Span) :-
    KeyEnd is Key + Span,
    (current_predicate(tmp:fail/5) ->
        tmp:fail(Key,DB,Root,Span,FailCount),
        retract(tmp:fail(Key,DB,Root,Span,_)),
        FailCount1 is FailCount + 1
      ; FailCount1 = 1
    ),
    % --Debug>>>
    debug((write('Fail '), write(Key), write('..'), write(KeyEnd), write(' FailCount='), write(FailCount1), write(' DB='), write(DB), write_short_root(Root), nl)),
    % --Debug<<<
    (FailCount1 > 5 ->
        (retract(sync:get(Key,Root,DB,Span)) ; true)
        ; assert(tmp:fail(Key,DB,Root,Span,FailCount1))).
    
sync_db_next(DB,Root,Key,Span) :-
    findall(get(K,R,D,S), sync:get(K,R,D,S), L),
    sort(L, SortedL),
    member(get(Key,Root,DB,Span), SortedL),
    \+ tmp:run(get(Key,Root,DB)), !.

sync_db_push_scan(Root,DB,LastKey,FromKey) :-
    (current_predicate(tmp:scan/4) -> true ; 
     (sync:low_db(DB,Low), Low < FromKey ->
         % --Debug>>>
         debug((write('Scan From='), write(FromKey), write(' DB='), write(DB), write_short_root(Root), nl)),
         % --Debug<<<
         assert(tmp:scan(Root,DB,LastKey,FromKey)) 
    ; true)).

sync_db_scan :-
    \+ current_predicate(tmp:runscan/0),
    current_predicate(tmp:scan/4),
    tmp:scan(Root,DB,LastKey,FromKey),
    ready(Connection),
    sync:'timeout'(Timeout),
    assert(tmp:runscan),
    (db_keys(Root,DB,FromKey,10,Result) @= (Connection else
           (Result = fail)) timeout Timeout),
    freeze(Result, critical_section(sync_db_process_scan(Root,DB,LastKey,FromKey,Result))).

sync_db_process_scan(Root,DB,LastKey,FromKey,Result) :-
    retract(tmp:runscan),
    db_keys(Result,FromKey,10,Keys),
    length(Keys, NumKeys),
    retract(tmp:scan(Root,DB,LastKey,FromKey)),
    (NumKeys = 0 ->
      % --Debug>>>
      debug((write('Done DB='), write(DB), write_short_root(Root), nl)),
      % --Debug<<<
      assert(tmp:end(Root,DB)),
      % And remove any pending jobs
      (current_predicate(sync:get/4) -> retractall(sync:get(_, Root, DB, _)) ; true)
      ; Keys = [FirstKey|_],
%        write('Keys '), write(Keys), nl,
        diff_list(Keys, Diffs),
%        write('Diffs '), write(Diffs), nl,
        average_list(Diffs, AvgDist),
%        write('Avg '), write(AvgDist), nl,
        sync:step(OldStep),
        retract(sync:step(OldStep)),
        tmp:numkeys(LastNumKeys),
        NewStep is LastNumKeys * AvgDist,
        % --Debug>>>
        debug((write('Step '), write(NewStep), write(' was '), write(OldStep), nl)),
        % --Debug<<<
        assert(sync:step(NewStep)),
        sync_db_update_low(Root,DB,LastKey,FirstKey),
        add_gets(1, FirstKey, Root, DB, 0)).

diff_list([_], []).
diff_list([X1,X2|Rest], [D|Diffs]) :-
    D is X2 - X1,
    diff_list([X2|Rest], Diffs).

average_list(List, Avg) :-
    List = [X|_], average_list(List, X, Avg).

average_list([], Acc, Acc).
average_list([X|Xs], Acc, Avg) :-
    Acc1 is (X + Acc) // 2,
    average_list(Xs, Acc1, Avg).
   
%    db_keys(Root,DB,FromKey,10,Result) @
%    dn_keys(Result, 


)PROG";

    con_cell old_module = interp_.current_module();
    interp_.set_current_module(interp_.functor("sync_impl",0));
    interp_.use_module(interp_.ME);
    set_complete(false);
    try {
	// First load init
	std::string init_str = get_init();
	if (!init_str.empty()) {
	    init_str += ".";
	    interp_.load_program(init_str);
        }

        std::string template_source;
        template_source += template_source_1;
        template_source += template_source_2;
        template_source += template_source_3;

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
