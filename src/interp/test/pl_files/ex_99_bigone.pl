% Meta: fileio on
% Meta: debug off
% Meta: WAM-only

% Read in standard library
:- [std].

%
% Construct state machine for LR parser
%
% What the h*** are you doing? This is insane! This is a tailored
% YACC/Bison written in Prolog. Why?
% 
% Well, three reasons:
%
% 1) Prolog's grammar is a little bit complicated as new operators can
%    be defined on the fly. It's difficult to map this to YACC/Bison
%    as the lookahead set new includes "non tokens". This could be
%    "compensated" by the tokenizer. But... meh. Ugly.
%
% 2) I'd like the system to rely on as few external tools as possible.
%    But Prolog is an external tool?! Yes, but once I have written my
%    own interpreter here it should be possible to run this. That by
%    itself is a great way of testing the interpreter.
%
% 3) The output of YACC/Bison is ugly. This tailored thing will produce
%    code that better fits with the rest of the code.
%

main :-
%    (retract(log_cnt(_)) ; true),
%    assert(log_cnt(0)),
    !,
    read_grammar_and_properties('term_grammar.pl', G),
    write('------------------------------------------------'),
    write(G), nl,
    write('------------------------------------------------'),
    extract_properties(G, Properties, Grammar),
    StartItem = [(start :- 'DOT', subterm_1200, full_stop)],
    Start = state(0, StartItem, []),
    states(Grammar, Start, States),
    % resolve_conflicts(States, Properties, States1),
    sort_actions(States, NewStates),
    emit(Grammar, Properties, NewStates),
    tell('states.txt'),
    print_states(NewStates),
    told.

%
% read_grammar(-G)
%
% Read grammar from prolog_grammar.pl. Return list of clauses
% (represented as terms) in G.
%
read_grammar_and_properties(FileName, G) :-
    open(FileName, read, F),
    read_clauses(F,G),
    close(F),
    !.

read_clauses(F, []) :- at_end_of_stream(F), !.
read_clauses(F, Cs) :-
        read(F, C),
	(C = end_of_file -> Cs = []
       ; Cs = [C|Cs1], read_clauses(F, Cs1)
	).

extract_properties([], [], []).
extract_properties([(Head :- Body)|Gs], Properties, [(Head :- Body)|Grammar]):-
    extract_properties(Gs, Properties, Grammar).
extract_properties([property(A,B)|Gs], [property(A,B)|Properties], Grammar) :-
    extract_properties(Gs, Properties, Grammar).

get_property(Properties, Name, Value) :-
    member(property(Name, Value), Properties), !.

%
% Emit states
%

emit(Grammar, Properties, States) :-
    get_property(Properties, filename, FileName),
    tell(FileName),
    emit_begin(Grammar, Properties, States),
    emit_states(States, Properties),
    emit_end(Properties),
    told,
    true.

emit_begin(Grammar, Properties, States) :-
    get_property(Properties, namespace, NameSpace),
    emit_namespace_begin(NameSpace),
    nl, nl,
    all_symbols(Grammar, States, Symbols),
    emit_symbols_enum(Symbols),
    write('template<typename Base, typename... Args> class '),
    get_property(Properties, classname, ClassName),
    write(ClassName), write(' : public Base {'), nl,
    i1, write('protected:'), nl, nl,
    i1, write('term_parser_gen(Args&... args) : Base(args...) { }'), nl, nl,
    emit_process_state(States).

emit_symbols_enum(Symbols) :-
    i1, write('enum symbol_t {'), nl,
    emit_symbols_enum_body([sym(unknown,unknown,0),sym(t,eof,1000)|Symbols]),
    i1, write('};'), nl, nl.

emit_symbols_enum_body([]).
emit_symbols_enum_body([sym(Type,Name,Ordinal)|Syms]) :-
    emit_symbols_enum_name(Name,Type,Ordinal),
    (Syms = [_|_] -> write(',') ; true), !,
    nl,
    emit_symbols_enum_body(Syms).

emit_symbols_enum_name(Name, _Type, Ordinal) :-
    i2, emit_symbol(Name), write(' = '), write(Ordinal).

emit_process_state(States) :-
    i1, write('void process_state() {'), nl,
    i2, write('switch (Base::current_state()) {'), nl,
    length(States, NumStates),
    emit_process_state_case(0, NumStates),
    i2, write('}'),
    i1, write('}'), nl, nl.

emit_process_state_case(Num, Num) :- !.
emit_process_state_case(N, Num) :-
    i3, write('case '), write(N), write(': '),
    write('state'), write(N), write('(); break;'), nl,
    N1 is N + 1,
    emit_process_state_case(N1, Num).

emit_end(Properties) :-
    write('};'), nl, nl,
    get_property(Properties, namespace, NameSpace),
    emit_namespace_end(NameSpace),
    nl.

