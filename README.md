# Prologcoin (working title)

The purpose of this cryptocurrency is to combine MimbleWimble (the
only known scalable technology for cryptocurrencies) with the
execution engine of Prolog (a logic programming language.)

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

Prolog also has a pecular concept called "logic variables." The word
"logic" should not be confused with true/false, but rather that these
variables can be bound (to terms) in order to solve a "data structure"
equation.

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
variable fits very well with "immutability" that is also
important in blockchains.

## Prolog, Cryptocurrencies and MimbleWimble

The main reason for chosing Prolog is:

1. Anything in Prolog is about predicates, something that is true/false
   by definition.
2. Logic variables enable state transitions.

## Mimblewimble recap

This intro to Mimblewimble is similar to the one for Grin, but I tell
the story in a slightly different way.

### Elliptic Curves

Whenever you see:

```
E = a*G
```

You should know that you only can compute `a*G` if you know 'a'.  Just
knowing `a*G` doesn't let you know 'a'. This is the foundation of
elliptic curve cryptography. Note that a*G is really a function: G(a),
but we use the syntax `a*G` because this function is linear and
commutative, so algebraic operations such as adding and/or multiplying
make sense. In fact, the function G is a so called group generator,
and you can choose an arbitrary point on the elliptic curve as your
generator. Encryption libraries offer standard points so you never
have to worry about the internal details. Just think that whenever you
see 'a' you can compute `a*G` using a library function.

It's also very deceiving when you read this document. For example,
when you see:

```
E = 42*G
```

You think that, aha! the private key is 42, but if you're just the
recipient to its product it just looks like a big random (256 bit)
number:

```
E = 12838747271397732182286342
```

And from this it is impossible to extract '42' from this, unless you
already know the value. Guessing/iterating over all numbers is in fact
the only known way of finding that value. Of course, in practice, we
don't choose small numbers as 42 either, but rather big 256-bit random
numbers.

### Pedersen commitments

A Pedersen commitment is an arithmetic computation over an elliptic
curve such as:

```
P = b*G + v*H
```

where 'b' and 'v' are plugged in parameters. G is a generator point
for a elliptic curve. H is just another generator on the same curve
that is guaranteed to be "numerically disjoint." In practice this is
done by just setting H = hash(G). Again, an elliptic curve encryption
library has standard values for these, so given 'b' and 'v' it will
just compute 'P' for us.

We say that `b*G` is a blinding factor. The 'b' serves as a private key.
If we imagine that P (which is just a 256-bit big number) is an UTXO
(Unspent Transaction Output), then we can prove we own that UTXO, even
though the rest of the world don't know the value of 'v'.

The owner simply does `P - v*H`, because the owner knows 'v', he can
easily compute v*H, and then the owner signs a message with his
private key 'b' and everybody can confirm the signature using `P - v*H`
as the public key. The message to be signed can be any 256-bit number,
e.g. the SHA256 hash of the empty string (or whatever.)

### Double Communication. Unfortunately.

For Mimblewimble transactions we'll see that, unfortunately, there
needs to be a direct connection between sender and recipient of funds.
This is in contrast with Bitcoin where the recipient just publish a
receiving address and the sender of funds can just broadcast a
transaction based on that address.

In Mimblewimble the owner of an UTXO may need to split his private key
into a sum subset:

```
P = b*G + v*H = (b1+b2+...+bn)*G + (v1+v2+...+vn)*H 
  = b1*G + b2*G + ... + bn*G + v1*H + v2*H + ... + vn*H
```

Some of these bi's (and vi's) need to be sent to the recipient, which
reacts and replies with information back to the sender. Note that this
scheme still supports "cold storage," i.e. the recipient does not have
to have his destination private keys online for the receiving funds.

### Transactions

In Mimblewimble we identify what UTXOs we'd like to spend (each UTXO
is a Pedersen commitment) and then we identify one (or more)
recipients of funds. What we'll do is to compute the difference:

Outputs - Inputs

Where Inputs are just the sum of all UTXOs. And outputs are also a sum
of all new UTXOs. Then we'll prove some properties over this
difference.  These proofs will guarantee that no theft has taking
place and that there was no money creation either in this process,
i.e.  the monetary inflation is solely determined by the coinbases of
the currency.

Let's say we have Alice who owns:

```
P = 42*G + 8*H
```

(here 42 is normally a ridiciously large number, which will serve as
the private key for Alice.) Now she wants to send 3 units to Bob and 4
units to Carol (and keep 1 unit to herself.)  First she'll split her
private key 42 as a sum of 3 random numbers:

```
20 + 5 + 17 = 42
```

This means that:

```
P = 20*G + 1*H + 5*G + 3*H + 17*G + 4*H ( = 42*G + 8*H)
```

Bob wants his UTXO in `4711*G + 3*H`. So Alice gives Bob 5 (one number
of the splitted sum.) Bob then computes `4711*G+3*H` and `(4711-5)*G`
and gives both these numbers back to Alice, who just replaces:
`5*G+3*H` with difference of those two:

