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
    spend_tx(PubAddr, Value, 1234, Tx, OldUtxos), retract_utxos(OldUtxos),
    commit(Tx) @- node,
    sync,
    Count1 is Count + 1,
    tmp:send1(Count1, EndCount).

?- tic, password("foobar1"), tmp:send(500), toc.
% Expect: true/*





