% This test was created to lock down trail size to be 0 as we
% had a trail leak problem.
%
% This also tests that frozen closures work properly.

% Meta: WAM-only
% Meta: fileio on
% Meta: stdlib
% Meta: debug enabled

:- [sorting].
% Expect: true

?- setup_numbers(256, 16).
% Expect: true

check_program_state :-
    program_state,
    program_state(S),
    member(heap_size(HeapSize), S), HeapSize < 500000,
    member(program_stack_size(ProgStackSize), S), ProgStackSize < 500,
    member(stack_size(0), S),
    member(trail_size(0), S),
    member(temp_size(0), S),
    member(temp_trail_size(0), S),
    member(num_frozen_closures(0), S),
    member(num_predicates(34), S),
    member(num_clauses(307), S).

?- sort_and_check(256), check_program_state.
% Expect: true
