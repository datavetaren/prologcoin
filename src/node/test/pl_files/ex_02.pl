% Meta: WAM-only
% Meta: fileio on
% Meta: debug enabled

?- drop_global.
% Expect: true

?- nolimit.
% Expect: true

?- consult('load_file_global').
% Expect: true

?- load_file_global('sorting.pl').
% Expect: true

?- advance.
% Expect: true

?- consult(sorting).
% Expect: true

?- setup_numbers(1024, 16).
% Expect: true

?- setup_sorting(1024) @ global.
% Expect: true

?- advance.
% Expect: true

feed_sorter(N, N) :- !.
feed_sorter(N0, N1) :-
    frozenk(0, 1, [H]) @ global,
    tmp:numbers(N0, Numbers1),
    N0B is N0 + 1,
    tmp:numbers(N0B, Numbers2),
    defrost(H, _Closure, [trigger, Numbers1, Numbers2, _Result]) @ global,
    ! @ global,
    NN is N0B + 1,
    feed_sorter(NN, N1).

feed_sorters(N, N) :- !.
feed_sorters(N, M) :-
    N1 is N + 256,
    feed_sorter(N, N1),
    feed_sorters(N1, M).

?- feed_sorters(0, 512).
% Expect: true

?- advance.
% Expect: true

?- feed_sorters(512, 1024).
% Expect: true

wait_sorters(0, Result, Result) :- !.
wait_sorters(N, _, Result) :-
    frozenk(0, 1, [H]) @ global,
    !,
    defrost(H, Closure, [trigger, Result0]) @ global,
    N1 is N - 1,
    wait_sorters(N1, Result0, Result).

wait_some :- wait_sorters(256, [], Result).

?- program_state @ global.
% Expect: true

?- wait_some.
% Expect: true

?- advance.
% Expect: true

wait_some2 :- wait_sorters(254, [], Result).

?- wait_some2.
% Expect: true

?- advance.
% Expect: true

?- chain.
% Expect: true

final_check :- wait_sorters(1, [], Result), check_result(Result).

?- final_check.
% Expect: true

?- advance.
% Expect: true

program_state_check :-
    program_state(X) @ global,
    member(trail_size(0), X),
    member(stack_size(0), X),
    member(program_stack_size(0), X),
    member(num_frozen_closures(0), X),
    member(temp_trail_size(0), X),
    member(temp_size(0), X),
    member(num_predicates(P), X), P < 32,
    member(num_clauses(C), X), C < 50.
    
?- program_state @ global, program_state_check.
% Expect: true

% Let's roll back to last previous state.
?- chain(5, [Id]), switch(Id), chain, program_state @ global.
% Expect: true

% Let's run the last command again from this state

?- final_check.
% Expect: true
?- program_state_check.
% Expect: true

% Let's go further back in time and retry
?- chain(4, [Id]), switch(Id), chain.
% Expect: true

?- wait_some2, final_check.
% Expect: true
?- program_state_check.
% Expect: true

% And even further back
?- chain(3, [Id]), switch(Id), chain.
% Expect: true

?- feed_sorters(512, 1024), wait_some, wait_some2, final_check.
% Expect: true
?- program_state_check.
% Expect: true


