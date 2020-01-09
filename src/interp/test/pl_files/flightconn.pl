%
% This example shows how to setup a small DB with airports.
% Then it tries to find all flight paths (from and to every city.)
%

:- write('Flight connection example'), nl.

%
% This is the flight connections DB.
%
conn(stockholm, helsinki).
conn(stockholm, copenhagen).
conn(copenhagen, boston).
conn(stockholm, reykjavik).
conn(reykjavik, boston).
conn(stockholm, newyork).
conn(newyork, boston).
conn(reykjavik, newyork).
conn(stockholm, amsterdam).
conn(amsterdam, boston).
conn(amsterdam, newyork).

%
% This is member for difference lists.
%
memb(Xs-Xs, _) :- fail.
memb([X|_]-_-_, X).
memb([_|Xs]-T, X) :- memb(Xs-T,X).

%
% Compute a path from X to Y and store it as a difference list.
%
path(X,Y,Path-PathTail) :-
    conn(X,Z),
    \+ memb(Z, Path),
    PathTail = [Z | NewTail],
    path(Z,Y,Path-NewTail).
path(X,X,_-[]).

%
% Find all flight paths using findall/3.
%
flight_paths(X,Y,Paths) :-
    findall([X|Path], path(X,Y,Path-Path), Paths).

%
% Print a path represented as a standard list of city names.
%
print_path([C1,C2|Cities]) :-
    write(C1), write(' --> '), print_path([C2|Cities]).
print_path([C]) :- write(C), nl.

%
% Main execution. Find all flight paths between Stockholm and Boston
% and then print them one by one.
%
:- flight_paths(stockholm, boston, Paths),
   findall(Path, (member(Path, Paths), print_path(Path)), _).
