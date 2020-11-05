% Meta: WAM-only
% Meta: fileio on
% Meta: stdlib
% Meta: debug enabled

:- [sorting].
% Expect: true

?- setup_numbers(8192, 16), program_state.
% Expect: true

?- sort_and_check(8192), program_state.
% Expect: true
