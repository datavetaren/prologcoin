% Meta: WAM-only

%
% Erase all existing nodes & wallets
%

?- erase_all.
% Expect: true

%
% First create two nodes
%

?- start(node, n1, 9000), start(node, n2, 9001), nolimit @ node(n1), nolimit @ node(n2).
% Expect: true

%
% Setup init
%
?- ignore_pow(true) @ node(n1), ignore_pow(true) @ node(n2).
% Expect: true

%
% Create an interesting chain on node n1
%

advance(0,_) :- !.
advance(N,Node) :-
      advance @ node(Node), N1 is N - 1, advance(N1,Node).

% Create chain - add 10 elements.
?- advance(10,n1), current(Id) @ node(n1), assert(tmp:tip(Id)).
% Expect: true/*

% Go back to previous state 4
?- (chain(4, [X]), switch(X)) @ node(n1).
% Expect: true/*

% Change state
?- (assert(foo(42)) @ global, advance) @ node(n1).
% Expect: true/*

% Advance
?- advance(2, n1), current(Id) @ node(n1), assert(tmp:here(Id)), advance(3, n1).
% Expect: true/*

% Go back to state Id
?- tmp:here(Id), switch(Id) @ node(n1).
% Expect: true/*

% Change state and advance
?- (assert(foo(4711)) @ global) @ node(n1), advance(3, n1).
% Expect: true/*

% Let's go to original tip and make it the longest
?- tmp:tip(Id), advance(10,n1), current(NewId) @ node(n1), retract(tmp:tip(Id)), assert(tmp:tip(NewId)).
% Expect: true/*

%
% ---- Let's program node 'n2' to use a small step size (so sync isn't so fast)
%

?- sync_init(:- (assert(sync:step(2)))) @ node(n2).
% Expect: true/*

%
% Let's start syncing
%
?- sync @ node(n2).
% Expect: true/*

?- add_address('127.0.0.1', 9000) @ node(n2).
% Expect: true/*

%
% At this point we'll wait until sync is done
%

wait_sync_complete(20, _) :- !, write('Waiting for too long. Giving up.'), nl, fail.
wait_sync_complete(N, Node) :-
    (sync_mode(Mode), syncing_meta(Block)) @ node(Node),
    write('Waiting iteration='), write(N), write(' mode='), write(Mode), write(' progress='), write(Block), write(' connections='), connections(Cs) @ node(Node), write(Cs), nl,
    sleep(5000),
    (sync_complete(true) @ node(Node) -> write('Sync is done'), nl
       ; N1 is N + 1, wait_sync_complete(N1,Node)).
    
?- wait_sync_complete(0, n2).
% Expect: true/*


% Check again
?- sync_complete(true) @ node(n2).
% Expect: true/*

%
% Start a 3rd node
%

?- start(node, n3, 9002), (nolimit, ignore_pow(true)) @ node(n3).
% Expect: true

?- sync_init(:- (assert(sync:step(4)))) @ node(n3).
% Expect: true

?- (add_address('127.0.0.1', 9000), add_address('127.0.0.1', 9001)) @ node(n3).
% Expect: true

?- sync @ node(n3).
% Expect: true

?- wait_sync_complete(0, n3).
% Expect: true

%
% Start a 4th node
%

%
% Should download from 3 other nodes in parallel.
%

?- start(node, n4, 9003), (nolimit, ignore_pow(true)) @ node(n4).
% Expect: true

?- sync_init(:- (assert(sync:step(4)))) @ node(n4).
% Expect: true

?- (add_address('127.0.0.1', 9000), add_address('127.0.0.1', 9001), add_address('127.0.0.1', 9002)) @ node(n4).
% Expect: true

?- sync @ node(n4).
% Expect: true

?- wait_sync_complete(0, n4).
% Expect: true


