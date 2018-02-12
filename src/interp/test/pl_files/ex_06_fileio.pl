%
% Meta: fileio on
% Meta: WAM-only
%

openclose_test(X, S, T) :-
    open(X, read, S),
    read(S, T),
    close(S).
?- openclose_test('ex_06_fileio.pl', A, B).
% Expect: A = '$stream'(3), B = openclose_test(X, S, T) :- open(X, read, S), read(S, T), close(S)
% Expect: end

%
% Check end_of_file
%

readterms(File, C) :-
    open(File, read, S),
    readterms_read(S, Ts),
    length(Ts, C), !,
    close(S).

readterms_read(S, Ts) :-
    read(S, T),
    (T = end_of_file -> Ts = []
   ; Ts = [T|Ts0], readterms_read(S, Ts0)
    ).
length([], 0).
length([X|Xs], N) :- length(Xs, N0), N is N0 + 1.

% Meta: WAM-only
?- readterms('sample_file.pl', C).
% Expect: C = 10
% Expect: end

%
% Check tell/told
%

%tell_told :-
%    tell('foo.txt'),
%    write('hello'), write(' '), write('world'), nl,
%    tell('bar.txt'),
%    write('this is'), write(' another '), write('line'), nl,
%    told,
%    write('continuing on the foo.txt file'), nl,
%    told.
%?- tell_told.



