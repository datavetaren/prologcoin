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

%
% sformat
%
% Meta: xlocale

?- sformat(Q1, '~a', 'atom with space').
% Expect: Q1 = "atom with space"
?- sformat(Q2, '~c', 65).
% Expect: Q2 = [65]
?- sformat(Q3, '~10c', 65).
% Expect: Q3 = "AAAAAAAAAA"
?- sformat(Q4, '~5d', 12345).
% Expect: Q4 = "0.12345"
?- sformat(Q5, '~5d', 123456).
% Expect: Q5 = "1.23456"
?- sformat(Q6, '~1:d', 123456789).
% Expect: Q6 = "12 345 678,9"
?- sformat(Q7, '~1D', 123456789).
% Expect: Q7 = "12,345,678.9"
?- sformat(Q8, '~e', 12345).
% Expect: Q8 = "1.234500e+04"
?- sformat(Q8, '~3e', 12345).
% Expect: Q8 = "1.234e+04"
?- sformat(Q9, '~3E', 12345).
% Expect: Q9 = "1.234E+04"
?- sformat(Q10, '~3f', 12345).
% Expect: Q10 = "12345.000"
?- sformat(Q11, '~3g', 12345).
% Expect: Q11 = "12345.000"
?- sformat(Q12, '~3g', 123456).
% Expect: Q12 = "1.235e+05"
?- sformat(Q13, '~3G', 123456).
% Expect: Q13 = "1.235E+05"
?- sformat(Q14, 'foo~ibar~d', [10, 42]).
% Expect: Q14 = "foobar42"
?- sformat(Q15, '~I', 123456789).
% Expect: Q15 = "123_456_789"
?- sformat(Q16, '~k', 10+20*30).
% Expect: Q16 = "+(10, *(20, 30))"
?- sformat(Q17, '~n', []).
% Expect: Q17 = [10]
?- sformat(Q18, '~2n', []).
% Expect: Q18 = [10,10]
?- sformat(Q19, 'foo~n~Nbar~Nbaz', []).
% Expect: Q19 = "foo\nbar\nbaz"
?- sformat(Q20, 'foo~*nbarbaz', [3]).
% Expect: Q20 = "foo\n\n\nbarbaz"
?- sformat(Q21, 'foox~qybar', 'abc def'(10)).
% Expect: Q21 = "foox\'abc def\'(10)ybar"
?- sformat(Q22, 'testing ~10r', 255).
% Expect: Q22 = "testing 255"
?- sformat(Q23, 'testing ~16r', 255).
% Expect: Q23 = "testing ff"
?- sformat(Q24, 'testing ~16R', 255).
% Expect: Q24 = "testing FF"
?- sformat(Q25, 'testing ~16:R', 65535).
% Expect: Q25 = "testing F FFF"
?- sformat(Q26, 'testing ~s', ["some string"]).
% Expect: Q26 = "testing some string"
?- sformat(Q27, 'a~tb~tc~20|', []).
% Expect: Q27 = "a        b         c"
?- sformat(Q28, 'a1~tb~tc~20|', []).
% Expect: Q28 = "a1        b        c"
?- sformat(Q29, 'a123456789~tb~tc~20|', []).
% Expect: Q29 = "a123456789    b    c"
?- sformat(Q30, 'a123456789~tb~tc123~20|', []).
% Expect: Q30 = "a123456789  b   c123"
?- sformat(Q31, 'a123456789~tbabc~tc123~20|', []).
% Expect: Q31 = "a123456789 babc c123"
?- sformat(Q32, 'a123456789~tbabcd~tc123~20|', []).
% Expect: Q32 = "a123456789babcd c123"
?- sformat(Q33, 'a123456789~tbabcd~tc123x~20|', []).
% Expect: Q33 = "a123456789babcdc123x"
?- sformat(Q34, 'a123456789~tbabcde~tc123x~20|', []).
% Expect: Q34 = "a123456789babcdec123x"
?- sformat(Q35, 'Runtime: ~`.t ~2f~34|  Inferences: ~`.t ~D~72|~n', [10, 20]).
% Expect: Q35 = "Runtime: ................... 10.00  Inferences: ..................... 20\n"
?- sformat(Q36, 'a~`1t~a~30|b~`2tx~`3tr~10+',[xyz]). 
% Expect: Q36 = "a11111111111111111111111111xyzb222x3333r"
?- sformat(Q37, 'a~`1t~a~30|b~`2t~`3tr~10+',[xyz]). 
% Expect: Q37 = "a11111111111111111111111111xyzb22223333r"
% (SWI-Prolog is wrong on this one)

