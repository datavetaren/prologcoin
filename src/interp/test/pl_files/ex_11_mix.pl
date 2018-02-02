%
% Test mixing compiled with non-comiled
%

% Meta: debug off
% Meta: dont-compile simple_interp/3
% Meta: dont-compile more_simple/2

foo(X, Y, Z, A) :-
    inside_wam(X, Y),
    simple_interp(X, Y, Z),
    back_to_wam(X, Y, Z, A).

inside_wam(X, Y) :-
    first_wam(X, A),
    middle_wam(X, B),
    last_wam(A, B, Y).

simple_interp(X, Y, Z) :-
    Q = simple(X, Y),
    more_simple(Q, Z).
more_simple(Q, Z) :-
    Z = more(Q).

first_wam(X, A) :-
    A = foo(X).
middle_wam(X, B) :-
    B = bar(X).
last_wam(A, B, Y) :-
    Y = combined(A,B).

back_to_wam(X, Y, Z, back(X,Y,Z)).

?- foo(10, Q1, Q2, Q3).
% Expect: Q1 = combined(foo(10), bar(10)), Q2 = more(simple(10, combined(foo(10), bar(10)))), Q3 = back(10, combined(foo(10), bar(10)), more(simple(10, combined(foo(10), bar(10)))))
% Expect: end

