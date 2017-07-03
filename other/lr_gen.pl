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

%
% TODO:
%
% What remains for the state machine:
%
% We need to collapse items where only lookaheads are different.
% Also we'd like to collapse even when the heads differ in the
% case of f(N) and f('<'(N)). This will bring us closer to LALR(1)
% (instead of LR(1)). Should reduce both conflicts and number of
% states.
%
%

test :-
	read_grammar(G),
	StartItem = [(start :- 'DOT', subterm_1200, full_stop)],
	Start = state(0, StartItem, []),
	states(G, Start, States),
	tell('states.txt'),
	print_states(States),
	told,
%    closure(G, StartItem, Closure),
%    print_closure(Closure),
    true.
%    lookahead(G, (start :- 'DOT', subterm(_), full_stop), Q).

%
% read_grammar(-G)
%
% Read grammar from prolog_grammar.pl. Return list of clauses
% (represented as terms) in G.
%
read_grammar(G) :-
    open('prolog_grammar2.pl', read, F),
    read_clauses(F,G),
    close(F).

read_clauses(F, []) :- at_end_of_stream(F), !.
read_clauses(F, Cs) :-
        read(F, C),
	(C = end_of_file -> Cs = []
       ; Cs = [C|Cs1], read_clauses(F, Cs1)
	).

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

print_action(reduce(LA,Rule)) :-
    print_reduce(LA,Rule).

print_reduce([LA|LAs],Rule) :-
    write('   reduce on \''), write(LA), write('\' with rule '), write(Rule),
    nl,
    print_reduce(LAs,Rule).
print_reduce([],_).

print_check_actions(Actions) :-
	findall(Sym, member(shift(Sym,_), Actions), ShiftSyms),
	findall(SymsList, member(reduce(SymsList,_), Actions), ReduceSymsList),
	flatten(ReduceSymsList, ReduceSyms),
	append(ShiftSyms, ReduceSyms, AllSyms),
	print_check_syms(AllSyms).

print_check_syms([]).
print_check_syms([Sym|Syms]) :-
	(member(Sym, Syms) -> write('Conflict on symbol '), write(Sym), nl ; true),
	print_check_syms(Syms).

%
% Construct state machine
%
% states(+Grammar, +InitialState, -States)
%

states(Grammar, State, StatesOut) :-
    states(Grammar, [State], State, StatesOut).

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
    state_add_action(FromState, shift(Symbol, N), FromState1),
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
    Reductions = [reduce(LA, Rule)|Reductions0],
    items_get_reductions(Items, Reductions0).
items_get_reductions([_|Items],Reductions) :-
    items_get_reductions(Items, Reductions).

item_to_rule((Head :- Body), (Head :- NewBody)) :-
    item_to_rule_body(Body, NewBody).

item_to_rule_body((A, ('DOT', _)), A) :- !.
item_to_rule_body((A, 'DOT'), A) :- !.
item_to_rule_body((A, B), (A, NewB)) :-
    item_to_rule_body(B, NewB).

has_state([state(_,KernelItems,_)|_], KernelItems) :- !.
has_state([_|States], KernelItems) :- has_state(States, KernelItems).

add_state(StatesIn, KernelItems, StatesOut, State) :-
    add_state(StatesIn, 0, KernelItems, StatesOut, State).

add_state([], Cnt, KernelItems, [State], State) :-
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
    !,
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
    closure_kernel(KernelItems,Grammar,[],Closure).

closure_kernel([],_,Closure,Closure).
closure_kernel([KernelItem|KernelItems],Grammar,ClosureIn,ClosureOut) :-
    closure(Grammar,KernelItem,ClosureIn,ClosureIn1),
    closure_kernel(KernelItems,Grammar,ClosureIn1,ClosureOut).

closure(Grammar,Item,ClosureIn,ClosureOut) :-
%    write('Check Item '), write(Item), nl,
    \+ member(Item, ClosureIn), % We have not seen this before
    !, % Do not backtrack.
    copy_term(Item, ItemCopy),
    (item_move_next(ItemCopy, ItemCopyNext), lookahead(Grammar, ItemCopyNext, LA)
     ; LA = []),
    match(Grammar, ItemCopy, MatchedItems),
%    write('Matched '), write(MatchedItems), nl,
    add_lookaheads(MatchedItems, LA, NewItems),
    append(ClosureIn, [Item], ClosureIn1),
    closure_list(NewItems, Grammar, ClosureIn1, ClosureOut).
closure(_,_,Closure,Closure).

closure_list([], _, Closure, Closure).
closure_list([Item|Items], Grammar, ClosureIn, ClosureOut) :-
    closure(Grammar, Item, ClosureIn, ClosureIn1),
    closure_list(Items, Grammar, ClosureIn1, ClosureOut).

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
    lookahead(Grammar, Item, VisitedIn, Visited1, LAItem),
    lookahead_list(Items, Grammar, Visited1, VisitedOut, LAItems),
    append(LAItem, LAItems, LA0),
    sort(LA0, LA).

item_at_end((_ :- Body)) :-
    item_at_end_body(Body).

item_at_end_body(('DOT', [_|_])) :- !.
item_at_end_body(('DOT', [])) :- !.
item_at_end_body('DOT') :- !.
item_at_end_body((_,B)) :-
    item_at_end_body(B).


item_lookahead((_ :- Body), LA) :-
    item_lookahead_body(Body, LA).

item_lookahead_body([], []) :- !.
item_lookahead_body([X|Xs], [X|Xs]) :- !.
item_lookahead_body((_,B), LA) :-
    !, item_lookahead_body(B, LA).
item_lookahead_body(_, []).

item_move_next((Head :- Body), (Head :- NewBody)) :-
    item_move_next_body(Body, NewBody).

item_move_next_body(('DOT', (A, B)), (A, ('DOT', B))) :- !.
item_move_next_body(('DOT', A), (A, 'DOT')) :- !.
item_move_next_body((A, B), (A, NewB)) :-
    item_move_next_body(B, NewB).

terminal([], _).
terminal([(Head :- _) | Clauses], Symbol) :-
	functor(Symbol, _, 0),
        Head \= Symbol,
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
    !, item_next_symbol_found(X, Symbol).
item_next_symbol_body((_,B), Symbol) :-
    item_next_symbol_body(B, Symbol).

item_next_symbol_found((Symbol, _), Symbol) :- !, \+ is_list(Symbol).
item_next_symbol_found(Symbol, Symbol) :- \+ is_list(Symbol).

match_symbol([], _, []).
match_symbol([C | Cs], Symbol, [Match | Matched]) :-
	C = (Head :- _),
	match_heads(Head, Symbol),
	!,
	copy_term(C, (HeadCopy :- BodyCopy)),
	copy_term(Symbol, CopySymbol),
	unify_heads(HeadCopy, CopySymbol),
	Match1 = (HeadCopy :- ('DOT', BodyCopy)),
	process_arithmetics(Match1, Match),
	match_symbol(Cs, Symbol, Matched).
match_symbol([_ | Cs], Symbol, Matched) :-
	match_symbol(Cs, Symbol, Matched).

match_heads(X, Y) :-
%	write('match_heads '), write(X), write( ' '), write(Y), nl,
	X =.. [XF|XArgs],
	Y =.. [YF|YArgs],
	XF = YF,
	match_args(XArgs, YArgs).

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