emit_namespace_begin([]).
emit_namespace_begin([N|Ns]) :-
    write('namespace '), write(N), write(' { '),
    emit_namespace_begin(Ns).

emit_namespace_end([]).
emit_namespace_end([_|Ns]) :-
    write('} '),
    emit_namespace_end(Ns).

emit_states([], _).
emit_states([State | States], Properties) :-
    emit_state(State, Properties),
    emit_states(States, Properties).

i1 :- write(' ').
i2 :- i1, i1.
i3 :- i1, i1, i1.
i4 :- i2, i2.
i5 :- i3, i2.

emit_state(state(N, _, Actions), _Properties) :-
    i1, write('void state'), write(N), write('() {'), nl,
    i2, write('switch (Base::lookahead().ordinal()) {'), nl,
    emit_shift_actions(Actions, Actions),
    emit_goto_actions(Actions),
    emit_reduce_actions(Actions),
    emit_extra_actions(Actions),
    (\+ member(reduce([empty], _, _), Actions) ->
	i3, write('default: Base::parse_error(); break;'), nl
    ; true),
    i2, write('}'), nl,
    i1, write('}'),
    nl, nl.

emit_shift_actions([shift(Symbol, N)|Actions], AllActions) :-
    check_no_cond_reduce(AllActions, Symbol),
    !, i3, write('case '), emit_symbol(Symbol), write(': '),
    write('Base::shift_and_goto_state('), write(N), write('); break;'), nl,
    emit_shift_actions(Actions, AllActions).
emit_shift_actions([_|Actions], AllActions) :-
    emit_shift_actions(Actions, AllActions).
emit_shift_actions([], _).

check_no_cond_reduce([], _).
check_no_cond_reduce([reduce(SymList,Cond,_)|Actions], Symbol) :-
	Cond \= [], !, \+ member(Symbol, SymList),
	check_no_cond_reduce(Actions, Symbol).
check_no_cond_reduce([_|Actions], Symbol) :-
	check_no_cond_reduce(Actions, Symbol).

emit_goto_actions([goto(Symbol, N)|Actions]) :-
    !, i3, write('case '), emit_symbol(Symbol), write(': '),
    write('Base::goto_state('), write(N), write('); break;'), nl,
    emit_goto_actions(Actions).
emit_goto_actions([_|Actions]) :-
    emit_goto_actions(Actions).
emit_goto_actions([]).

emit_reduce_actions([reduce(Symbols,[],(Head:-Body))|Actions]) :-
    !, emit_cases(Symbols),
    i4,
    emit_reduce_rule((Head:-Body)),
    i4, write('break;'), nl,
    emit_reduce_actions(Actions).
emit_reduce_actions([_|Actions]) :-
    emit_reduce_actions(Actions).
emit_reduce_actions([]).

emit_reduce_rule((Head:-Body)) :-
    length_body(Body,Num),
    write('Base::reduce('), emit_symbol(Head), write(', '),
    write('Base::reduce_'), emit_tokenize_rule((Head:-Body)),
    write('(Base::args('), write(Num), write(')));'), nl.

emit_extra_actions(Actions) :-
	findall(reduce(Symbol,Cond,Rule), 
	       (member(reduce(Symbols,Cond,Rule),Actions),
	       Cond \= [],member(Symbol,Symbols)),
	       Reduce),
        sort(Reduce,ReduceSorted),
	emit_extra_actions1(ReduceSorted, Actions).

emit_extra_actions1([], _).
emit_extra_actions1([reduce(Symbol,Cond,(Head:-Body))|Actions], AllActions) :-
	emit_cases([Symbol]),
	(member(shift(Symbol, N), AllActions) ->
	    i4, write('if ('), write('Base::'), write(Cond), write('(Base::lookahead())) {'), nl,
	    i5, emit_reduce_rule((Head:-Body)),
	    i4, write('} else {'), nl,
	    i5, write('Base::shift_and_goto_state('), write(N), write(');'), nl,
	    i4, write('}'), nl
	; i4, emit_reduce_rule((Head:-Body))
        ), i4, write('break;'), nl,
	emit_extra_actions1(Actions, AllActions).


length_body((_,B), N) :-
    !,
    length_body(B, N1),
    N is N1 + 1.
length_body(_, 1).

emit_cases([]).
emit_cases([Symbol|Symbols]) :-
    i3, 
    (Symbol = empty -> write('default: ') ; write('case '), emit_symbol(Symbol), write(':') ), nl,
    emit_cases(Symbols).

has_empty_symbol([empty|_]) :- !.
has_empty_symbol([_|Syms]) :- has_empty_symbol(Syms).

emit_tokenize_rule((Head:-Body)) :-
    body_to_list(Body,List),
    write(Head),
    write('__'),
    emit_tokenize_rule_list(List).

