%
% Testing disprove operator
%

member(X, [X|_]).
member(X, [_|Xs]) :- member(X, Xs).

%
% Check that something is not true
%

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

?- dp2(3, Q6).
% Expect: fail



    
