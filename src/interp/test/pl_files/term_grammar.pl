property('namespace', ['prologcoin', 'common']).
property('classname', 'term_parser_gen').
property('filename', '../../../../bin/test/interp/term_parser_gen.hpp').
% property(prefer, reduce).

start :- subterm_1200, full_stop.

subterm_1200 :- subterm_n.
subterm_999 :- subterm_n.

subterm_n :- term_n.

term_n :- op_fx, subterm_n, { reduce_cond(check_op_fx) }.
term_n :- op_fy, subterm_n, { reduce_cond(check_op_fy) }.
term_n :- subterm_n, op_xfx, subterm_n, { reduce_cond(check_op_xfx) }.
term_n :- subterm_n, op_xfy, subterm_n, { reduce_cond(check_op_xfy) }.
term_n :- subterm_n, op_yfx, subterm_n, { reduce_cond(check_op_yfx) }.
term_n :- subterm_n, op_xf, { reduce_cond(check_op_xf) }.
term_n :- subterm_n, op_yf, { reduce_cond(check_op_yf) }.
term_n :- term_0.

term_0 :- functor_lparen, arguments, rparen.
term_0 :- lparen, subterm_1200, rparen.
term_0 :- lbrace, subterm_1200, rbrace.
term_0 :- list.
term_0 :- string.
term_0 :- constant.
term_0 :- variable.

arguments :- subterm_999.
arguments :- subterm_999, comma, arguments.

list :- lbracket, rbracket.
list :- lbracket, listexpr, rbracket.

listexpr :- subterm_999.
listexpr :- subterm_999, comma, listexpr.
listexpr :- subterm_999, vbar, subterm_999.

constant :- atom.
constant :- number.

number :- unsigned_number.
number :- inf.
number :- nan.

unsigned_number :- natural_number.
unsigned_number :- unsigned_float.

atom :- name.
atom :- empty_list.
atom :- empty_brace.

