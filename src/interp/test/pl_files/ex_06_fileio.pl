%
% Meta: fileio on
%

openclose_test(X, S, T) :-
    open(X, read, S),
    read(S, T),
    close(S).
?- openclose_test('ex_06_fileio.pl', A, B).
% Expect: A = '$stream'(1), B = openclose_test(X, S, T) :- open(X, read, S), read(S, T), close(S)
% Expect: end


