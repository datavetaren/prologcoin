%
% Character properties
%

?- upcase_atom(foo, 'FOO').
% Expect: true

?- upcase_atom(10, '10').
% Expect: true

?- upcase_atom('Foo', 'FOO').
% Expect: true

?- upcase_atom(foo, Q1).
% Expect: Q1 = 'FOO'
% Expect: end

?- upcase_atom(10, Q2).
% Expect: Q2 = '10'
% Expect: end

?- upcase_atom(foo(10), Q3).
% Expect: upcase_atom/2: First argument was not 'atomic', found 'foo(10)'

?- upcase_atom(_, Q4).
% Expect: upcase_atom/2: Arguments are not sufficiently instantiated