```
P = 20*G + 1*H + (4711*G+3*H) - (4711-5)*G + 17*G + 4*H
```

What can be seen here is that nothing has changed. Alice doesn't know
4711, and she doesn't know 4711-5 either. But she can verify that the
difference between those two big randomly numbers that she got from
Bob indeed becomes the same as `5*G + 3*H`.

Let's proceed with Carol. She wants her UTXO to be in `123*G+4*H`.  Bob
gives her 17 and 4, and Carol computes: `(123*G+4*H) - (123-17)*G`.
Gives those numbers back to Bob. P is now composed of:

```
P = 20*G + 1*H + (4711*G+3*H) - (4711-5)*G + (123*G+4*H) - (123-17)*G
```

Numerically, nothing has changed. But we'll only going to publish the
new UTXOs, so how can the rest of the world verify that...

1) No new money has been created?
2) That money wasn't stolen?

So let's split the above P into three parts:

```
P1 = 20*G + 1*H
P2 = (4711*G+3*H) - (4711-5)*G
P3 = (123*G+4*H) - (123-17)*G
P = P1 + P2 + P3
```

If we remove the negative terms from P2 and P3, then we get the new
UTXO set:

```
P1' = 20*G + 1*H
P2' = 4711*G + 3*H
P3' = 123*G + 4*H
```

And these are the numbers we'd like on the blockchain, so
that everyone in the world knows its new state of ownership. But then
difference becomes:

```
P1'+P2'+P3' - P = (4711-5)*G + (123-17)*G + 0*H
```

This means the rest of world can be convinced we didn't create new
funds in this process if we can prove that the difference is a valid
point on the G curve (which it only can be if H=0).
So what remains is proving:

`(4711-5)*G` is a point on the G curve, and
`(123-17)*G` is a point on the G curve. The law of addition also states
that the sum of these is also a point on the G curve.

Only Bob knows 4711-5, and can use this number to sign something. That
signature can then be verified using `(4711-5)*G`.

Only Carol knows 123-17, and she can use this number to sign
something. That signature can then be verified using `(123-17)*G`.

These signatures can be given back to Bob who can then publish the
entire transaction:

1. The new UTXO set and what UTXOs that got spent.
2. The signatures and the corresponding public keys
   (in this example `(4711-5)*G` and `(123-17)*G` respectively.)

But there's one more thing. In this process we can trick the balance
with the use of negative numbers. It's perfectly possible to create a
balanced (H=0) by doing something like:

```
P = 42*G + 8*H
```

is rewritten into:

```
P = 20*G + 1000008*H + 22*G - 1000000*H
```

And this become the same value as above. The H's cancel out to 0, but
now we get a new UTXO with a humongous amount of money, i.e. we've
actually become a central bank. We'd like to eliminate central banks,
not to become one, so it's not a very good move.

But it turns out that we can fix this. For a Pedersen commitment:

```
P = b*G + v*H
```

there's something called a range proof that even though we don't
reveal 'v' we can prove that it is within a range of values,
e.g.  [0...1000000] (= only a positive number.)

And it gets better. There's an invention called Bulletproof range
proofs, that yields in 674 bytes chunk of data for each range
proof. So to summarize, a transaction needs:

1. The new UTXO set and what UTXOs that got spent.
2. The signatures and the corresponding public keys
   (in this example `(4711-5)*G` and `(123-17)*G` respectively.)
3. Range proofs for each new UTXO (in our example we need 3 range proofs.)

### Pruning

Imgaine we have a transaction:

P1 -> P2 -> P3

The only thing that needs to be published for P2 are the proofs for
H=0, i.e. the signatures and the public keys `(4711-5)*G` and
`(123-17)*G`.  Unfortunately, we can't combine these signatures.  But
imagine that times go by, and that old P2 transaction gets burried
over time:

P1 -> P2 -> P3 -> ... -> P100

and it has been 2 years since P2 were published. Then users could
publish the private keys `(4711-5)` and `(123-17)` and then anyone can
create a new combined signature `(4711-5+123-17)` with public key
`(4711-5+123-17)*G` and everybody can still confirm it's the same
excess value because `(4711-5+123-17)*G` is still the difference
between Outputs - Inputs. Now imagine that we do this for entire
blocks of transactions, and viola, we got a pruned blockchain. And yet
it fully validates for monetary inflation. Unfortunately with a small
compromise to privacy.

For this to work in practice we need an economic incentive to publish
old spent private keys. If we'd lock 50% of miners reward for a block,
and the other half is collectable as a linear function of its base
size. For example, if everything is replaced with a single proof you'd
collect the other 50%. A miner would then need to provide a proof that
he can do block compression. This part is still something I'm thinking
about how do get this right. Sorry for not being more precise at this
time.

## Adding Prolog Predicates

In Mimblewimble, instead of signing the empty string, we can let the
new UTXO be associated with a Prolog program, e.g.

