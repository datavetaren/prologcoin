# Prologcoin (working title)

Mission: To fuse Prolog (the logic programming language) with
cryptocurrency technology.

## How to build

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

Yeah, only legacy addresses for now. Shame on me.

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

Prolog also has a peculiar concept called "logic variables." The word
"logic" should not be confused with true/false, but rather that these
variables can be bound (to terms) in order to solve a "data structure"
equation.

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

... then it would fail and the global query and would be rejected by
the network.

A user can add new code using the traditional assert command.  For
example,

```
?- assert( p(X) :- X = 42 ), ... predicate p/1 is now available  ...
```

## Special terms

Terms with functors that start with a '$' character are reserved and
may only be created by the "operating system." Here, "operating
system" is not the actual operating system, but the prologcoind
engine.  Thus, "write mode" for terms are only allowed for selected
(built-in) predicates. For example,

```
?- X = '$foo'(1,2).
```

... will fail because the user is attempting to create a new term on
the heap containing a functor with a leading '$' character.

## Coins

```
'$coin'(Value, Spent)
```

Will be reserved for monetary value. Value is bound to the amount and
Spent is either unbound = coin is available / not spent, and if the
argument is '[]' (the empty list) then it is considered spent.

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
serialized in some format.) However, the hash is wrapped in a term:

```
Hash = user(Value)   or   Hash = '$sys'(Value)
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

A transaction can now defined as:

```
tx(CoinIn, Hash, Sign, PubKey, PubKeyHash, CoinOut) :-
    CoinIn = '$coin'(V, X),
    var(X), % Not currently spent
    X = [], % Spend it
    freeze([Hash, Sign, PubKey],  % Wait until vars become bound
        hash(PubKey, PubKeyHash), % Validate PubKey
        ground(Hash), 
        Hash = '$sys'(_), % Hash must not have been computed by user code
        validate(Hash, PubKey, Sign), % Validate signature
	CoinOut = '$coin'(V, _)). % Let CoinOut become available/spendable.
```

When this predicate is run, it will mark CoinIn as spent and then halt
its execution (freeze/2) until the variables Hash, Sign and PubKey have
become bound.  Once they become bound it'll check that the provided
Hash, Sign and PubKey validate, where Hash has been computed by the
operating system (and not by user code.)

So for example, let's say there's some a previous transaction with a
coin (on the global query) that the user would like to spend. Then the
user first computes the signatures:

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

## Summary

The intent of Prologcoin is to run a regular Prolog execution of a
query (indefinitely) with some small additional constraints on how you
can append things to the global query. Keeping the execution model as
close as possible to a regular execution of Prolog keeps the mental
model relatively simple for the cryptocurrency. The API will simply be
interactions with Prolog. The only difference is that there's a global
query that is maintained by all the nodes of the system.

## Open Questions

Garbage collection. Once the heap grows we'd like to prune things that
are irrelevant. For example, spent coins and other data structures can
be ignored if the only relevant thing to track are the unspent coins
(which would model the UTXOs in Bitcoin.) We could take the approach
that terms can be joined with UTXO coins to store additional data (not
to be garbage collected.) Thus, any attempt to refer to logic
variables where all terms are ground and where no unspent '$coin' is
available, could be treated as failing unifications.

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

