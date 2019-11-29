%
% Operator =..
%

ex1(X) :-
    foo(1,10) =.. X.

?- ex1(Q1).
% Expect: Q1 = [foo,1,10]
% Expect: end

ex2(A,B,C) :-
    foo(B,bar) =.. [A,B,C].

?- ex2(Q2,Q3,Q4).
% Expect: Q2 = foo, Q4 = bar
% Expect: end

ex3(X) :-
    X =.. [10].

?- ex3(Q5).
% Expect: Q5 = 10
% Expect: end

ex4(X) :-
    10 =.. X.

?- ex4(Q6).
% Expect: Q6 = [10]
% Expect: end

ex5(X) :-
    X =.. Y.
?- ex5(Q7).
% Expect: =../2: Arguments are not sufficiently instantiated

ex6(X,Y) :-
   foo(X,X) =.. [Y,foo,bar].
?- ex6(Q8,Q9).
% Expect: fail

ex7(X) :-
   foo(X,X) =.. [X,X,X].
?- ex7(Q10).
% Expect: Q10 = foo
% Expect: end

ex8(A,B) :-
   foo(bar(42)) =.. [A|B].
?- ex8(Q11, Q12).
% Expect: Q11 = foo, Q12 = [bar(42)]
% Expect: end

ex9(A,B,C) :-
   foo(1,bar(B),b(b(c))) =.. [A,B|C].
?- ex9(Q13,Q14,Q15).
% Expect: Q13 = foo, Q14 = 1, Q15 = [bar(1),b(b(c))]
% Expect: end

ex10(A,B) :-
   foo =.. [A|B].
?- ex10(Q16,Q17).
% Expect: Q16 = foo, Q17 = []
% Expect: end

ex11(A) :-
   A =.. [foo,bar].
?- ex11(Q18).
% Expect: Q18 = foo(bar)
% Expect: end

ex12(A, B) :-
   T = (1,2,3,4),
   sub12(T, A, B).
sub12((A,B), A, B).
   
?- ex12(A, B).
% Expect: A = 1, B = (2, 3, 4)
% Expect: end

ex13(T, A, B) :-
   T = {1,2,3,4},
   sub13(T, A, B).
sub13({A,B}, A, B).
?- ex13(T, A, B).
% Expect: T = {1,2,3,4}, A = 1, B = (2, 3, 4).
% Expect: end

ex14(T) :-
   T = {1,{2,3},4}.
?- ex14(T).
% Expect: T = {1, {2, 3}, 4}
% Expect: end

%
% Test unification 
%

ex15(Q) :-
   T = {foo}, F = {},
   functor(T, F1, A1),
   A1 = 1,
   F1 = F,
   Q = F1-A1.
?- ex15(Q).
% Expect: Q = {}-1
% Expect: end

%
% Test same_term
%

ex16(T) :-
    (same_term(X,X) -> T = true ; T = false).

?- ex16(T).
% Expect: T = true
% Expect: end

ex17(T) :-
    (same_term(X,Y) -> T = true ; T = false).

?- ex17(T).
% Expect: T = false
% Expect: end

ex18(T) :-
    Y = X, (same_term(X,Y) -> T = true ; T = false).

?- ex18(T).
% Expect: T = true
% Expect: end

ex19(T) :-
    Y = foo(1),  X = foo(1), (same_term(X,Y) -> T = true ; T = false).

?- ex19(T).
% Expect: T = false
% Expect: end

ex20(T) :-
    Y = 42,  X = 42, (same_term(X,Y) -> T = true ; T = false).

?- ex20(T).
% Expect: T = true
% Expect: end

%
% Test arg
%

ex21(T) :-
    A = foo(10,20,30), arg(2, A, T).
?- ex21(T).
% Expect: T = 20
% Expect: end