```
p(Signature, output(Output)) :-
     member(PubKey, [<hardcoded list of public keys>]),
     check_signature(dummy(Output),Signature,PubKey).
```

i.e. we use this as the base for the signature instead of the empty
string.

p(Signature, output(Output)) is true iff

* The provided signature validates the message "dummy(Output)". I added
  the "dummy" functor to show that you can write an arbitrary term here
  whose contents will be serialized for signature validation.
* The signature can be validated using one of the public keys from the
  hard coded list of public keys, so it serves as a "1 of M" signature.
  Of course, this is inefficient compared to a Schnorr signature,
   but this is only for illustration.
* The last argument, wrapped in a functor output(Output),
  becomes the blinding factor of the new UTXO. That functor acts as
  meta knowledge to the system to identify that its argument will become
  the new UTXO.
* We need to incorporate Output in the signature to prevent it from being
  malleable; an eavesdropper should not be able to rewrite
  this transaction and send the funds to a different destination.

To spend this new UTXO you need to know both its (Mimblewimble)
private key and make the predicate evaluate to true (by providing some
call/instance when spending it.)  Once the new UTXO is on the
blockchain the user can simply reveal its Mimblewimble private
key. From this point, anyone who can make that predicate evaluate to
true can spend that UTXO. That is, we've gone from a simple
Mimblewimble transaction to a smart contract. Unfortunately with some
loss of privacy due to the revelation of the private key. However,
depending on type of smart contract, that Mimblewimble private key
could also circulate within a smaller group of people.

If we know the Mimblewimble private key we can now spend that UTXO
if we attach a predicate call:

```
p(<signature>, <new output>)
```

This basically enables basic bitcoin smart contracts. But it's still
not possible to enforce rules over a longer period of time.  So let's
enhance this a bit further.

Suppose a UTXO smart contract consists of three sections:

1. The predicate that needs to evaluate to true for you to spend it.
   (What we've got at this point.)
2. Put contraints on what the smart contract of the new UTXO must look like.
3. A condition that allows a user to terminate the smart contract (so that
   (2) is no longer necessary.)

Let's say we have:

```
In order to spend:

p(Signature, state(State), output(Output)) :-
     tail(State, T), T = [s | _],
     member(PubKey, [<hardcoded list of public keys>]),
     check_signature(dummy(Output),Signature,PubKey).
tail(X,X) :- var(X).
tail([_|Xs],T) :- tail(Xs,T).

New contract must be:

p(Signature, state(State), output(Output1)) :-
     tail(State, T), T = [s | _],
     ... % Here you can add arbitrary code

Or choose to terminate the contract, but then:

p(Signature, state(State), output(Output1)) :-
    % We don't want trailing variable get bound to the
    % empty list, so that's the reason for the double disproof.
    \+ \+ (length(State, N), N >= 100).
```

Here the variable State is shared, i.e. state(State) means that it
gets unified from the result of evaluating the inputs. If you have
multiple inputs, you'll do multiple unifications on the same
term/variable. This has the interesting behavior that State becomes a
list whose length corresponds to the accumulated length of
transactions.

The user can choose to terminate this contract (free itself from the
constraint of its new UTXO predicate) if the number of accumulated
transactions exceeds 100.

You could also enforce a contract termination (after 100 transactions)
with:

```
New contract must be:

p(Signature, state(State), output(Output1)) :-
     \+ \+ (length(State, N), N < 100),
     tail(State, T), T = [s | _],
     ... % Here you can add arbitrary code
```

You can probably come up with more interesting examples. This only
illustrates the basic infrastructure.

Once a smart contract is terminated, it has the potential to go back
to a regular Mimblewimble monetary transaction and if the smart
contract gets burried enough, then we can just prune it like any other
Mimblewimble transaction. The property we'd like to preserve is to
prove that no (violation of) inflation has taken place. Reversing a
transaction based on an old spent transaction would require an
enormous amount of proof-of-work to outcompete the current tip of the
chain, so I think trusting proof-of-work for this should be acceptable.

You may wonder how we can guarantee that the contracts terminate
(i.e. halting problem?) In this case we use a gas based solution. In
Prologcoin any Prolog program that gets executed has an execution cost
associcated with it. This cost is computed as part of the consensus
rules. So publishing the smart contract together with its expected
cost is guaranteed to terminate and anyone on the network will be able
to verify it.  The free market can provide a market price per
"execution cost unit" similar to Bitcoin's Satoshis per byte.

### Does it work?

I'm still thinking g on whether there's some hole in the above design.
It feels correct, but I may have made a fundamental design error, so
it'll take some time to digest these thoughts. However, I'll start
working on some basic primitives. The first step is to incorporate the
basic elliptic curvature functionality in my Prolog implementation,
and design the basic Mimblewimble transactions.

## Still a 100% bitcoin maximalist

I'm still a 100% hardcore bitcoin maximalist. This is just another
awesome alt-coin for fun only. It might make sense just to get some in
case it catches on.


