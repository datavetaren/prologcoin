% Meta: WAM-only

?- create("foobar", X).
% Expect: true/*

?- newkey(A,B).
% Expect: true/*

?- newkey(A,B).
% Expect: true/*

?- newkey(A,B).
% Expect: true/*

?- newkey(A,B).
% Expect: true/*

?- newkey(A,B).
% Expect: true/*

?- commit(reward(58'1MfD7vFBpdzwC1rJPyAAmnmKkmgUas59My)) @ node.
% Expect: true/*

?- sync.
% Expect: true/*

?- balance(X).
% Expect: X = 42445243724242

?- commit(reward(58'1MfD7vFBpdzwC1rJPyAAmnmKkmgUas59My)) @ node.
% Expect: true/*

?- sync.
% Expect: Erroneous argument [2]/1530:CON at offset 272 due to 967:STR at offset 232
