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

?- commit(reward(58'11uia4BfsJzo1QnUnvi2URFmDgcbEbKSYS)) @ node.
% Expect: true/*

?- sync.
% Expect: true/*

?- balance(X).
% Expect: X = 21000000000

?- commit(reward(58'11uia4BfsJzo1QnUnvi2URFmDgcbEbKSYS)) @ node.
% Expect: true/*

?- sync.
% Expect: true/*

?- balance(X).
% Expect: X = 42000000000    
    
?- save.
% Expect: true/*


