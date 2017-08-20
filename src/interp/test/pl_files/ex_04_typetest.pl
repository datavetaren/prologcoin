%
% Testing var/1
%

?- var(_).
% Expect: true

?- var(10).
% Expect: fail

?- var(foo).
% Expect: fail

?- var(foo(42)).
% Expect: fail

%
% Testing nonvar/1
%

?- nonvar(_).
% Expect: fail

?- nonvar(10).
% Expect: true

?- nonvar(foo).
% Expect: true

?- nonvar(foo(2)).
% Expect: true

%
% Testing integer/1
%

?- integer(_).
% Expect: fail

?- integer(10).
% Expect: true

?- integer(foo).
% Expect: fail

?- integer(foo(42)).
% Expect: fail

%
% Testing number/1
%

?- number(_).
% Expect: fail

?- number(10).
% Expect: true

?- number(foo).
% Expect: fail

?- number(foo(42)).
% Expect: fail

%
% Testing atom/1
%

?- atom(foo).
% Expect: true

?- atom(foo(10)).
% Expect: fail

?- atom([]).
% Expect: true

?- atom(123).
% Expect: fail

?- atom(_).
% Expect: fail

%
% Testing atomic/1
%

?- atomic(foo).
% Expect: true

?- atomic(foo(10)).
% Expect: fail

?- atomic([]).
% Expect: true

?- atomic(123).
% Expect: fail

?- atomic(_).
% Expect: fail

%
% Testing compound/1
%

?- compound(foo).
% Expect: fail

?- compound(foo(10)).
% Expect: true

?- compound([]).
% Expect: fail

?- compound(123).
% Expect: fail

?- compound(_).
% Expect: fail

?- compound([A|A]).
% Expect: A = G_0

%
% Testing callable/1
%

?- callable(_).
% Expect: fail

?- callable(foo).
% Expect: true

?- callable(foo(42)).
% Expect: true

?- callable(123).
% Expect: fail

%
% Testing ground/1
%

?- ground(123).
% Expect: true

?- ground(_).
% Expect: fail

?- ground(foo(bar(42, baz), [1,2,3])).
% Expect: true

?- ground(foo(bar(42, baz), [1,2,3|_])).
% Expect: fail





