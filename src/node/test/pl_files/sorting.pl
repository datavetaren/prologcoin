rand(Xin,Xout) :-
    X0 is Xin*1103515245 + 12345,
    (X0 < 0 -> X1 = -X0 ; X1 = X0),
    Xout = X1.
    
generate_numbers(N, Xs) :-
    generate_numbers1(N, 123, Xs).

generate_numbers1(0, _, []) :- !.

generate_numbers1(N, R, [X|Xs]) :-
    rand(R, R1), !, X is R1 mod 179424691,
    N1 is N - 1,
    generate_numbers1(N1, X, Xs).

first_elements(0, Xs, [], Xs) :- !.
first_elements(N, [X|Xs], [X|Ys], Zs) :-
    N1 is N - 1,
    first_elements(N1, Xs, Ys, Zs).

split_numbers([], _, []) :- !.
split_numbers(Xs, N, [Ys|Ss]) :-
    first_elements(N, Xs, Ys, Zs),
    split_numbers(Zs, N, Ss).

merge([], Ys, Ys) :- !.
merge(Xs, [], Xs) :- !.
merge([X|Xs], [Y|Ys], [Z|Zs]) :-
    X @=< Y, !, Z = X, merge(Xs, [Y|Ys], Zs).
merge(Xs, [Y|Ys], [Y|Zs]) :-
    merge(Xs, Ys, Zs).

merge1(Xs,Ys,Zs) :-
    merge(Xs,Ys,Zs).

delayed_merge(Trigger, Xs, Ys, Zs) :-
    freeze(Trigger, merge1(Xs, Ys, Zs)).

setup_merge([], [], []) :- !.
setup_merge([R], [R], []) :- !.
setup_merge([A,B|Cs], [R|Rs], [T|Ts]) :-
    delayed_merge(T,A,B,R),
    setup_merge(Cs,Rs,Ts).

setup_mergers([R], [R], []) :- !.
setup_mergers(Xs, Rs, Ts) :-
    setup_merge(Xs, As, Ts1),
    setup_mergers(As, Rs, Ts2),
    append(Ts1,Ts2,Ts).

setup_sorting(N) :-
    length(L, N), 
    setup_mergers(L, [_], _).

setup_numbers(N, M) :-
    T is N*M,
    write('generate_numbers'), nl,
    generate_numbers(T, Xs),
    write('split_numbers'), nl,    
    split_numbers(Xs, M, Ys),
    write('sort chunks'), nl,
    findall(Y, (member(X,Ys), sort(X,Y)), Cs),
    write('store numbers'), nl,
    store_numbers(Cs, 0).

store_numbers([], _).
store_numbers([C|Cs], N) :-
    assert(tmp:numbers(N, C)),
    N1 is N + 1,
    store_numbers(Cs, N1).

get_result(N, R) :-
    write('setup_sorting'), nl,
    setup_sorting(N),
    write('feed_sorters'), nl,
    feed_sorters(0, N),
    write('wait_sorters'), nl,
    wait_sorters(0, [], R).

get_numbers(R) :-
    get_numbers1(N, [], R).

get_numbers1(N, Acc, R) :-
    tmp:numbers(N, Ns),
    !, append(Ns, Acc, Acc1),
    N1 is N + 1,
    get_numbers1(N1, Acc1, R).
get_numbers1(_, Acc, Acc).

check_result(R) :-
    get_numbers(Ns),
    sort(Ns, Sorted),
    Sorted = R.
