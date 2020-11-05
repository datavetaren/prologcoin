load_file_global(FileName) :-
    open(FileName, read, S),
    read_code(S),
    close(S),
    compile @ global.

read_code(S) :-
    read(S, Clause),
    read_code1(Clause, S).
read_code1(end_of_file, _) :- !.
read_code1(Clause, S) :-
    assert(Clause) @ global,
    read_code(S).
