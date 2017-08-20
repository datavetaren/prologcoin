part([], _, [], []).
part([A|As], P, [A|Xs], Ys) :- A @=< P, part(As, P, Xs, Ys).
part([A|As], P, Xs, [A|Ys]) :- A @> P, part(As, P, Xs, Ys).

merge([], Bs, Bs).
merge(As, [], As).
merge([A|As], [B|Bs], [A|Cs]) :- A @=< B, merge(As, [B|Bs], Cs).
merge([A|As], [B|Bs], [B|Cs]) :- A @> B, merge([A|As], Bs, Cs).

qsort([], []).
qsort([P|Xs], Ys) :-
    part(Xs, P, As, Bs),
    qsort(As, AsSorted),
    qsort(Bs, BsSorted),
    merge(AsSorted, [P|BsSorted], Ys).
    
?- qsort([8,2,10,17,3,7,123,42,4711,11], Q1).
% Expect: Q1 = [2,3,7,8,10,11,17,42,123,4711]
