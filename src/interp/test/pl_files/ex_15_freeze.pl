%
% Include standard lib
%

:- [std].

%
% Freeze simple
%

foo(X,Y) :- freeze(X, (Y = bound(X))).

?- foo(Q1, Q2), Q1 = 42.
% Expect: Q1 = 42, Q2 = bound(42)
% Expect: end

%
% Freeze with backtracking
%

foo2(X,Y) :- member(A, [1,2,3,4]), freeze(X, (Y = bound(X,A))).

?- foo2(Q3, Q4), Q4 = 4711.
% Expect: Q4 = 4711
% Expect: Q4 = 4711
% Expect: Q4 = 4711
% Expect: Q4 = 4711
% Expect: end

?- foo2(Q5,Q6), Q5 = 4711.
% Expect: Q5 = 4711, Q6 = bound(4711,1).
% Expect: Q5 = 4711, Q6 = bound(4711,2).
% Expect: Q5 = 4711, Q6 = bound(4711,3).
% Expect: Q5 = 4711, Q6 = bound(4711,4).
% Expect: end

%
% Nested freeze
%

foo3(X, Y, A, B) :- freeze(A, freeze(B, Y = bound(X,A,B))).

?- foo3(Q7,Q8,Q9,Q10), Q9 = 1, Q7 = 42, Q10 = 4711.
% Expect: Q7 = 42, Q8 = bound(42, 1, 4711), Q9 = 1, Q10 = 4711.
% Expect: end

%
% Nested interpreted freeze
%

foo4(X, Y, A, B) :- W = freeze(B, Y = bound(X,A,B)), freeze(A, W).
?- foo4(Q11,Q12,Q13,Q14), Q13 = 1, Q11 = 42, Q14 = 4711.
% Expect: Q11 = 42, Q12 = bound(42, 1, 4711), Q13 = 1, Q14 = 4711.
% Expect: end
