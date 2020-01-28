%
% Include standard lib
%

:- [std].

%
% Freeze simple
%

foo(X,Y) :- freeze(X, (Y = bound(X))).

?- foo(Q1, Q2), Q1 = 42.
% Expect: Q1 = 42, Q2 = bound(42)
% Expect: end

%
% Freeze with backtracking
%

foo2(X,Y) :- member(A, [1,2,3,4]), freeze(X, (Y = bound(X,A))).

?- foo2(Q3, Q4), Q4 = 4711.
% Expect: Q4 = 4711
% Expect: Q4 = 4711
% Expect: Q4 = 4711
% Expect: Q4 = 4711
% Expect: end

?- foo2(Q5,Q6), Q5 = 4711.
% Expect: Q5 = 4711, Q6 = bound(4711,1).
% Expect: Q5 = 4711, Q6 = bound(4711,2).
% Expect: Q5 = 4711, Q6 = bound(4711,3).
% Expect: Q5 = 4711, Q6 = bound(4711,4).
% Expect: end

%
% Nested freeze
%

foo3(X, Y, A, B) :- freeze(A, freeze(B, Y = bound(X,A,B))).

?- foo3(Q7,Q8,Q9,Q10), Q9 = 1, Q7 = 42, Q10 = 4711.
% Expect: Q7 = 42, Q8 = bound(42, 1, 4711), Q9 = 1, Q10 = 4711.
% Expect: end

%
% Nested interpreted freeze
%

foo4(X, Y, A, B) :- W = freeze(B, Y = bound(X,A,B)), freeze(A, W).
?- foo4(Q11,Q12,Q13,Q14), Q13 = 1, Q11 = 42, Q14 = 4711.
% Expect: Q11 = 42, Q12 = bound(42, 1, 4711), Q13 = 1, Q14 = 4711.
% Expect: end

%
% Testing frozen closures
%

% Meta: WAM-only

foo5 :- freeze(A, B = hello(A)), frozenk(-1, 10, Xs), Xs = [_].
?- foo5.
% Expect: true
% Expect: end

% Unfreeze frozen closures by accessing them explicitly
foo6(Closure) :- foo5, frozenk(-1, 10, [Addr]), frozen(Addr, Closure).
foo7(T) :- foo6(Closure), arg(2, Closure, Closure0), arg(1, Closure0, V), V = 424711, arg(2, Closure0, T).
?- foo7(T), frozenk(0, 10, []).
% Expect: T = hello(424711).
% Expect: end

%
% Testing transaction idea
%
dummy_hash(thepubkey, thepubkeyhash).
dummy_validate(sys(somehash), thepubkey, thesign).

tx(CoinIn, Hash, Sign, PubKey, PubKeyHash, CoinOut) :-
    CoinIn = coin(V, X),
    var(X),
    X = [],
    freeze(Hash,
	   (dummy_hash(PubKey, PubKeyHash),
	    ground(Hash),
	    Hash = sys(_),
	    dummy_validate(Hash, PubKey, Sign),
	    CoinOut = coin(V, _))).

foo8 :-
    CoinIn = coin(100, _),
    tx(coin(100, _), Hash, Sign, PubKey, thepubkeyhash, CoinOut).
?- foo8.
% Expect: true

%
% Now spend that frozen coin...
%

% Meta: fileio on
    
foo9 :-
    foo8, % Existing coin...
    frozenk(0, 10, [Addr]),
    frozen(Addr, Closure),
    arg(2, Closure, Closure0),
    % Check that we got the right closure
    arg(3, Closure0, thepubkeyhash),
    % Prepare everything except hash
    arg(2, Closure0, thepubkey),
    arg(5, Closure0, thesign),
    % Wake up frozen closure...
    arg(1, Closure0, sys(somehash)),
    % Now we have the coin:
    arg(6, Closure0, OurCoin),
    tx(OurCoin, _, _, _, youraddress, _),
    write(OurCoin), nl.

?- foo9.
% Expect: true