emit_tokenize_rule_list([]).
emit_tokenize_rule_list([X1,X2|Xs]) :-
    write(X1),
    write('_'),
    !,
    emit_tokenize_rule_list([X2|Xs]).
emit_tokenize_rule_list([X|Xs]) :-
    write(X),
    emit_tokenize_rule_list(Xs).

emit_symbol(Symbol) :-
    Symbol =.. Atoms,
    upcase_atoms(Atoms, UpAtoms),
    write('SYMBOL_'),
    emit_atom_list(UpAtoms).

emit_atom_list([]).
emit_atom_list([A,A1|As]) :-
    !,write(A), write('_'), emit_atom_list([A1|As]).
emit_atom_list([A]) :- write(A).

upcase_atoms([], []).
upcase_atoms([Atom|Atoms], [UpAtom|UpAtoms]) :-
	upcase_atom(Atom,UpAtom),
	upcase_atoms(Atoms,UpAtoms).

body_to_list((A,B), [A|L]) :-
    !, body_to_list(B, L).
body_to_list(X, [X]).

%
% Get all symbols
%

all_symbols(Grammar, States, Symbols) :-
    all_symbols_states(States, Syms),
    sort(Syms, UniqueSyms),
    partition_symbols(UniqueSyms, Grammar, Symbols1),
    sort(Symbols1, Symbols),
    symbol_ordinals(Symbols, nt, 1),
    symbol_ordinals(Symbols, t, 1001).

symbol_ordinals([],_,_).
symbol_ordinals([sym(Type,_,Ordinal)|Syms],Type,Ordinal) :-
    !, Ordinal1 is Ordinal + 1,
    symbol_ordinals(Syms, Type, Ordinal1).
symbol_ordinals([_|Syms],Type,Ordinal) :-
    symbol_ordinals(Syms, Type, Ordinal).

partition_symbols([], _, []).
partition_symbols([S|Symbols], Grammar, [sym(Type,S,_)|Partitioned]) :-
    (terminal(Grammar, S) -> Type = t ; Type = nt), !,
    partition_symbols(Symbols, Grammar, Partitioned).

all_symbols_states([], []).
all_symbols_states([state(_N, _, Actions)|States], Symbols) :-
    all_symbols_actions(Actions, S),
    append(S, Symbols1, Symbols),
    all_symbols_states(States, Symbols1).

all_symbols_actions([shift(S,_)|Actions], [S|Symbols]) :-
    !, all_symbols_actions(Actions, Symbols).
all_symbols_actions([goto(S,_)|Actions], [S|Symbols]) :-
    !, all_symbols_actions(Actions, Symbols).
all_symbols_actions([reduce(S,_Cond,(Head:-_))|Actions], Symbols) :-
    !,
    append([Head|S], Symbols1, Symbols),
    all_symbols_actions(Actions, Symbols1).
    
all_symbols_actions([], []).

%
% Resolve conflicts
%

resolve_conflicts(States, Properties, NewStates) :-
	(get_property(Properties, prefer, _Prefer) ->
	    resolve_conflicts1(States, Properties, NewStates)
	; NewStates = States).

resolve_conflicts1([], _Properties, []).
resolve_conflicts1([State|States], Properties, [NewState|NewStates]) :-
    resolve_conflicts_state(State, Properties, NewState),
    resolve_conflicts1(States, Properties, NewStates).

resolve_conflicts_state(state(N, KernelItems, Actions),
			Properties,
			state(N, KernelItems, NewActions)) :-
    resolve_conflicts_actions(Actions, Properties, NewActions).

eresolve_conflicts_actions(Actions, Properties, NewActions) :-
    action_syms(Actions, ShiftSyms, _GotoSyms, ReduceSyms),
    get_property(Properties, prefer, Prefer),
    (Prefer = shift ->
	 filter_actions(Actions, shift, ShiftSyms, NewActions)
       ; filter_actions(Actions, reduce, ReduceSyms, NewActions)
    ).


%
% Sort actions
%

sort_actions([], []).
sort_actions([state(N, KernelItems, Actions)|States],
	     [state(N, KernelItems, NewActions)|NewStates]) :-
    sort_actions1(Actions, NewActions),
    sort_actions(States, NewStates).

sort_actions1(Actions, SortedActions) :-
    findall(shift(Sym,N), member(shift(Sym,N), Actions),ShiftActions1),
    findall(goto(Sym,N), member(goto(Sym,N), Actions), GotoActions1),
    findall(reduce(Syms,Cond,Rule),member(reduce(Syms,Cond,Rule),Actions),ReduceActions1),
    sort(ShiftActions1, ShiftActions),
    sort(GotoActions1, GotoActions),
    sort(ReduceActions1, ReduceActions),
    append(ShiftActions, GotoActions, Actions1),
    append(Actions1, ReduceActions, SortedActions).
    

