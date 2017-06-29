start :- subterm(10000), full_stop.

subterm(N) :- term('<'(N)).

term(N) :- op(N,fx), subterm('<'(N)).
term(N) :- op(N,fy), subterm(N).
term(N) :- subterm('<'(N)), op(N,xfx), subterm('<'(N)).
term(N) :- subterm('<'(N)), op(N, xfy), subterm(N).
term(N) :- subterm(N), op(N, yfx), subterm('<'(N)).
term(N) :- subterm('<'(N)), op(N, xf), subterm('<'(N)).
term(N) :- subterm(N), op(N, yf).

term(1000) :- subterm(999), comma, subterm(1000).

term(0) :- functor, lparen, arguments, rparen.
term(0) :- lparen, subterm(1200), rparen.
term(0) :- lbrace, subterm(1200), rbrace.
term(0) :- list.
term(0) :- string.
term(0) :- constant.
term(0) :- variable.

arguments :- subterm(999).
arguments :- subterm(999), comma, arguments.

list :- lbracket, rbracket.
list :- lbracket, listexpr, rbracket.

listexpr :- subterm(999).
listexpr :- subterm(999), comma, listexpr.
listexpr :- subterm(999), vbar, subterm(999).

constant :- atom.
constant :- number.

number :- unsigned_number.
number :- sign, unsigned_number.
number :- sign, inf.
number :- sign, nan.

unsigned_number :- natural_number.
unsigned_number :- unsigned_float.

atom :- name.

functor :- name.

sign :- plus.
sign :- minus.

