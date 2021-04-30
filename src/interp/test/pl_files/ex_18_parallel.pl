%
% Testing @/2, @-/2 and @=/2 operators
%
% Meta: WAM-only
% Meta: stdlib
% Meta: fileio on
% Meta: retain state

:- assert( fib(0,1) ) @ foo.
:- assert( fib(1,1) ) @ foo.
:- assert( (fib(N,R) :- N1 is N - 1, N2 is N - 2, fib(N1,R1), fib(N2,R2), R is R1 + R2 ) ) @ foo.
    
?- fib(10, R) @ foo.
% Expect: R = 89

:- assert( (doit(N, X) :- W is N*100, sleep(W), X is N*2) ) @ foo.
:- assert( (doit(N, X) :- W is N*100, sleep(W), X is N*2) ) @ foo2.

loop(N,N) :- !.
loop(N,M) :-
    N1 is N + 1,
    doit(N1, R1) @= foo,
    freeze(R1, (assert( result(N1,R1) ), write('Computed '), write(N1), nl)),
    N2 is N + 2,
    doit(N2, R2) @= foo2,
    freeze(R2, (assert( result(N2,R2) ), write('Computed '), write(N2), nl)),
    loop(N2,M).

wait_until_done(T) :-
     write('here'), nl,
    (current_predicate(result/2), findall(N-M, result(N,M), L), length(L, T), write(L), nl
     -> true ; sleep(100), write('wait'), nl, write(T), nl, wait_until_done(T)).

print_it :- result(X,Y), write(X), write(' -> '), write(Y), nl, fail.
print_it.
    
?- loop(0,10), wait_until_done(10).
% Expect: true/*

:- print_it.

%
% Check that computation is correct and done
%
check_it(11) :- !.
check_it(N) :- result(N, M), M is N * 2, N1 is N + 1, check_it(N1).
?- check_it(1).
% Expect: true/*
    
