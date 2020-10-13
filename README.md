# Prologcoin (working title)

Mission: To fuse Prolog (the logic programming language) with
cryptocurrency technology.

## Latest Prologcoin binaries (automatically updated)

[Linux Ubuntu 18] https://github.com/datavetaren/prologcoin/releases/download/master/prologcoin_linux_ubuntu_18_latest.zip
[Mac OSX Catalina] https://github.com/datavetaren/prologcoin/releases/download/master/prologcoin_macosx_catalina_latest.zip
[Windows 10] https://github.com/datavetaren/prologcoin/releases/download/master/prologcoin_macosx_catalina_latest.zip

## How to build from source

Go to some directory:

```
> git clone https://github.com/datavetaren/prologcoin
> git clone https://github.com/datavetaren/secp256k1-zkp
> [install C++ Boost, 1.62 or later]
> cd prologcoin
> make all
```

## How to run

(Note, global query, blockchain, transactions, commits, etc. are still
not implemented.)

In two separate terminal windows do:
```
> bin/prologcoind --name foo --port 8701 --interactive
> bin/prologcoind --name bar --port 8702 --interactive
```

(You can launch multiple engines on the same machine on different
ports.)

There are some special commands and operators:

List current connections:

``` 
?- me:connections.
```

Add another prologcoind to your address book:

