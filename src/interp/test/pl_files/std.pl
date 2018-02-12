%
% Some standard predicates
%

%
% member/2
%

member(X, Xs) :- member0(Xs, X).
member0([X|_], X).
member0([_|Xs], X) :- member0(Xs, X).

%
% reverse/2
%

reverse(Xs, Ys) :-
    reverse0(Xs, [], Ys).

reverse0([], Ys, Ys).
reverse0([X|Xs], Acc, Ys) :-
    reverse0(Xs, [X|Acc], Ys).

%
% append/3
%

append([], Zs, Zs).
append([X|Xs], Ys, [X|Zs]) :-
    append(Xs, Ys, Zs).

%
% is_list
%
is_list([]).
is_list([_|Xs]) :- is_list(Xs).

%
% length/2
%

length([], 0).
length([_|Xs], N) :- length(Xs, N0), N is N0 + 1.

%
% sort/2 (merge sort)
%

%sort([],[]).
%sort([A],[A]).
%sort([A,B|Xs0], Ys) :-
%    Xs = [A,B|Xs0],
%    split(Xs, As, Bs),
%    sort(As, As1),
%    sort(Bs, Bs1),
%    merge(As1, Bs1, Ys).

split([], [], []).
split([A], [A], []).
split([A,B|Xs], [A|As], [B|Bs]) :-
    split(Xs, As, Bs).

merge([], Ys, Ys).
merge(Xs, [], Xs).
merge([X|Xs], [X|Ys], [X|Zs]) :-
    merge(Xs, Ys, Zs).
merge([X|Xs], [Y|Ys], [X|Zs]) :-
    X @< Y, merge(Xs, [Y|Ys], Zs).
merge([X|Xs], [Y|Ys], [Y|Zs]) :-
    X @> Y, merge([X|Xs], Ys, Zs).

%
% flatten/2 
%
flatten(X, Y) :- \+ var(X), X == [], X = Y.
flatten(A, Ys) :- \+ var(A), A = [X|Xs],
    !,
    flatten(X, F1),
    flatten(Xs, F2),
    append(F1, F2, Ys), !.
flatten(X, [X]).

    

