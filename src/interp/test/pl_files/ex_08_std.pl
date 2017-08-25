%
% Test including files
%

:- [std].

?- member(Q1, [1,2,3]).
% Expect: Q1 = 1
% Expect: Q1 = 2
% Expect: Q1 = 3
% Expect: end