%
% Filter actions
%

filter_actions([], _, _, []).
filter_actions([shift(Sym,N)|Actions], Prefer, ExcludedSyms,
	       [shift(Sym,N)|NewActions]) :-
    (Prefer = reduce, \+ member(Sym, ExcludedSyms) ;
     Prefer = shift), !,
    filter_actions(Actions, Prefer, ExcludedSyms, NewActions).
filter_actions([reduce(Syms,Cond,Rule)|Actions], Prefer, ExcludedSyms,
	       [reduce(Syms,Cond,Rule)|NewActions]) :-
    (Prefer = shift, \+ member(Syms, ExcludedSyms) ;
     Prefer = reduce), !,
    filter_actions(Actions, Prefer, ExcludedSyms, NewActions).
filter_actions([goto(Sym,N)|Actions], Prefer, ExcludedSyms, [goto(Sym,N)|NewActions]) :-
    filter_actions(Actions, Prefer, ExcludedSyms, NewActions).
filter_actions([_|Actions], Prefer, ExcludedSyms, NewActions) :-
    filter_actions(Actions, Prefer, ExcludedSyms, NewActions).
	      
%
% Pretty print states
%
print_states(States) :-
    print_states(States, 100000).
print_states([], _).
print_states(_, 0) :- !.
print_states([State|States], Cnt) :-
    print_state(State),
    Cnt1 is Cnt - 1,
    print_states(States, Cnt1).

print_state(state(N, KernelItems, Actions)) :-
    write('--- State '), write(N), write(' ------------------------------'),nl,
    print_kernel_items(KernelItems),
    write('-----------------------------------'),nl,
    print_actions(Actions),
    print_check_actions(Actions),
    nl, nl.

print_kernel_items([]).
print_kernel_items([Item|Items]) :-
    print_kernel_item(Item),
    print_kernel_items(Items).

print_kernel_item(Item) :-
    write('   '), write(Item), nl.

print_actions([]).
print_actions([Action|Actions]) :-
    print_action(Action),
    print_actions(Actions).

print_action(shift(Symbol,N)) :-
	write('   shift on \''), write(Symbol), write('\' '),
	write(' and goto state '), write(N), nl.

print_action(goto(Symbol,N)) :-
	write('   goto on \''), write(Symbol), write('\' '),
	write(' to state '), write(N), nl.

print_action(reduce(LAs,Cond,Rule)) :-
    print_reduce(LAs,Cond,Rule).

print_reduce([LA|LAs],Cond,Rule) :-
    write('   reduce on \''), write(LA), write('\''),
    (Cond \= [] -> write(' if '), write(Cond) ; true),
    write(' with rule '), write(Rule),
    nl,
    print_reduce(LAs,Cond,Rule).
print_reduce([],_,_).

print_check_actions(Actions) :-
    action_syms(Actions, ShiftSyms, GotoSyms, ReduceSyms),
    append(ShiftSyms, GotoSyms, Syms1),
    append(Syms1, ReduceSyms, AllSyms),
    print_check_syms(AllSyms).

action_syms(Actions, ShiftSyms, GotoSyms, ReduceSyms) :-
	findall(Sym, member(shift(Sym,_), Actions), ShiftSyms),
	findall(Sym, member(goto(Sym,_), Actions), GotoSyms),
	findall(SymsList,
	        member(reduce(SymsList,[],_), Actions),
	        ReduceSymsList),
	flatten(ReduceSymsList, ReduceSyms).

print_check_syms([]).
print_check_syms([Sym|Syms]) :-
	(member(Sym, Syms) -> write('Conflict on symbol '), write(Sym), nl ; true),
	print_check_syms(Syms).

%
% Construct state machine
%
% states(+Grammar, +InitialState, -States)
%

states(Grammar, InitState, StatesOut) :-
    states(Grammar, [InitState], InitState, StatesOut1),
    InitState = state(0, [KernelItem], _),
    item_move_to_end(KernelItem, AcceptingItem),
    member(state(N, [AcceptingItem], _), StatesOut1),
    item_to_rule(AcceptingItem, AcceptingRule),
    NewAcceptingActions = [reduce([empty], [], AcceptingRule)],
    AcceptingState = state(N, [AcceptingItem], NewAcceptingActions),
    replace_state(StatesOut1, AcceptingState, StatesOut).

states(Grammar, StatesIn, State, StatesOut) :-
    State = state(_, KernelItems, _Actions),
    closure(Grammar, KernelItems, Closure),
    all_next_symbols(Closure, Symbols),
    state_transitions(Symbols, Grammar, Closure, State, StatesIn, StatesOut1,
		      UpdatedState, NewStates),
    state_reductions(KernelItems, UpdatedState, UpdatedState1),
    replace_state(StatesOut1, UpdatedState1, StatesOut2),
    states_list(NewStates, Grammar, StatesOut2, StatesOut).

