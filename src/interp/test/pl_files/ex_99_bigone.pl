% Meta: fileio on

% Read in standard library
:- [std].

%
% Construct state machine for LR parser
%
main(Q) :-
    read_grammar_and_properties('term_grammar.pl', G),
    extract_properties(G, Properties, Grammar),
    StartItem = [(start :- 'DOT', subterm_1200, full_stop)],
    Start = state(0, StartItem, []),
    Q = true.
%    states(Grammar, Start, States),
%    % resolve_conflicts(States, Properties, States1),
%    sort_actions(States, NewStates),
%    emit(Grammar, Properties, NewStates),
%    tell('states.txt'),
%    print_states(NewStates),
%    told.

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
% Construct state machine
%
% states(+Grammar, +InitialState, -States)
%

states(Grammar, InitState, StatesOut) :-
    states(Grammar, [InitState], InitState, StatesOut1).
%    InitState = state(0, [KernelItem], _),
%    item_move_to_end(KernelItem, AcceptingItem),
%    member(state(N, [AcceptingItem], _), StatesOut1),
%    item_to_rule(AcceptingItem, AcceptingRule),
%    NewAcceptingActions = [reduce([empty], [], AcceptingRule)],
%    AcceptingState = state(N, [AcceptingItem], NewAcceptingActions),
%    replace_state(StatesOut1, AcceptingState, StatesOut).

states(Grammar, StatesIn, State, StatesOut) :-
    State = state(_, KernelItems, _Actions),
    closure(Grammar, KernelItems, Closure).
%    all_next_symbols(Closure, Symbols),
%    state_transitions(Symbols, Grammar, Closure, State, StatesIn, StatesOut1,
%		      UpdatedState, NewStates),
%    state_reductions(KernelItems, UpdatedState, UpdatedState1),
%    replace_state(StatesOut1, UpdatedState1, StatesOut2),
%    states_list(NewStates, Grammar, StatesOut2, StatesOut).

%
% closure(+Grammar,+Item,-Closure)
%
% Compute closure for given Item. An item is a clause with
% a 'DOT', symbolizing that we have read a prefix for it.
%

closure(Grammar,KernelItems,Closure) :-
    closure_kernel(KernelItems,Grammar,[],Closure1),
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
     lookahead(Grammar, ItemCopyNext, LA), ! ; LA = []),
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


?- main(Q1).
% Actual: Q1 = true
% Expect: Q1 = true
