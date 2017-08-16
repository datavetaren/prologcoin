%
% append/3
%

append([],Ys,Ys).
append([X|Xs],Ys,[X|Zs]) :-
    append(Xs,Ys,Zs).

?- append([1,2,3], [4,5,6], Q1).
% Expect: Q1 = [1,2,3,4,5,6]
% Expect: end

%
% member/2
%

member(X, [X|_]).
member(X, [_|Xs]) :- member(X,Xs).
?- member(Q2, [1,2,3]).
% Expect: Q2 = 1
% Expect: Q2 = 2
% Expect: Q2 = 3
% Expect: end

%
% split/3
%

split([],[],[]).
split([A],[A],[]).
split([A,B|Xs],[A|As],[B|Bs]) :-
    split(Xs,As,Bs).

?- split([1,2,3,4,5],Q3,Q4).
% Expect: Q3 = [1,3,5], Q4 = [2,4]
?- split([1,2,3,4],Q5,Q6).
% Expect: Q5 = [1,3], Q6 = [2,4]
?- split([],Q7,Q8).
% Expect: Q7 = [], Q8 = []

%
% mergesort/2 
%