states_list([], _, StatesOut, StatesOut).
states_list([State|States], Grammar, StatesIn, StatesOut) :-
    states(Grammar, StatesIn, State, StatesOut1),
    states_list(States, Grammar, StatesOut1, StatesOut).

state_transitions([], _, _, FromState, States, States, FromState, []).
state_transitions([Symbol|Symbols], Grammar, Closure,
		  FromState, StatesIn, StatesOut, UpdatedFromState, NewStates) :-
    select_items(Closure, Symbol, KernelItems),
%    compact_items(KernelItems1, KernelItems),
    (\+ has_state(StatesIn, KernelItems) ->
	 NewStates = [state(N,KernelItems,Actions)|NewStates0]
       ; NewStates = NewStates0),
    add_state(StatesIn, KernelItems, StatesOut1, state(N,KernelItems,Actions)),
%    write('shift \''), write(Symbol), write('\' --> goto '), write(N), nl,
%    lookahead_list(Grammar, KernelItems, ShiftLA),
    (terminal(Grammar, Symbol) ->
	state_add_action(FromState, shift(Symbol, N), FromState1)
     ;  state_add_action(FromState, goto(Symbol, N), FromState1)
    ),
    state_transitions(Symbols, Grammar, Closure, FromState1, StatesOut1,
		      StatesOut, UpdatedFromState, NewStates0).

state_reductions(KernelItems,
		 state(N,KernelItems,Actions),
		 state(N,KernelItems,NewActions)) :-
    items_get_reductions(KernelItems,Reductions),
    append(Actions,Reductions,NewActions).

items_get_reductions([],[]).
items_get_reductions([Item|Items],Reductions) :-
    item_at_end(Item),
    !,
    lookahead(_, Item, LA),
    item_to_rule(Item, Rule),
    (find_reduce_cond(Item, Cond), ! ; Cond = []),
    Reductions = [reduce(LA, Cond, Rule)|Reductions0],
    items_get_reductions(Items, Reductions0).
items_get_reductions([_|Items],Reductions) :-
    items_get_reductions(Items, Reductions).

item_to_rule((Head :- Body), (Head :- NewBody)) :-
    item_to_rule_body(Body, NewBody).

item_to_rule_body((A, 'DOT', _), A) :- !.
item_to_rule_body((A, 'DOT'), A) :- !.
item_to_rule_body((A, B), (A, NewB)) :-
    item_to_rule_body(B, NewB).

item_extra((_Head :- Body), Extra) :-
	item_extra_body(Body, Tail),
	(Extra,_LA) = Tail,
	functor(Extra, {}, _).

item_extra_body('DOT', []) :- !.
item_extra_body(('DOT', A), A) :- !.
item_extra_body((_, B), B1) :- item_extra_body(B, B1).

find_reduce_cond(Item, Cond) :-
	item_extra(Item, Extra),
	Extra =.. [{}, ExtraBody],
	body_to_list(ExtraBody, ExtraList),
	member(reduce_cond(Cond), ExtraList).

has_state([state(_,KernelItems,_)|_], KernelItems) :- !.
has_state([_|States], KernelItems) :- has_state(States, KernelItems).

add_state(StatesIn, KernelItems, StatesOut, State) :-
    add_state(StatesIn, 0, KernelItems, StatesOut, State).

add_state([], Cnt, KernelItems, [State], State) :-
%    write('new state '), write(Cnt), nl,
    State = state(Cnt, KernelItems, []).
add_state([State|States], Cnt, KernelItems, [State|StatesOut], Found) :-
    State = state(_N,StateKernelItems,_StateActions),
    (\+ \+ KernelItems = StateKernelItems ->
	 Found = State,
	 States = StatesOut
   ; Cnt1 is Cnt + 1,
     add_state(States, Cnt1, KernelItems, StatesOut, Found)).

replace_state([state(N,_,_)|States], state(N,KernelItems,Actions),
	      [state(N,KernelItems,Actions)|States]).
replace_state([State|States], UpdatedState, [State|NewStates]) :-
    replace_state(States, UpdatedState, NewStates).

state_add_action(state(N,KernelItems,Actions), Action,
		 state(N,KernelItems,NewActions)) :-
    append(Actions, [Action], NewActions).

select_items([], _, []).
select_items([Item|Items], Symbol, KernelItems) :-
    write('select items'), nl,
    (item_next_symbol(Item, Symbol0), match_heads(Symbol0, Symbol) ->
	item_move_next(Item, NewItem),
	KernelItems = [NewItem|KernelItems0]
      ; KernelItems0 = KernelItems
    ),
    select_items(Items, Symbol, KernelItems0).