```
?- me:add_address('127.0.0.1', 8702).
```
(It'll get connected within a minute.)

```
?- member(X, [1,2,3,4]) @ bar.
```

Run the goal "member(X, [1,2,3,4])" on the node 'bar'. (Requires a
connection to the node 'bar'.)

Use the regular 'halt/0' predicate to quit (interactive) prologcoind:

```
?- halt.
```

For fun you can actually create compatible bitcoin keys with prologcoind:

```
?- ec:privkey(NewKey), ec:pubkey(NewKey,PubKey), ec:address(PubKey,Addr).
NewKey = 58'KzV52gSQKumZu1fuTbLjzHT3KYnaB3whqUxbFGAHoiqXY8CZMwgp,
PubKey = 58'1pneNLWqDHkAs33Gqz96YA7aRiBYwNUThjJg8k3rJdg56,
Addr = 58'1EQUjQiTSkQnpUz6sos6QL5FJd4dPYTHf2.
```

Yeah, only legacy bitcoin addresses for now. Shame on me.

## What is Prolog

Prolog is a language based on predicates (= "program") and terms (=
"data structures.") Prolog is basically all about "equation solving on
data structures".)

## Why Prolog?

Blockchains are about synchronizing a state among N machines (nodes)
over a non-trusted network. The state is proven to be valid using
various axioms/rules (also known as consensus.) Words such as
"proving," "validation," "axioms," and "logic" fit the Prolog
paradigm very well.

Prolog has the concept of "logic variables." The word "logic" in this
context is not assigning true/false values, but how to assign terms
(data structures) to make a certain equation true.

## Blockchain and queries

In Prolog you have an axiomatic set (called the "program") and then
you ask queries based on the axiomatic set. Here we imagine that the
blockchain is just an indefinitely global expandable query.  We have
the constraint that users are only allowed to extend goals to the
global query as long as the overall query doesn't fail.

For example,

```
?- X = 1.
```

Will produce a state where the logic variable 'X' is equal to the constant 1.
If a user attempts to extend this query with let's say X = 2:

```
?- X = 1, X = 2.
```

... then it would fail and the query extension and would be rejected
by the network.

A user can add new code using the traditional assert command.  For
example,

```
?- assert( p(X) :- X = 42 ), ... predicate p/1 is now available  ...
```

This shows how new "sub routines" can be added and thus enable the
system to become more expressive.

## Special terms

Certain terms with functors that start with a '$' character are
reserved and may only be created by the "operating system." Here,
"operating system" is not the actual operating system, but the
prologcoind engine.  Thus, "write mode" for terms are only allowed for
selected (built-in) predicates. For example,

```
?- X = '$coin'(1,2).
```

... will fail because the user is attempting to create a new term on
the heap containing a functor '$coin'. This special functor is
reserved by the operating system.

## Coins

```
'$coin'(Value, Spent)
```

Will be reserved for monetary value. Variable Value is bound to the
amount and Spent is either unbound which means the coin is available
for spending or '[]' (the empty list) when spent.

There are two built-in predicates (by the system) that allows you
joining or spliting coins and only these preciates are allowed to work
on '$coin' terms. cjoin0/3 is a helper predicate for cjoin/2 and is not
considered available directly from the user code.

```
cjoin(InCoins, '$coin'(Sum, _) ) :-
    cjoin0(InCoins, 0, Sum).
cjoin0(['$coin'(V, X)|InCoins], Acc, Sum) :-
    var(X), % Coin must be unspent!
    X = [], % No longer possible to spend
    Acc1 is Acc + V,
    cjoin0(InCoins, Acc1, Sum).
cjoin0([], Sum, Sum).
```

You can also split coins into mutliple smaller constituents using csplit/3:

```
csplit(InCoin, Values, OutCoins) :-
    V = '$coin'(InCoin, Spent),
    var(Spent),
    Spent = [],
    csplit0(Values, Available, OutCoins).
csplit0([Value|Values], Available, ['$coin'(Value, _)|OutCoins)]) :-
    Available >= Value,
    Available0 is Available - Value,
    csplit0(Available0, Values, OutCoins).
csplit0([], Available, OutCoins) :-
    (Available == 0 -> OutCoins = []
     ; OutCoins = ['$coin'(Available, _)]
    ).
```

For example, that Coin='$coin'(100, _):

```
?- csplit(Coin, [10 20], [MyCoin1, MyCoin2, MyCoin3]).
MyCoin1 = '$coin'(10, _),
MyCoin2 = '$coin'(20, _),
MyCoin3 = '$coin'(70, _)
```

These predicates, and the constraint that only these predicates can
write '$coin' terms, ensure that money cannot be created from thin
air. Only the (consensus) system may create '$coin'/2 terms.

## Hashes & signatures

To compute the (SHA256) hash of a term Prologcoin provides the hash/2
predicate:

```
hash(Term, Hash)
```

This is true iff Hash is the SHA256 hash of Term (which is uniquely
serialized in some format.) However, the hash can sometimes be wrapped in a term:

```
Hash = Value (numeric)   or   Hash = '$sys'(Value)
```

This enables us to distinguish if the hash has been computed by the
opearating system or by user code. We'll see in a moment why this is
important.

We can compute a signature using sign/3:

```
sign(Hash, PrivateKey, Sign)
```

And we can validate a signature using validate/3:

```
validate(Hash, PublicKey, Sign)
```

## Transactions

A monetary transaction can now defined as tx/6 (which is a system
builtin.) Note that CoinOut = '$coin'(V, _) writes a '$coin' term on
the heap, which is not allowed to do from user code.

```
tx(CoinIn, Hash, Sign, PubKey, PubKeyHash, CoinOut) :-
    CoinIn = '$coin'(V, X),
    var(X), % Not currently spent
    X = [], % Spend it
    freeze(Hash,  % Wait until Hash become bound
        hash(PubKey, PubKeyHash), % Validate PubKey
        ground(Hash), 
        Hash = '$sys'(_), % Hash must not have been computed by user code
        validate(Hash, PubKey, Sign), % Validate signature
	CoinOut = '$coin'(V, _)). % Let CoinOut become available/spendable.
```

When this predicate is run, it will mark CoinIn as spent and then halt
its execution (freeze/2) until the variable Hash is bound. It is
assumed that the variables PubKey and Sign are bound before Hash.
Once they all become bound it'll check that the provided Hash, Sign
and PubKey validate, where Hash has been computed by the operating
system (and not by user code.) And if it does, then CoinOut will
become available for spending.

So for example, let's say there's some a previous transaction with a
coin (on the global expandable query) that the user would like to
spend. Then the user first computes the signatures:

```
T = (t(SelfHash) :-
   PubKey = <constant provided by the user>
   Hash = SelfHash,
   % If Sign is provided this will now make "SomeCoin" spendable
   tx(SomeCoin, OutHash, OutSign, OutPubKey, somepubkeyhashconstant, OutCoin)).
```

Running 'commit(T)' will make the system first compute
'$sys'(SelfHash) where the Hash is computed on the Body of the
't(SelfHash) :- Body' clause. The hash is computed before the Body is
run, so all unifications (e.g. Hash = SelfHash) are just unbound
variables for the created term of T. However, running 'commit(T)' is
not enough as we need to bind 'Sign' (so that SomeCoin is released.)
The user can use T to compute the user hash of Body, i.e.

```
T = (_ :- Body), hash(Body, UserHash), sign(UserHash, PrivateKey, Signature)
```

And the user can now create two commits:

```
?- ... commit((Sign = Signature)), commit(T).
```

This somewhat complicated way of creating transactions is to ensure
that once commits are broadcasted to the network, they cannot be
modified by an attacker while in transit (to the miners.) The
signatures are thus separated from the action and thus make the
signatures non-mallable.

## MuSig

The previous definition of a transaction is the old ECDSA style.  Much
has happened since and Blockstream released (almost a year ago) a new
Schnorr signature framework called "MuSig" which enables efficient
multi-signatures. This MuSig framework also supports so called
"adaptor signatures" making it possible to incorporate an optional
hidden secret. That secret could be a hash of a script (or MAST) to
serve as a last resort of a dispute between the signers. Using the
MuSig library we could define the transaction as:

```
tx2(CoinIn, Hash, Sign, PubKey, PubKeys, CoinOut) :-
    CoinIn = '$coin'(V, X),
    var(X), % Not currently spent
    X = [], % Spend it
    freeze(Hash,  % Wait until Hash become bound
        ground(Hash), 
        Hash = '$sys'(_), % Hash must not have been computed by user code
        member(PubKey, PubKeys), % Make sure PubKey is an element of PubKeys
	ec:musig_combine(PubKeys, CombinedPubKey, _), % Combine public keys (Schnorr)
        ec:musig_verify(Hash, CombinedPubKey, Sign), % Validate signature
	CoinOut = '$coin'(V, _)). % Let CoinOut become available/spendable.
```

Note that this enables using a single signature for multiple UTXO inputs:

```
Sign = ...  (single signature for the combined public key)

T = (_ :- Body)

where Body:

PubKeys1 = [PubKey1, PubKey2, PubKey3, ...],
PubKeys2 = PubKeys1,
PubKeys3 = PubKeys1
cjoin([CoinOut1,CoinOut2,CoinOut3], SumCoin),
csplit(SumCoin, [10000000], [MyFee, MyCoin]),
tx2(MyCoin, _, _, <new public key>, _, _).
```

In order to improve performance we'd like to cache the result of
`ec:musig_combine(PubKeys, CombinedPubKey), ec:musig_verify(Hash, CombinedPubKey, Sign)`
so that the actual signature verification is done exactly once.

Note that the fee is simply an unconditional coin that can immediately
be grabbed by the miner who'll just add his own transaction:

`tx2(MyFee, _, _, <miner public key>, _, _)`

The cool thing is that it is relatively easy to define transaction
types using Prolog as the base language.

## Taproot

We can also formulate Taproot in this framework. Taproot means that an
UTXO has two spending paths; either the transaction is signed by all
parties (musig) or it is enforced via a script. It is implemented by
tweaking the published public key.

```
tx3(CoinIn, Hash, Sign, PubKeyMusig, PubKeyScript, Script, CoinOut) :-
    CoinIn = '$coin'(V, X),
    var(X), % Not currently spent
    X = [], % Spend it
    freeze(Hash,  % Wait until Hash become bound
        ground(Hash), 
        Hash = '$sys'(_), % Hash must not have been computed by user code
        (var(PubKeyScript) ->
            % Key spending path
	    ec:musig_verify(Hash, PubKeyMusig, Sign)
	  ; % Script spending path
            ec:musig_verify(Hash, PubKeyScript, Sign),
            hash([PubKeyMusig,Script], H),
	    ec:pubkey_tweak_add(PubKeyScript, H, PubKeyMusig),
	    call(Script)
	),
	CoinOut = '$coin'(V, _)). % Let CoinOut become available/spendable.
```

In this version there are two ways we can redeem the coins locked by
tx3(...). Either we do:

```
Sign = ... (A musig signature that validates with PubKeyMusig)
(PubKeyMusig has already been set in the previous transaction.)

or:

Sign = ...
PubKeyScript = ...
Script = ...
```

Note that the tx3/7 wakes up at the moment when Hash is bound, which
happens in the commit phase (as previously described.)

## Generalizing Transactions

Note that tx/6, tx2/6 and tx3/7 have a similar pattern, and we can generalize
the transaction model and factor the gateway predicate as follows:

```
tx(CoinIn, Hash, Script, Args, CoinOut) :-
    CoinIn = '$coin'(V, X),
    var(X),
    X = [],
    freeze(Hash,
           ground(Hash),
	   Hash = '$sys'(_),
           call(Script, Hash, Args),
	   CoinOut = '$coin'(V, _)).
```

Can can then formulate the variants as:

First is pay-to-pubkey-hash:

```
tx1(Hash, args(Sign, PubKey, PubKeyHash)) :-
    hash(PubKey, PubKeyHash),
    validate(Hash, PubKey, Sign).
```

where we then can rewrite the call to tx/5 as:

```
tx(CoinIn, Hash, tx1, args(Sign, PubKey, PubKeyHash)).
```


MuSig batched transactions is then:

```
tx2(Hash, args(Sign, PubKey, PubKeys)) :-
   member(PubKey, PubKeys),
   eu:musig_combine(PubKeys, CombinedPubKey, _),
   ec:musig_verify(Hash, CombinedPubKey, Sign).
```

And finally Taproot:

```
tx3(Hash, args(Sign, PubKeyMusig, PubKeyScript, Script)) :-
   (var(PubKeyScript) ->
        ec:musig_verify(Hash, PubKeyMusig, Sign)
      ; ec:musig_verify(Hash, PubKeyScript, Sign),
        hash([PubKeyMusig,Script], H),
	ec:pubkey_tweak_add(PubKeyScript, H, PubKeyMusig),
        call(Script)
   ).
```

The important thing to notice is that the transaction that moves the
coin is separate in 'tx' which guarantees that the money supply
cannot be altered. The gateway predicate to control whenever a coin
can be moved is thus abstracted in tx1, tx2 and tx3.

## Global State

The intent of Prologcoin is to run a regular Prolog execution of a
global expandable query (indefinitely) with some small additional
constraints. Keeping the execution model as close as possible to
regular execution of Prolog keeps the mental model relatively simple
for the cryptocurrency. The API will simply be interactions with
Prolog. The only difference is that there's a global query that is
maintained by all the nodes of the system.

## Garbage Collection

It turns out that the only relevant thing to keep around for the
global expandable query are the frozen closures (those are created
when calling freeze/2.) Everything else can be thrown away. Frozen
closures are like "interrupts" for embedded systems. They can suddenly
become awake, and they have the potential to falsify.

Thus we keep a set of all frozen closures, and all the data they can
reach. This is our live root set in a garbage collection system.
However, for the global state we don't do the traditional compacting
garbage collection, because we'd like all heap addresses to have
unique meanings. The heap grows linearily indefinitely, but if data
becomes unreachable, then there's no need to keep them in memory.  So
if the heap is organized into blocks, then we can conveniently forget
prior blocks that are unreachable.

To make sure everybody see the same heap, we use a merklezied tree on
both the live set and the heap blocks. By using proof-of-work on the
state (and not on the transitions) we also enable fast synching yet it
is possible for any node in the network to detect if a miner is
producing an erroneous state.

## Custom Tokens

The current security model for '$coin' in Prologcoin is that only a
limited set of predicates have write mode access for that particular
term. Recall that write mode means that new terms are created on the
heap.

It's possible to enable users create their own custom tokens by a
similar security model. Let's say have we token/2:

```
token('$mytoken', [mysplit, myjoin, mytx])
```

That creates a new token ('$mytoken') and only the predicates mysplit,
myjoin and mytx have write mode access to these terms. Now it's
possible to create a replica of '$coin' but for '$mytoken'.

We'll now extend the predicates with hidden local states.
If we annotate a predicate (in the list predicates of token/2) as:

```
token('$mytoken', [mysplit, myjoin, mytx, myswapin(state), myswapout(state)])
```

Then myswapin is assumed to be defined as:

```
myswapin(StateIn, StateOut, ... Args ...) :- ...
```

That is, the first two arguments are state variables, the input state
and the resulting output state. However, these arguments are omitted in
the call of myswapin:

```
myswapin( ... Args ...)
```

will be rewritten by the system to pass in the current state for that
token as well as what the output state is. Note that these states are
still declarative (i.e. they don't destructively update anything.)
This is similar to state monads in lazy functional programming
languages.

Before introducing the example, let's assume we make the following
modificatons.

* cjoin/2 and csplit/3 can accept any $-term (not just $coin)
* Same with tx/5 (the generalized version)
* czero/1 is czero('$coin'(0, _)) and can be accessed from any predicate

Consider the following definitions of myswapin and myswapout:

```
myswapin(StateIn, StateOut, RealCoinIn, TokenOut) :-
    % Extract from StateIn, or initialize a new state (StateIn = [] initially)
    (StateIn = state(CoinSum,Count) ; czero(CoinSum),Count=0),
    Count1 is Count + 1,                 % Increment counter
    StateOut = state(CoinNewSum,Count1), % Prepare new state
    cjoin([CoinSum, RealCoinIn], CoinNewSum), % Add real coin to sum
    RealCoinIn = '$coin'(Value, _), % Read mode as '$coin'(...) is ground
    TokenOut = '$mytoken'(Value, _).

myswapout(StateIn, StateOut, TokenIn, RealCoinOut) :-
    TokenIn = '$mytoken'(Value, X),
    var(X), % Not currently spent
    X = [], % Spend it
    StateIn = state(RealCoinSum, Count),
    Count1 is Count + 1,
    StateOut = state(NewCoinSum, Count1),
    csplit(RealCoinSum, [Value], [NewCoinSum, RealCoinOut]).
```

The things to notice:

* myswapin/4 is used to swap a real '$coin' for a '$mytoken'
* myswapout/4 is used to swap a '$mytoken' for a real '$coin'.
* The exchange rate between '$coin' and '$mytoken' is 1:1.
* The initial state is [] (set by system when the token is created)
* The state is represented with a term : state(RealCoin, Count)
* Count is the current number of swaps (in or out)

The basic idea is to enable users swapping real coins for something
else (and back) so that "something else" can be used in experiments.
Think Bitcoin Liquid, or off chain ZK SNARKs, Mimblewimble, etc.  The
idea is to have a state where coins can be accumulated and used with
different smart contracts without having to worry about wrecking the
original money supply for '$coin'.

## Appendix

### Basics of Prolog

For example, given

```
  foo(X,3) = foo(7,Y)
```

We can make left-hand side and right-side equal by _binding_ X = 7 and
Y = 3. Or a more sophisticated example:

```
   foo(A,bar(B,baz)) = foo(B,bar(42,C)). 
```

Can be solved with A = B = 42 and C = baz. This process of "solving
data structure equations" is known as "unification."

When you write programs in Prolog it's all about writing predicates
that are proven true. For example, you can type your query (= "the
stuff you'd like to prove") as:

```
   append(X, [1,2,3,4], Z)
```

to make Prolog trying to search for all possible solutions where Z is
the concatenation of the list X and [1,2,3,4].

There'll be infinitely many of course, but it will nevertheless
enumerate them. Something like:

```
   X = [], Z = [1,2,3,4]
   X = [A], Z = [A,1,2,3,4]
   X = [A,B], Z = [A,B,1,2,3,4]
   ...
```

append(Xs,Ys,Zs) is typically a built-in predicate, but you can also define
it yourself:

```
   append([],Xs,Xs).
   append([X|Xs], Ys, [X|Zs]) :- append(Xs, Ys, Zs).
```

i.e. you define when append/3 should be true. ":-" is an implication
arrow. I know that ":-" looks ugly and why not "<-" ? But that's
legacy we're inheriting. "[A|B]" is list construction (where A is
the first element and B the rest of the list.) A common idiom is to
name variables "X" to represent a single element and "Xs" for multiple
elements ("s" for plural.)

When you bind a logic variable in Prolog, you cannot change its
value. (Unless the underlying engine is "searching for a solution" for
which variables are constantly unbound and rebound.)

You can still make something grow though:

```
X = [1|Y], Y = [2|Z], Z = [3|Q], ...
```

The concept of not being able to change the direct value of a bound
variable fits very well with "immutability."

