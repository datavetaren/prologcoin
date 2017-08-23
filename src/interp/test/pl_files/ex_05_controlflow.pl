%
% Cut operator
%

mycut(X) :- X = 1, !.
mycut(X) :- X = 2.

?- mycut(Q1).
% Expect: Q1 = 1
% Expect: end

mycut2(X) :- X = 1.
mycut2(X) :- X = 2, !.
mycut2(X) :- X = 3.

?- mycut2(Q2).
% Expect: Q2 = 1
% Expect: Q2 = 2
% Expect: end

mycut3(X) :- X = 1.
mycut3(X) :- mycut3_sub(X).
mycut3(X) :- X = 4.

mycut3_sub(X) :- X = 2, !.
mycut3_sub(X) :- X = 3.

?- mycut3(Q3).
% Expect: Q3 = 1
% Expect: Q3 = 2
% Expect: Q3 = 4
% Expect: end

%
% Disjunction operator (;)
%

mydisj1(X) :- (X = 1 ; X = 2).
?- mydisj1(Q4).
% Expect: Q4 = 1
% Expect: Q4 = 2
% Expect: end

mydisj2(X) :- (X = 1, myfail2(X) ; X = 2).
myfail2(X) :- X = 123.
?- mydisj2(Q5).
% Expect: Q5 = 2
% Expect: end

mydisj3(X) :- (X = 1 ; X = 2; X = 3).
?- mydisj3(Q6).
% Expect: Q6 = 1
% Expect: Q6 = 2
% Expect: Q6 = 3
% Expect: end

mydisj4(X) :- ((X = 1 ; X = 2); (X = 3; X = 4)).
?- mydisj4(Q7).
% Expect: Q7 = 1
% Expect: Q7 = 2
% Expect: Q7 = 3
% Expect: Q7 = 4
% Expect: end

mydisj5(X) :- (mydisj5a(X) ; mydisj5b(X)).
mydisj5a(X) :- X = 1 ; X = 2.
mydisj5b(X) :- X = 3 ; X = 4.
?- mydisj5(Q8).
% Expect: Q8 = 1
% Expect: Q8 = 2
% Expect: Q8 = 3
% Expect: Q8 = 4
% Expect: end

%
% Combination of ; and !
%

mydcut(A,X) :- myit(A), (myit2(X) , ! ; myit2(X)).
myit(1).
myit(2).
myit2(1).
myit2(2).
?- mydcut(Q9,Q10).
% Expect: Q9 = 1, Q10 = 1
% Expect: end

%
% If-then
%

myifthen(X) :- myit(X), (true -> true).
?- myifthen(Q11).
% Expect: Q11 = 1
% Expect: Q11 = 2

myifthen2(X) :- myit(X), (myit2(Y) -> Y = 2).
?- myifthen2(Q12).
% Expect: fail

%
% If-then-else
%

myifthenelse(X,Z) :- myit(X), (myit2(Y) -> Z = 1 ; Z = 2).
?- myifthenelse(Q13,Q14).
% Expect: Q13 = 1, Q14 = 1
% Expect: Q13 = 2, Q14 = 1
% Expect: end

myifthenelse2(X,Z) :- myit(X), (myit2(Y) -> Y = 2 ; Z = 2).
?- myifthenelse2(Q15,Q16).
% Expect: fail

myifthenelse3(A,B) :- myit(A), (myit2(Y), Y = 2 -> B = 42 ; B = 4711).
?- myifthenelse3(Q17,Q18).
% Expect: Q17 = 1, Q18 = 42
% Expect: Q17 = 2, Q18 = 42
% Expect: end

myifthenelse4(A,B) :- myit(A), (myit2(Y), Y = 3 -> B = 42 ; B = 4711).
?- myifthenelse4(Q19,Q20).
% Expect: Q19 = 1, Q20 = 4711
% Expect: Q19 = 2, Q20 = 4711
% Expect: end
