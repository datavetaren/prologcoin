%
% Testing wallet. Create a new wallet and then make some transactions.
%

% Meta: WAM-only
% Meta: fileio on

?- create("foobar", [hurdle,review,jump,cash,fork,history,spider,bracket,collect,inherit,tape,keep,advice,exist,suspect,deny,judge,fog,fold,violin,hub,agent,elegant,fringe]).
% Expect: true.

%
% Create a new key.
%
?- newkey(PubKey, PubKeyAddress).
% Expect: PubKey = 58'1gkobm5Lkp7D6wLKuUsPrA9t46eGswjieMSZtdJsvJH1J, PubKeyAddress = 58'1NHqbNvC6GahrKKpcvomhr3YLgwnwqdfDA

%
% Create another key
%
?- newkey(PubKey2, PubKeyAddress2).
% Expect: PubKey2 = 58'28fytjeHRDG3WqiLRVWmu6tp4Y1cAQ76WTxnx6JXoERnV, PubKeyAddress2 = 58'1F1krUUBG9dQbLahcGbEEPsiiCffMTRs8v

%
% Create third key
%
?- newkey(PubKey3, PubKeyAddress3).
% Expect: PubKey3 = 58'1sJWiEpLVeYiZBxgnsXyoRtfAJ3a6YXTftqCoWPrE11Ht, PubKeyAddress3 = 58'1DgKvJZtrs8KEqWEcrPXW2a7A5TYDoYwKm


%
% Create some rewards at the node using first key
%
?- commit(reward(58'1KjJ9E9TekYgSmPUFiFYre6BhKrBqciY81)) @ node.
% Expect: true
?- commit(reward(58'1F1krUUBG9dQbLahcGbEEPsiiCffMTRs8v)) @ node.
% Expect: true
?- commit(reward(58'1F1krUUBG9dQbLahcGbEEPsiiCffMTRs8v)) @ node.
% Expect: true

%
% Create some rewards at the node with second key
%
?- commit(reward(58'1NHqbNvC6GahrKKpcvomhr3YLgwnwqdfDA)) @ node.
% Expect: true
?- commit(reward(58'1NHqbNvC6GahrKKpcvomhr3YLgwnwqdfDA)) @ node.
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
?- password("foobar"), spend_tx(58'1DgKvJZtrs8KEqWEcrPXW2a7A5TYDoYwKm, 31_000_000_000, 32_000, FinalTx, ConsumedUtxos), write(FinalTx), nl, commit(FinalTx) @ node, retract_utxos(ConsumedUtxos), sync.
% Expect: true/*

%
% Check the new balance. Since we spent to ourselves it's only the fee that is lost.
%
?- balance(B2).
% Expect: B2 = 83999968000

%
% Resync and see we get the same result.
%
?- resync, balance(B3).
% Expect: B3 = 83999968000

%
% Send everything to some random address
%
?- password("foobar"), spend_tx(58'13PCw4x3Pc7AKum9Rc8nEgzbq4BX2rvEqf, 83_999_968_000, 0, FinalTx2, ConsumedUtxos2), commit(FinalTx2) @ node, retract_utxos(ConsumedUtxos2), sync.
% Expect: true/*

?- balance(B4).
% Expect: B4 = 0