%compact_items([(head(N) :- Body) | Items],NewItems) :-
%	compact_item_select(Items, (head('<'(N)) :- Body), Found, Rest),
%
%compact_item_select((head(N) :- Body1), (head('<'(N)) :- Body), Found, Rest),
%	strip_lookaheads(Body1, StrippedBody),
%	strip_lookaheads(Body, StrippedBody),
%	% We have a match, so collapse them together.
%	Found = 

%item_equal_structure((HeadA :- BodyA), (HeadB :- BodyB)) :-
%	(match_heads(HeadA, HeadB) ; match_heads(HeadB, HeadA)),
%	item_equal_structure_body(BodyA, BodyB).
%
%item_equal_structure_body((A1, A2), (B1, B2)) :-
%	item_equal_structure_body(A1, B1),
%	item_equal_structure_body(A2, B2).
%item_equal_structure_body([_|_], []) :- !.
%item_equal_structure_body([], [_|_]) :- !.
%item_equal_structure_body([_|_], 'DOT') :- !.
%item_equal_structure_body([], 'DOT') :- !.
%item_equal_structure_body('DOT', [_|_]) :- !.
%item_equal_structure_body('DOT', []) :- !.
	

all_next_symbols(Items, Symbols) :-
    all_next_symbols(Items, [], Symbols).
all_next_symbols([], SymbolsOut, SymbolsOut).
all_next_symbols([Item|Items], SymbolsIn, SymbolsOut) :-
    item_next_symbol(Item, Symbol),
    (member(Symbol, SymbolsIn) ->
	 SymbolsIn1 = SymbolsIn
       ; SymbolsIn1 = [Symbol | SymbolsIn]
    ),
    all_next_symbols(Items, SymbolsIn1, SymbolsOut).
all_next_symbols([_|Items], SymbolsIn, SymbolsOut) :-
    all_next_symbols(Items, SymbolsIn, SymbolsOut).

%
% closure(+Grammar,+Item,-Closure)
%
% Compute closure for given Item. An item is a clause with
% a 'DOT', symbolizing that we have read a prefix for it.
%

closure(Grammar,KernelItems,Closure) :-
    write('-------- closure '), nl,
    closure_kernel(KernelItems,Grammar,[],Closure1),
    % profile,
    write('-------- closure done'), nl,
    sort(Closure1, Closure2),
    closure_compact(Closure2, Closure3),
    reverse(Closure3, Closure).

closure_kernel([],_,Closure,Closure).
closure_kernel([KernelItem|KernelItems],Grammar,ClosureIn,ClosureOut) :-
    closure(Grammar,KernelItem,ClosureIn,ClosureIn1),
    closure_kernel(KernelItems,Grammar,ClosureIn1,ClosureOut).

closure(Grammar,Item,ClosureIn,ClosureOut) :-
    \+ member(Item, ClosureIn), % We have not seen this before
    !, % Do not backtrack.
    copy_term(Item, ItemCopy),
    (item_move_next(ItemCopy, ItemCopyNext),
     lookahead(Grammar, ItemCopyNext, LA),
     ! ; LA = []),
    % length(ClosureIn, N), write('ClosureIn '), write(N), nl,
    match(Grammar, ItemCopy, MatchedItems),
    ClosureIn1 = [Item | ClosureIn],
    add_lookaheads(MatchedItems, LA, NewItems),
    closure_list(NewItems, Grammar, ClosureIn1, ClosureOut).
closure(_,_,Closure,Closure).

closure_list([], _, Closure, Closure).
closure_list([Item|Items], Grammar, ClosureIn, ClosureOut) :-
    closure(Grammar, Item, ClosureIn, ClosureIn1),
    closure_list(Items, Grammar, ClosureIn1, ClosureOut).

closure_compact([], []).
closure_compact([Clause1,Clause2|Clauses], NewClauses) :-
    Clause1 = (Head1 :- Body1),
    Clause2 = (Head2 :- Body2),
    Head1 == Head2,
    same_bodies_ignore_lookahead(Body1, Body2),
    !,
    item_lookahead_body(Body1, LA1),
    item_lookahead_body(Body2, LA2),
    append(LA1, LA2, LA0),
    sort(LA0, LA),
    strip_lookaheads_body(Body1, NakedBody),
    add_lookaheads_body(NakedBody, LA, MergedBody),
    ClauseMerged = (Head1 :- MergedBody),
    closure_compact([ClauseMerged|Clauses], NewClauses).
closure_compact([C|Cs], [C|NewCs]) :-
    closure_compact(Cs,NewCs).

print_closure([]).
print_closure([Item|Items]) :-
        write(Item), nl,
	print_closure(Items).

