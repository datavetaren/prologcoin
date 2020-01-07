:- write('this is startup.pl'), nl.

conn(stockholm, helsinki).
conn(stockholm, copenhagen).
conn(copenhagen, boston).
conn(stockholm, iceland).
conn(iceland, boston).
conn(stockholm, newyork).
conn(newyork, boston).
conn(iceland, newyork).
conn(stockholm, amsterdam).
conn(amsterdam, boston).
conn(amsterdam, newyork).

memb(Xs-Xs, _) :- fail.
memb([X|_]-_-_, X).
memb([_|Xs]-T, X) :- memb(Xs-T,X).

path(X,Y,Path-PathTail) :-
    conn(X,Z),
    \+ memb(Z, Path),
    PathTail = [Z | NewTail],
    path(Z,Y,Path-NewTail).
path(X,X,_-[]).

flight_paths(X,Y,Paths) :-
    findall([X|Path], path(X,Y,Path-Path), Paths).

print_path([C1,C2|Cities]) :-
    write(C1), write(' --> '), print_path([C2|Cities]).
print_path([C]) :- write(C), nl.

:- flight_paths(stockholm, boston, Paths),
   findall(Path, (member(Path, Paths), print_path(Path)), _).
