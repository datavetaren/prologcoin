%
% Simple append/3 test
%

append([],Ys,Ys).
append([X|Xs],Ys,[X|Zs]) :-
    append(Xs,Ys,Zs).

?- append([1,2,3], [4,5,6], Q).
% Expect: Q = [1,2,3,4,5,6]
% Expect: end

%
% Simple member/2 test
%

member(X, [X|_]).
member(X, [_|Xs]) :- member(X,Xs).
?- member(X, [1,2,3]).
% Expect: X = 1
% Expect: end



