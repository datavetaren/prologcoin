% Meta: fileio on

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

?- main(Q1).
% Actual: Q1 = true
% Expect: Q1 = true
