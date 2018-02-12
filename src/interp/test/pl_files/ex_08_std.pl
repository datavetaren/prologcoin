%
% Test including files
%

:- [std].

?- member(Q1, [1,2,3]).
% Expect: Q1 = 1
% Expect: Q1 = 2
% Expect: Q1 = 3
% Expect: end

?- sort([1,10,17,5,18,7,32,3,5,9], Q2).
% Expect: Q2 = [1,3,5,7,9,10,17,18,32]
% Expect: end

my_app(X) :-
    append(_,[4,5,6],X), !.

?- my_app(Q3).
% Expect: Q3 = [4,5,6]
% Expect: end

?- flatten([1,2,[3,4],5], Q4).
% Expect: Q4 = [1,2,3,4,5]
% Expect: end


