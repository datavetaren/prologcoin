%
% Meta: fileio on
%

openclose_test(X, S) :-
    open(X, read, S),
    close(S).
?- openclose_test('ex_06_fileio.pl', S).
% Expect: Undefined predicate close/1
