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

%
% Convert strings into big integers
%
    
?- chars_number("hello world in foobar", Q5).
% Expect: Q5 = 58'7RL9MURV5D8RFkvhpWtcLUkMWTRR3.

?- chars_number(Q6, Q6).
% Expect: chars_number/2: Not both arguments can be unbounded variables.

?- chars_number([100, -1, 1024], Q7).
% Expect: chars_number/2: Element at position 2 is not an integer between 0 and 255; was -1
