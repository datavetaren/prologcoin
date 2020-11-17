% Meta: WAM-only

% Meta: erase all test wallets

% Remove global DB
?- drop_global @ node.
% Expect: true

?- (nolimit, heartbeat) @ node.
% Expect: true

%
% Wallets
%
% X = [skate,glory,blur,park,famous,cattle,bread,drift,arch,track,absurd,hospital,estate,sock,laundry,regular,rely,neutral,quiz,coil,exchange,slogan,human,choose]
%
% X = [define,menu,year,glad,accident,tail,scissors,ostrich,only,place,coast,always,guilt,lizard,once,evoke,iron,nothing,explain,burger,elbow,spy,dog,comic]
%
% X = [wealth,excuse,alien,enough,puzzle,borrow,attitude,post,air,decorate,offer,betray,stock,clump,skirt,lava,marble,walnut,rely,they,craft,resist,brain,often]
%
% X = [gift,model,laugh,perfect,zone,fee,radar,height,income,valid,long,witness,jeans,muffin,install,random,candy,iron,reveal,oxygen,uncover,turn,tenant,fence]
%
% X = [minute,mass,cool,eagle,crack,attack,shield,call,oblige,moon,confirm,scissors,minimum,odor,night,rescue,analyst,winter,address,umbrella,wing,cabbage,cream,sword]
%
% X = [reason,lens,champion,crumble,lottery,paddle,deputy,special,print,own,series,space,shoe,april,wrap,gravity,clean,alert,ball,lesson,attend,latin,permit,shoulder]
% X = [rely,decorate,any,struggle,core,own,sibling,sand,between,hospital,taste,planet,matter,wage,decline,foot,state,hour,forum,gown,swarm,develop,hedgehog,glance]
% X = [tortoise,zone,display,boat,script,pitch,idle,material,city,snow,artwork,bus,sniff,finish,buyer,urban,lab,attract,undo,sight,sing,begin,pulp,play]
%
% X = [embark,logic,purpose,eternal,cigar,say,catalog,recipe,snow,erupt,burst,upon, cactus,decide,stem,order,index,fiber,clog,lady,obvious,script,host,wait]
%
% X = [soon,kidney,pilot,ship,stock,neither,quit,reject,tongue,horn,faint,beef,lady,vendor,skirt,humble,laugh,labor,execute,raven,lawn,dose,orange,loyal]
%

