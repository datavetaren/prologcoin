% ----------------------------------------------------
%  Standard term order @< (less than)
% ----------------------------------------------------

lt(A, B, C) :- A @< B, C = 1.

?- lt(10, 20, Q1).
% Expect: Q1 = 1

?- lt(-10, 20, Q1B).
% Expect: Q1B = 1

?- lt(10, foo, Q2).
% Expect: Q2 = 1

?- lt(_, 10, Q3).
% Expect: Q3 = 1

?- lt(_, foo, Q4).
% Expect: Q4 = 1

?- lt(X, Y, Q5).
% Expect: Q5 = 1

?- lt(foo, foo(2), Q6).
% Expect: Q6 = 1

?- lt(foo(10,bar), foo(10,baz), Q7).
% Expect: Q7 = 1

% ----------------------------------------------------
%  Standard term order @> (greater than)
% ----------------------------------------------------

gt(A, B, C) :- A @> B, C = 1.

?- gt(20, 10, Q8).
% Expect: Q8 = 1

?- gt(20, -10, Q8B).
% Expect: Q8B = 1    

?- gt(foo, 10, Q9).
% Expect: Q9 = 1

?- gt(10, _, Q10).
% Expect: Q10 = 1

?- gt(foo, _, Q11).
% Expect: Q11 = 1

?- gt(foo(2), foo, Q12).
% Expect: Q12 = 1

?- gt(foo(10,baz), foo(10,bar), Q13).
% Expect: Q13 = 1

% ----------------------------------------------------
%  Standard term order @=< (less than or equal)
% ----------------------------------------------------

le(A, B, C) :- A @=< B, C = 1.

?- le(10, 20, Q14).
% Expect: Q14 = 1

?- le(-10, 20, Q14B).
% Expect: Q14B = 1

?- le(foo, foo, Q15).
% Expect: Q15 = 1

?- le(_, 10, Q16).
% Expect: Q16 = 1

?- le(_, foo, Q17).
% Expect: Q17 = 1

?- le(X, Y, Q18).
% Expect: Q18 = 1

?- le(foo, foo(2), Q19).
% Expect: Q19 = 1

?- le(foo(10,bar), foo(10,baz), Q20).
% Expect: Q20 = 1

?- le(foo(10,bar), foo(10,bar), Q21).
% Expect: Q21 = 1

% ----------------------------------------------------
%  Standard term order @>= (greater than or equal)
% ----------------------------------------------------

ge(A, B, C) :- A @>= B, C = 1.

?- ge(20, 10, Q22).
% Expect: Q22 = 1

?- ge(10, -20, Q22B).
% Expect: Q22B = 1    

?- ge(foo, 10, Q23).
% Expect: Q23 = 1

?- le(foo, foo, Q24).
% Expect: Q24 = 1

?- ge(10, _, Q25).
% Expect: Q25 = 1

?- ge(foo, _, Q26).
% Expect: Q26 = 1

?- ge(foo(2), foo, Q27).
% Expect: Q27 = 1

?- ge(foo(10,baz), foo(10,bar), Q28).
% Expect: Q28 = 1

?- le(foo(10,bar), foo(10,bar), Q29).
% Expect: Q29 = 1

% ----------------------------------------------------
%  Standard term order == (equals)
% ----------------------------------------------------

eq(A, B, C) :- A == B, C = 1.

?- eq(foo(2,3,baz(4711)),foo(2,3,baz(4711)),Q30).
% Expect: Q30 = 1

?- eq(A, A, Q31).
% Expect: Q31 = 1

?- eq(-1, -1, Q31B).
% Expect: Q31B = 1    

% ----------------------------------------------------
%  Standard term order \== (not equals)
% ----------------------------------------------------

ne(A, B, C) :- A \== B, C = 1.

?- ne(foo(2,3,baz(4711)),foo(2,3,baz(4712)),Q32).
% Expect: Q32 = 1

?- ne(A, B, Q33).
% Expect: Q33 = 1

?- ne(10, foo, Q34).
% Expect: Q34 = 1

?- ne(10, 20, Q35).
% Expect: Q35 = 1

% ----------------------------------------------------
%  Standard term order compare/3
% ----------------------------------------------------

cmp(A, B, C) :- compare(A, B, C).

?- cmp(Q36, 10, 20).
% Expect: Q36 = <

?- cmp(Q37, 20, 10).
% Expect: Q37 = >

?- cmp(Q38, foo, foo).
% Expect: Q38 = =
