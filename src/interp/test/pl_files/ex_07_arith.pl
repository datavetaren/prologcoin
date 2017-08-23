%
% is/2
%

arith1(X) :-
    X is (1 + 2*3 - 4)*5.
?- arith1(Q1).
% Expect: Q1 = 15
% Expect: end

%
% Length of list
%

length([], 0).
length([X|Xs], N) :-
    length(Xs, N1),
    N is N1 + 1.
?- length([1,2,3,4,5],Q2).
% Expect: Q2 = 5
% Expect: end
