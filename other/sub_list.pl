sub_list(List, A, B, C, W) :-
    nonvar(List),
    length(List, L),
    get_val(L, 0, A),
    list_slice(List, A, B, W),
    C is L - A - B.

list_slice(List, 0, Length, Slice) :-
    list_prefix(List, Length, Slice).
list_slice([_H|T], N, Length, Slice) :-
    N1 is N - 1,
    list_slice(T, N1, Length, Slice).

list_prefix(_List, 0, []).
list_prefix([H|T], N, [H|PT]) :-
    nonvar(N),
    N > 0,
    N1 is N - 1,
    list_prefix(T, N1, PT).
list_prefix([H|T], N, [H|PT]) :-
    var(N),
    list_prefix(T, N1, PT),
    N is N1 + 1.

get_val(L,N,N) :- N =< L.
get_val(L,M, N) :-
    M1 is M + 1,
    M1 =< L,
    get_val(L, M1, N).