%
% lookahead(+Grammar, +Item, -LA)
%
% Compute the set of lookaheads for the given item.
%
lookahead(Grammar, Item, LA) :-
    lookahead(Grammar, Item, [], _, LA).
lookahead_list(Grammar, Items, LA) :-
	lookahead_list(Items, Grammar, [], _, LA).

lookahead(_, Item, Visited, Visited, LA) :-
        item_at_end(Item), !, item_lookahead(Item, LA).
lookahead(Grammar, Item, VisitedIn, VisitedOut, LA) :-
        \+ member(Item, VisitedIn),
        item_next_symbol(Item, Symbol),
	!,
	Visited1 = [Item | VisitedIn],
	(terminal(Grammar, Symbol) ->
	  LA = [Symbol], VisitedOut = VisitedIn
        ; copy_term(Item, ItemCopy),
	  match(Grammar, ItemCopy, MatchedItems),
	  lookahead_list(MatchedItems, Grammar, Visited1, Visited2, LA1),
	  (LA1 = [] ->
	       item_move_next(Item, ItemNext),
	       lookahead(Grammar, ItemNext, Visited2, VisitedOut, LA)
	 ; LA = LA1, VisitedOut = Visited2
	  )
        ).
lookahead(_, _, Visited, Visited, []).

lookahead_list([], _, Visited, Visited, []).
lookahead_list([Item|Items], Grammar, VisitedIn, VisitedOut, LA) :-
    (member(Item, VisitedIn) ->
        Visited1 = VisitedIn
      ; lookahead(Grammar, Item, VisitedIn, Visited1, LAItem)
    ),
    lookahead_list(Items, Grammar, Visited1, VisitedOut, LAItems),
    append(LAItem, LAItems, LA0),
    sort(LA0, LA).

item_at_end((_ :- Body)) :-
    item_at_end_body(Body).

item_at_end_body(('DOT', [_|_])) :- !.
item_at_end_body(('DOT', [])) :- !.
item_at_end_body('DOT') :- !.
item_at_end_body(('DOT', A)) :- functor(A, {}, _), !.
item_at_end_body(('DOT', A, _)) :- functor(A, {}, _), !.
item_at_end_body((_,B)) :-
    item_at_end_body(B).


item_lookahead((_ :- Body), LA) :-
    item_lookahead_body(Body, LA).

item_lookahead_body([], []) :- !.
item_lookahead_body([X|Xs], [X|Xs]) :- !.
item_lookahead_body((_,B), LA) :-
    !, item_lookahead_body(B, LA).
item_lookahead_body(_, []).

item_move_to_end(Item, Item) :- item_at_end(Item), !.
item_move_to_end(Item, NewItem) :-
    item_move_next(Item, Item1), item_move_to_end(Item1, NewItem).

item_move_next((Head :- Body), (Head :- NewBody)) :-
    write('item_move_next...'), nl,
    item_move_next_body(Body, NewBody).

item_move_next_body(('DOT', A, B), (A, 'DOT', B)) :-
	!, functor(A, F,_), F \= {}.
item_move_next_body(('DOT', A), (A, 'DOT')) :- !, functor(A, F, _), F \= {}.
item_move_next_body((A, B), (A, NewB)) :-
    item_move_next_body(B, NewB).

terminal([], _).
terminal([(Head :- _) | Clauses], Symbol) :-
	functor(Symbol, F1, N1),
        functor(Head, F2, N2),
        (F1 \= F2 ; N1 \= N2),
        !,
	terminal(Clauses, Symbol).

lookahead_symbol(Grammar, Symbol, LA) :-
        terminal(Grammar, Symbol), !, LA = Symbol.
lookahead_symbol(Grammar, NonTerminal, LA) :-
        match_symbol(Grammar, NonTerminal, MatchedItems),
	lookahead_list(MatchedItems, Grammar, LA).

add_lookaheads([], _, []).
add_lookaheads([Item|Items], LA, [NewItem | NewItems]) :-
	add_lookaheads_item(Item, LA, NewItem),
	add_lookaheads(Items, LA, NewItems).

add_lookaheads_item(C, [], C) :- !.
add_lookaheads_item((Head :- Body), LA, (Head :- NewBody)) :-
	add_lookaheads_body(Body, LA, NewBody).

add_lookaheads_body([X | Xs], LA, NewLA) :-
	!, append([X|Xs], LA, NewLA0),
	sort(NewLA0, NewLA).
add_lookaheads_body((A,B), LA, (A,B1)) :-
	!, add_lookaheads_body(B, LA, B1).
add_lookaheads_body(X, LA, (X,LA)).

%
% match(+Grammar, +Item, -FollowItems)
%
% Return items that immediately follows given item.
%
match(Grammar, Item, FollowItems) :-
	item_next_symbol(Item, Symbol),
	!,
	match_symbol(Grammar, Symbol, FollowItems).
