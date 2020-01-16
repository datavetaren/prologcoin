%
% Compile word list (text file) into a C++ constant.
%

word_list(TextFileName, CppFileName) :-
    open(TextFileName, read, S),
    read_lines(S, Lines),
    close(S),
    open(CppFileName, write, S2),
    get_word(TextFileName, ConstantName),
    write(S2,'// Generated from compile_word_list.pl using command '), nl(S2),
    write(S2,'// compile_word_list(''bip39_english_word_list.txt'', ''bip39_english_word_list.cpp'')'), nl(S2),
    nl(S2),
    write(S2, 'static const char *'),
    write(S2, ConstantName),
    write(S2, '[] = {'),
    nl(S2),
    write_words(Lines, 0, S2),
    write(S2, '};'),
    close(S2).

write_words([Word|Words], Cnt, S2) :-
    name(Atom,Word),
    write(S2, '"'), write(S2, Atom), write(S2, '"'),
    (Words = [_|_] -> write(S2, ', ') ; true),
    Cnt1 is Cnt + 1,
    ((0 is Cnt1 mod 8) -> nl(S2) ; true),
    write_words(Words, Cnt1, S2).
write_words([], _, _).

get_word(TextFileName, ConstantName) :-
    name(TextFileName, Chars),
    findall(Length-Word, (append(_,Y,Chars),longest_word(Y,Word),length(Word,Length)), Words),
    sort(Words, SortedWords),
    last(SortedWords, _-Name),
    name(AtomName, Name),
    upcase_atom(AtomName, ConstantName).

longest_word([C|Cs], W) :-
    ((char_type(C, alnum) ; C == 95) -> !, W = [C|W1], longest_word(Cs,W1) ; W =  []).
longest_word(_, []).

read_lines(S, [Line|Lines]) :-
    \+ at_end_of_stream(S),
    read_line(S, Line),
    !,
    read_lines(S, Lines).
read_lines(_, []).

read_line(S, [C|Cs]) :-
    peek_code(S, Code),
    \+ Code == 10, \+ Code == 13,
    !,
    get_code(S, C),
    read_line(S, Cs).
read_line(S, []) :-
    skip_nl(S).

skip_nl(S) :-
    peek_code(S, C), (C == 10 ; C == 13), get_code(S, C), !, skip_nl(S).
skip_nl(_).

