%
% Testing disprove operator
%

%
% Check that something is not true
%

member(X, [X|_]).
member(X, [_|Xs]) :- member(X, Xs).

dp1(X, Y) :-
    X = 3, \+ member(X, [1,2,4,6,9]), Y = 2.

?- dp1(Q1, Q2).
% Expect: Q1 = 3, Q2 = 2
% Expect: end

%
% Check fail when something is true
%

dp2(X, Y) :-
    \+ member(X, [1,2,3,4,5]), Y = 42.

?- dp2(3, Q3).
% Expect: fail

member0([X|_], X).
member0([_|Xs], X) :- member0(Xs, X).

dp3(X, Y) :-
    X = 3, \+ member0([1,2,4,6,9], X), Y = 2.

?- dp3(Q4, Q5).
% Expect: Q4 = 3, Q5 = 2
% Expect: end

dp4(X, Y) :-
    \+ member0([1,2,3,4,5], X), Y = 42.

?- dp4(3, Q6).
% Expect: fail

dp5(X, Y) :-
    dp5_get(X, Y),
    \+ member0([1,2,3,4,5], X),
    !,
    dp5_doit(X, Y),
    dp5_doit(X, Y).

dp5_get(10, 10).
dp5_doit(X, Y) :-
    X = Y.

?- dp5(10, Q7).
% Expect: Q7 = 10
% Expect: end

%
% Double disprove
%
dp6(X) :-
    (\+ \+ X = foo -> X = 1 ; X = 2).

?- dp6(X).
% Expect: X = 1
% Expect: end

dp7(A) :-
    \+ member(3, [1,2,3,4,5]).
dp7(A) :-
    A = 100.
?- dp7(Q7).
% Expect: Q7 = 100
% Expect: end
