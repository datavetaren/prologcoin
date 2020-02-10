%
% Startup
%
% Meta: WAM-only
% Meta: fileio on

?- create("foobar", [hurdle,review,jump,cash,fork,history,spider,bracket,collect,inherit,tape,keep,advice,exist,suspect,deny,judge,fog,fold,violin,hub,agent,elegant,fringe]).
% Expect: true.

%
% Create a new key.
%
?- newkey(PubKey, PubKeyAddress).
% Expect: PubKey = 58'25R88LyidJyM4ePTouuWsXBBV5BgXZibY34jwQ15Mow8j, PubKeyAddress = 58'1HsxgzLx5WSQLMC5awUNuGPaSo2csEEYFq

%
% Create another key
%
?- newkey(PubKey2, PubKeyAddress2).
% Expect: PubKey2 = 58'1hmJAajYJPuamGbNaT97dPY2D4rGjMDYXidmANNNBwVWj, PubKeyAddress2 = 58'151uwoSrYcQu7xrf8b5ot4XUR9kVGtUT1X

%
% Create third key
%
?- newkey(PubKey3, PubKeyAddress3).
% Expect: PubKey3 = 58'1t3kcH1TsQKstb5wQi9Qc2vPFUfavuFTXMmtaJWy6R8EW, PubKeyAddress3 = 58'1454TkdyBBsrT2udu86h4WwZnBCc8FHbBy


%
% Create some rewards at the node using first key
%
?- commit(reward(58'1KjJ9E9TekYgSmPUFiFYre6BhKrBqciY81)) @ node.
% Expect: true
?- commit(reward(58'1HsxgzLx5WSQLMC5awUNuGPaSo2csEEYFq)) @ node.
% Expect: true
?- commit(reward(58'1HsxgzLx5WSQLMC5awUNuGPaSo2csEEYFq)) @ node.
% Expect: true

%
% Create some rewards at the node with second key
%
?- commit(reward(58'151uwoSrYcQu7xrf8b5ot4XUR9kVGtUT1X)) @ node.
% Expect: true
?- commit(reward(58'151uwoSrYcQu7xrf8b5ot4XUR9kVGtUT1X)) @ node.
% Expect: true

%
% The global state should have 5 frozen closures
%
check_closures(N) :- ((frozenk(0, 100, HeapAddrs) @ global) @ node), length(HeapAddrs,N).

?- check_closures(5).
% Expect: true

%
% Sync the wallet with the global state
%
?- sync.
% Expect: true

%
% Get the balance, should be 21000000000*4 (the standard block reward * 4)
%
?- balance(B).
% Expect: B = 84000000000

%
% Compute a spending transaction (later to be broadcasted)
% This is actually spending to myself (key number 3.)
% Sending 31_000_000_000 (31 billion units) 
%
?- password("foobar"), spend_tx(58'1454TkdyBBsrT2udu86h4WwZnBCc8FHbBy, 31_000_000_000, 32_000, FinalTx).
% Expect: true/*

%
% There's a bug here. Private key is different every time, but public key is not.
%

:- password("foobar"), wallet:privkey(0, X), wallet:pubkey(0, Y), write(X), nl, write(Y), nl.
% Expect: true/*

:- password("foobar"), wallet:privkey(0, X), wallet:pubkey(0, Y), write(X), nl, write(Y), nl.
% Expect: true/*


% xxxx: FinalTx = (Signature = 58'4tfgozsLwVi425jQnjZNgWP4GBY7Uw87mwXFoWLK4NVRxQZ1LjEtaKE7btnys4HXttjWd95WiwJA4wZA75aaTMNM, Signature1 = 58'4cEHnkJrvJYRbjxCgYqPkPNCNVMJYwhi1wcJ5aQFX7pZXd4KDQcyiHRfXSdMmcRKfBUDMWt8xJbeJKiHFfpJEcNX, (p(Hash) :- defrost(1591, Closure, [Hash, args(Signature, 58'25R88LyidJyM4ePTouuWsXBBV5BgXZibY34jwQ15Mow8j, 58'1HsxgzLx5WSQLMC5awUNuGPaSo2csEEYFq)]), arg(4, Closure, Coin), defrost(1693, Closure1, [Hash, args(Signature1, 58'1hmJAajYJPuamGbNaT97dPY2D4rGjMDYXidmANNNBwVWj, 58'151uwoSrYcQu7xrf8b5ot4XUR9kVGtUT1X)]), arg(4, Closure1, Coin1), cjoin([Coin,Coin1], SumCoin), csplit(SumCoin, [31000000000,32000,10999968000], [FundsCoin,_,RestCoin]), tx(FundsCoin, Hash, tx1, args(_, _, 58'1454TkdyBBsrT2udu86h4WwZnBCc8FHbBy)), tx(RestCoin, Hash, tx1, args(_, _, 58'1EfXJQ3vnfJMwBzjaygQq1epFGkxvJbweH))))

