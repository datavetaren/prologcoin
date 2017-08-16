%
% Simple append/3 test
%

append([],Ys,Ys).
append([X|Xs],Ys,[X|Zs]) :-
    append(Xs,Ys,Zs).

?- append([1,2,3], [4,5,6], Q).
% Expect: Q = [1,2,3,4,5,6]
