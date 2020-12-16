% Meta: WAM-only

%
% Erase all existing nodes & wallets
%

?- erase_all.
% Expect: true

%
% First ew create some nodes
%

?- start(node, n1, 9000), start(node, n2, 9001), nolimit @ node(n1), nolimit @ node(n2).
% Expect: true

%
% Then create wallets
%
?- start(wallet, w1), start(wallet, w2).
% Expect: true

%
% Connect wallets to nodes
%
?- connect(wallet(w1), node(n1)), connect(wallet(w2), node(n2)).
% Expect: true

%
% Wallets
%
% X = [skate,glory,blur,park,famous,cattle,bread,drift,arch,track,absurd,hospital,estate,sock,laundry,regular,rely,neutral,quiz,coil,exchange,slogan,human,choose]
% X = [define,menu,year,glad,accident,tail,scissors,ostrich,only,place,coast,always,guilt,lizard,once,evoke,iron,nothing,explain,burger,elbow,spy,dog,comic]
    
?- create("foobar1", [skate,glory,blur,park,famous,cattle,bread,drift,arch,track,absurd,hospital,estate,sock,laundry,regular,rely,neutral,quiz,coil,exchange,slogan,human,choose]) @ wallet(w1).
% Expect: true

?- create("foobar2", [define,menu,year,glad,accident,tail,scissors,ostrich,only,place,coast,always,guilt,lizard,once,evoke,iron,nothing,explain,burger,elbow,spy,dog,comic]) @ wallet(w2).
% Expect: true

%
% Get lots of monieeee!
%

?- newkey(A,B) @ wallet(w1), assert(tmp:monieee(B)).
% Expect: true/*

?- tmp:monieee(Addr), commit(reward(Addr)) @ node(n1).
% Expect: true/*

?- sync @ wallet(w1).
% Expect: true/*

?- balance(X) @ wallet(w1).
% Expect: X = 42445243724242. 

?- save @ wallet(w1).
% Expect: true/*

% Create some keys in wallet 2 och store the addresses at node.
% (Note that entire interpreter is wiped when we switch wallets, so that's why we
% store it at the node (@ node).) It would be convenient if one could create scratch
% interpreters to store stuff in... will think about that.
%
tmp:create_addrs(N,N) :- !.
tmp:create_addrs(N, End) :-
    newkey(PubKey, PubAddr) @ wallet(w2), assert(tmp:wallet2(N, PubAddr)),
    N1 is N + 1,
    tmp:create_addrs(N1, End).

?- tmp:create_addrs(0, 10).
% Expect: true

%
% Create 1 transaction with 1000 outputs to wallet 2 (using the 10 addresses.)
%

tmp:amounts(N, N, []) :- !.
tmp:amounts(Count, EndCount, [Value|Amounts]) :-
    Value is 10000 + Count,
    Count1 is Count + 1,
    tmp:amounts(Count1, EndCount, Amounts).

save_commit(Commit) :-
    (current_predicate(tmp:save_commit_count/1) ->
	 tmp:save_commit_count(N), retractall(tmp:save_commit_count(_))
    ; N = 0),
    N1 is N + 1,
    assert(tmp:save_commit_count(N1)),
    write('Saving commit '), write(N), nl,
    assert(tmp:my_commit(N, Commit)).

tmp:send1(Count) :-
    tmp:send1b(0, Count, [], Tx), commit(Tx) @- node(n1), save_commit(Tx).

tmp:send1b(EndCount, EndCount, Addresses, Tx) :-
    !,
    length(Addresses, N),
    tmp:amounts(0, N, Amounts),
    reverse(Addresses, AddressesRev),
    spend_many(AddressesRev, Amounts, 1234, Tx, OldUtxos) @ wallet(w1),
    retract_utxos(OldUtxos) @ wallet(w1).

tmp:send1b(Count, EndCount, Addresses, Tx) :-
    AddrN is Count mod 10,
    tmp:wallet2(AddrN, PubAddr),
    Count1 is Count + 1,
    tmp:send1b(Count1, EndCount, [PubAddr|Addresses], Tx).

%
% Send to wallet 2, 1000 outputs (10 addresses), three times.
% This will build a heap with a trie that is 3 levels high.
%
?- tic, password("foobar1") @ wallet(w1), tmp:send1(1000), sync_all @ wallet(w1), save @ wallet(w1), tmp:send1(1000), sync_all @ wallet(w1), save @ wallet(w1), tmp:send1(1000), sync_all @ wallet(w1), save @ wallet(w1), tmp:send1(1000), sync_all @ wallet(w1), save @ wallet(w1), toc.
% Expect: true/*

%
% What is the current balance?
%
?- balance(X) @ wallet(w1).
% Expect: true/*

%
% What is the chain?
%
?- chain @ node(n1).
% Expect: true

%
% What does the global state look like?
%
?- chain(5, [Id]) @ node(n1), db_key(Id, heap, 0, 1000, X) @ node(n1).
% Expect: true/*

%
% Go back one state
%
?- chain(4, [Id]) @ node(n1), switch(Id) @ node(n1), chain @ node(n1).
% Expect: true/*

%
% Reply last 1000 send. Make sure it doesn't fork.
%
reply :- tmp:my_commit(3, Commit), commit(Commit) @- node(n1).

?- reply, chain @ node(n1).
% Expect: true/*
