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

test(Q) :-
    read_grammar(G),
    closure(G, (start :- 'DOT', subterm(_), full_stop), Q).

%
% read_grammar(-G)
%
% Read grammar from prolog_grammar.pl. Return list of clauses
% (represented as terms) in G.
%
read_grammar(G) :-
    open('prolog_grammar.pl', read, F),
    read_clauses(F,G),
    close(F).

read_clauses(F, []) :- at_end_of_stream(F), !.
read_clauses(F, [C|Cs]) :- read(F, C), read_clauses(F, Cs).

%
% closure(+Grammar,+Item,-Closure)
%
% Compute closure for given Item. An item is a clause with
% a 'DOT', symbolizing that we have read a prefix for it.
%
closure(Grammar,Item,Closure) :-
    closure(Grammar,Item,[],Closure).

closure(Grammar,Item,ClosureIn,ClosureOut) :-
    \+ member(Item, ClosureIn), % We have not seen this before
    !, % Do not backtrack.
    copy_term(Item, ItemCopy),
    lookahead(Grammar, ItemCopy, LA),
    match(Grammar, Item, MatchedItems),
    add_lookaheads(MatchedItems, LA, NewItems),
    closure_list(NewItems, Grammar, ClosureIn, ClosureOut).
closure(_,_,Closure,Closure).

closure_list([], _, Closure, Closure).
closure_list([Item|Items], Grammar, ClosureIn, ClosureOut) :-
    closure(Item, Grammar, ClosureIn, ClosureOut0),
    closure_list(Items, Grammar, ClosureOut0, ClosureOut).

%
% lookahead(+Grammar, +Item, -LA)
%
% Compute the set of lookaheads for the given item.
%
%lookahead(Grammar, Item, LA) :-
%	match(Grammar, Item, Matched).


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
	follow(Item, NonTerminal),
	match_nt(Grammar, NonTerminal, FollowItems).

match_nt([], _, []).
match_nt([C | Cs], NonTerminal, [Match | Matched]) :-
	C = (Head :- _),
	\+ \+ Head = NonTerminal,
	!,
	copy_term(C, (HeadCopy :- BodyCopy)),
	Match = (HeadCopy :- ('DOT', BodyCopy)),
	HeadCopy = NonTerminal,
	match_nt(Cs, NonTerminal, Matched).
match_nt([_ | Cs], NonTerminal, Matched) :-
	match_nt(Cs, NonTerminal, Matched).

follow((_ :- Body), NonTerminal) :-
	follow_body(Body, NonTerminal).

follow_body(('DOT', Found), NonTerminal) :- !, follow_found(Found,NonTerminal).
follow_body((_, Y), NonTerminal) :- follow_body(Y, NonTerminal).

follow_found((A,_), A) :- !.
follow_found(A, A).