match(_, _, []).

item_next_symbol((_ :- Body), Symbol) :-
    item_next_symbol_body(Body, Symbol).

item_next_symbol_body(('DOT', X), Symbol) :-
    !, functor(X, F, _), F \= {}, item_next_symbol_found(X, Symbol).
item_next_symbol_body((_,B), Symbol) :-
    item_next_symbol_body(B, Symbol).

item_next_symbol_found((Symbol, _), Symbol) :-
	!, functor(Symbol,F,_), F \= {}, \+ is_list(Symbol).
item_next_symbol_found(Symbol, Symbol) :-
	functor(Symbol,F,_), F \= {}, \+ is_list(Symbol).

match_symbol([], _, []).
match_symbol([C | Cs], Symbol, [Match | Matched]) :-
	C = (Head :- _),
	match_heads(Head, Symbol),
	!,
	copy_term(C, (HeadCopy :- BodyCopy)),
	copy_term(Symbol, CopySymbol),
	unify_heads(HeadCopy, CopySymbol),
	Match1 = (HeadCopy :- 'DOT', BodyCopy),
	process_arithmetics(Match1, Match),
	match_symbol(Cs, Symbol, Matched).
match_symbol([_ | Cs], Symbol, Matched) :-
         match_symbol(Cs, Symbol, Matched).

match_heads(X, Y) :-
	write('match_heads '), write(X), write( ' '), write(Y), nl,
	X =.. [XF|XArgs],
	Y =.. [YF|YArgs],
	XF = YF,
	match_args(XArgs, YArgs),
	write('match heads done'), nl.

match_args([], []).
match_args([X|Xs], [Y|Ys]) :-
	match_arg(X,Y),
	match_args(Xs,Ys).

match_arg(N,'<'(M)) :- number(N), !, N < M.
match_arg(N,M) :- \+ N \= M.

unify_heads(X, Y) :-
	X =.. [XF|XArgs],
	Y =.. [YF|YArgs],
	XF = YF,
	unify_args(XArgs, YArgs).

unify_args([], []).
unify_args([X|Xs], [Y|Ys]) :-
	unify_arg(X,Y),
	unify_args(Xs,Ys).

unify_arg(N,'<'(M)) :- number(N), !, N < M.
unify_arg(N,M) :- M = N.


process_arithmetics(X, NewX) :-
	var(X), !, NewX = X.
process_arithmetics('<'('<'(X)), '<'(X)) :- !.
process_arithmetics([X|Xs], New) :-
	!, process_arithmetics_list([X|Xs], New).
process_arithmetics((Head :- Body), (NewHead :- NewBody)) :-
	!,
	process_arithmetics(Head, NewHead),
	process_arithmetics(Body, NewBody).
process_arithmetics((A,B), (NewA, NewB)) :-
	!,
	process_arithmetics(A, NewA),
	process_arithmetics(B, NewB).
process_arithmetics(X, NewX) :-
	X =.. [F|Args], Args \= [], !,
	process_arithmetics_list(Args, NewArgs),
	NewX =.. [F|NewArgs].
process_arithmetics(X, X).

process_arithmetics_list([],[]).
process_arithmetics_list([X|Xs], [NewX|NewXs]) :-
	process_arithmetics(X,NewX),
	process_arithmetics_list(Xs,NewXs).

same_bodies_ignore_lookahead('DOT', 'DOT') :- !.
same_bodies_ignore_lookahead((A, B1), (A, B2)) :-
    !, same_bodies_ignore_lookahead(B1, B2).
same_bodies_ignore_lookahead((A, _), A) :- !.
same_bodies_ignore_lookahead(A, (A, _)) :- !.
same_bodies_ignore_lookahead(A, [_|_]) :- \+ functor(A, ',', _), !.
same_bodies_ignore_lookahead([_|_], B) :- \+ functor(B, ',', _), !.
same_bodies_ignore_lookahead(A, []) :- \+ functor(A, ',', _), !.
same_bodies_ignore_lookahead([], B) :- \+ functor(B, ',', _).
    

strip_lookaheads((Head :- Body), (Head :- NewBody)) :-
	strip_lookaheads_body(Body, NewBody).

strip_lookaheads_body((A, [_|_]), A) :- !.
strip_lookaheads_body((A, []), A) :- !.
strip_lookaheads_body((A, B), (A, NewB)) :-
	!, strip_lookaheads_body(B, NewB).
strip_lookaheads_body(A, A).
	

follow((_ :- Body), Symbol) :-
	follow_body(Body, Symbol).

follow_body(('DOT', Found), Symbol) :- !, follow_found(Found, Symbol).
follow_body((_, Y), Symbol) :- follow_body(Y, Symbol).

follow_found((A,_), A) :- !.
follow_found(A, A).


?- main.
% Expect: true

