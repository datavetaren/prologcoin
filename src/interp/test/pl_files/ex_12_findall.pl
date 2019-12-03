%
% Test findall/3
%

member(X, [X|_]).
member(X, [_|Xs]) :- member(X, Xs).

?- findall(X, member(X, [10,20,30,40,50]), Q1).
% Expect: Q1 = [10,20,30,40,50]
% Expect: end

nested(Xs, RR) :-
    findall(foo(R), inner(Xs,R), RR).

inner(Xs,R) :- findall(bar1(X), member(X,Xs), R).
inner(Xs,R) :- findall(bar2(X), member(X,Xs), R).

?- nested([1,2,4,8,16,32],Q2).
% Expect: Q2 = [foo([bar1(1),bar1(2),bar1(4),bar1(8),bar1(16),bar1(32)]), foo([bar2(1),bar2(2),bar2(4),bar2(8),bar2(16),bar2(32)])]
% Expect: end

kv(Xs,Ys) :-
    findall(f(A,B), member(f(A,B),Xs), Ys).

?- kv([f(a,1),f(b,2),f(c,3),f(d,4)],Q3).
% Expect: Q3 = [f(a, 1),f(b, 2),f(c, 3),f(d, 4)]
% Expect: end

% Try findall on arg

?- findall(X-Y, (arg(_, foo(a,b,c), X), arg(_, bar(10,20,30), Y)), R).
% Expect: R = [a-10,a-20,a-30,b-10,b-20,b-30,c-10,c-20,c-30].
% Expect: end

