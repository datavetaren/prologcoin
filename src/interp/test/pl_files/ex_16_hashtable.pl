% Meta: WAM-only
fill_table(0) :- !.
fill_table(N) :-
    assert(foo:bar(N)),
    N1 is N - 1,
    fill_table(N1).

?- fill_table(100).
% Expect: true

% Lookups
lookup(0) :- !.
lookup(N) :-
    foo:bar(N),
    N100 is N + 100,
    \+ foo:bar(N100),
    N1 is N - 1,
    lookup(N1).

% check
check0(P) :- status_predicate(foo:bar/1, P0), lookup(100), status_predicate(foo:bar/1, P1), P is P1 - P0.

?- check0(P).
% Expect: P = 500

% Then add a variable at foo:bar; it's no longer a "hash table"...
?- assert(foo:bar(X) :- X = default).
% Expect: true

% Now let's see how the lookup works now (it should be a lot worse.)
check1(P) :- status_predicate(foo:bar/1, P0), lookup(100), status_predicate(foo:bar/1, P1), P is P1 - P0.

?- check1(P).
% Expect: P = 20600

