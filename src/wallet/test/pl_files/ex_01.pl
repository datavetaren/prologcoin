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
% Create some rewards at the node using first key
%
?- commit(reward(founders_reward)) @ node.
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