?- file("test_wallet1.pl").
% Expect: true/*    

?- create("foobar1", [skate,glory,blur,park,famous,cattle,bread,drift,arch,track,absurd,hospital,estate,sock,laundry,regular,rely,neutral,quiz,coil,exchange,slogan,human,choose]).
% Expect: true/*

?- newkey(A,B), assert(tmp:monieee(B)).
% Expect: true/*

?- tmp:monieee(Addr), (reward(Addr) @ global) @ node.
% Expect: true/*

?- sync.
% Expect: true/*

?- balance(X).
% Expect: X = 42445243724242. 

?- save.
% Expect: true/*

%
% Wallet 1 now has a lot of money
%

%
% Create new wallet 2
%

?- file("test_wallet2.pl").
% Expect: true/*

?- create("foobar2", [define,menu,year,glad,accident,tail,scissors,ostrich,only,place,coast,always,guilt,lizard,once,evoke,iron,nothing,explain,burger,elbow,spy,dog,comic]).
% Expect: true/*

% Create some keys in wallet 2 och store the addresses at node.
% (Note that entire interpreter is wiped when we switch wallets, so that's why we
% store it at the node (@ node).) It would be convenient if one could create scratch
% interpreters to store stuff in... will think about that.
%
tmp:create_addrs(N,N) :- !.
tmp:create_addrs(N, End) :-
    newkey(PubKey, PubAddr), assert(tmp:wallet2(N, PubAddr)) @ node,
    N1 is N + 1,
    tmp:create_addrs(N1, End).

?- tmp:create_addrs(0, 10).
% Expect: true/*

%
% Switch back to wallet 1
%

?- file("test_wallet1.pl").
% Expect: true/*

%
% Create 100 transactions to wallet 2 (using the 10 addresses.)
%

tmp:send(Count) :-
    tmp:send1(0, Count), advance @ node.
tmp:send1(EndCount, EndCount) :- !.
tmp:send1(Count, EndCount) :-
    AddrN is Count mod 10,
    tmp:wallet2(AddrN, PubAddr) @ node,
    % write('sending '), write(Count), write(' to '), write(PubAddr), nl,
    Value is 1000000 + Count,
    spend_one(PubAddr, Value, 1234, Tx, OldUtxos), retract_utxos(OldUtxos),
    commit(Tx) @- node,
    sync,
    Count1 is Count + 1,
    tmp:send1(Count1, EndCount).

?- tic, password("foobar1"), tmp:send(100), toc.
% Expect: true/*

%
% Switch to wallet 2 and see if we have the expected funds
%

?- file("test_wallet2.pl").
% Expect: true/*

%
% The global state has advanced, so we need to sync.
%
?- sync, sync.
% Expect: true/*

% Print balance
?- balance(X2).
% Expect: true/*

expected_balance(0,0) :- !.
expected_balance(Exp,N) :- N1 is N - 1, expected_balance(Exp1,N1),
			Exp is 1000000 + N1 + Exp1.

% Check balance
?- balance(X3), expected_balance(X3,100).
% Expect: true/*

?- file("test_wallet3.pl").
% Expect: true/*

%
% This is wallet 3
%

?- create("foobar3", [wealth,excuse,alien,enough,puzzle,borrow,attitude,post,air,decorate,offer,betray,stock,clump,skirt,lava,marble,walnut,rely,they,craft,resist,brain,often]).
% Expect: true/*

%
% Create some keys in wallet 3 och store the addresses at node.
%
tmp:create_addrs3(N,N) :- !.
tmp:create_addrs3(N, End) :-
    newkey(PubKey, PubAddr), assert(tmp:wallet3(N, PubAddr)) @ node,
    N1 is N + 1,
    tmp:create_addrs3(N1, End).

?- tmp:create_addrs3(0, 10).
% Expect: true/*

%
% Next step is to create one transaction with many outputs. (Aggregate what
% we did for wallet 2.) So go back to wallet 1 (that has so much money.)
%

?- file("test_wallet1.pl").
% Expect: true/*

%
% Create 1 transaction with 100 outputs to wallet 3 (using the 10 addresses.)
%

tmp:amounts(N, N, []) :- !.
tmp:amounts(Count, EndCount, [Value|Amounts]) :-
    Value is 1000000 + Count,
    Count1 is Count + 1,
    tmp:amounts(Count1, EndCount, Amounts).

tmp:send3(Count) :-
    tmp:send3b(0, Count, [], Tx), commit(Tx) @- node, advance @ node.

tmp:send3b(EndCount, EndCount, Addresses, Tx) :-
    !,
    length(Addresses, N),
    tmp:amounts(0, N, Amounts),
    reverse(Addresses, AddressesRev),
    spend_many(AddressesRev, Amounts, 1234, Tx, OldUtxos),
    retract_utxos(OldUtxos).
tmp:send3b(Count, EndCount, Addresses, Tx) :-
    AddrN is Count mod 10,
    tmp:wallet3(AddrN, PubAddr) @ node,
    Count1 is Count + 1,
    tmp:send3b(Count1, EndCount, [PubAddr|Addresses], Tx).

%
% Send to wallet 3, 100 outputs (10 addresses)
%
?- tic, password("foobar1"), tmp:send3(100), toc.
% Expect: true/*

%
% Switch to wallet 3. And sync.
%

?- file("test_wallet3.pl").
% Expect: true/*

%
% The global state has advanced, so we need to sync.
%
?- sync, sync, sync.
% Expect: true/*

% Print balance
?- balance(X4).
% Expect: true/*

expected_balance3(0,0) :- !.
expected_balance3(Exp,N) :- N1 is N - 1, expected_balance3(Exp1,N1),
			Exp is 1000000 + N1 + Exp1.

% Check balance
?- balance(X5), expected_balance3(X5,100).
% Expect: true/*

%
% Here comes the interesting test. We'll now transfer from wallet 3
% to wallet 4 and build 100 transactions for that. This will be stored
% in one block and we can clock how long it'll take to validate that block
% (by going back one step, and then replay.)
%

%
% Wallet 4
%
?- file("test_wallet4.pl").
% Expect: true/*

?- create("foobar4", [gift,model,laugh,perfect,zone,fee,radar,height,income,valid,long,witness,jeans,muffin,install,random,candy,iron,reveal,oxygen,uncover,turn,tenant,fence]).
% Expect: true/*

%
% Create some keys in wallet 4 och store the addresses at node.
%
tmp:create_addrs4(N,N) :- !.
tmp:create_addrs4(N, End) :-
    newkey(PubKey, PubAddr), assert(tmp:wallet4(N, PubAddr)) @ node,
    N1 is N + 1,
    tmp:create_addrs4(N1, End).

?- tmp:create_addrs4(0, 10).
% Expect: true/*

%
% Go back to wallet 3
%
?- file("test_wallet3.pl").
% Expect: true/*

tmp:send4(Count) :-
    tmp:send4b(0, Count, [], TxList),
    tmp:list_to_commas(TxList, Tx),
    write('Size of transactions: '), term_size(Tx, N), write(N), write(' bytes.'), nl,
    commit(Tx) @- node.

tmp:list_to_commas([X], C) :- !, C = X.
tmp:list_to_commas([X|Xs], C) :-
    C = (X, Y), tmp:list_to_commas(Xs, Y).

tmp:send4b(EndCount, EndCount, Tx, Tx) :- !.
tmp:send4b(Count, EndCount, TxIn, TxOut) :-
    AddrN is Count mod 10,
    tmp:wallet4(AddrN, PubAddr) @ node,
    Value is 100000 + Count,
    spend_one(PubAddr, Value, 1234, Tx, OldUtxos), retract_utxos(OldUtxos),
    append([Tx], TxIn, TxIn1),
    Count1 is Count + 1,
    tmp:send4b(Count1, EndCount, TxIn1, TxOut).

?- tic, password("foobar3"), tmp:send4(100), toc.
% Expect: true/*

%
% Go to wallet 4 again
%
?- file("test_wallet4.pl").
% Expect: true/*
?- sync, sync, sync, sync, sync.
% Expect: true/*

% Print balance
?- balance(X6).
% Expect: true/*

expected_balance4(0,0) :- !.
expected_balance4(Exp,N) :- N1 is N - 1, expected_balance4(Exp1,N1),
			Exp is 100000 + N1 + Exp1.

% Check balance
?- balance(X7), expected_balance4(X7,100).
% Expect: true/*
