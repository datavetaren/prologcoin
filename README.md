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

### Pedersen commitments

A Pedersen commitment is an arithmetic computation over an elliptic
curve (EC) such as:

```
P = b*G + v*H
```

where 'b' and 'v' are plugged in parameters. G is a generator for a
elliptic curve, a precomputed constant. H is just another
generator on the same curve that is guaranteed to be "numerically
disjoint" (easily achieved with something like H = hash(G).)

We say that 'b' is a blinding factor. The 'b' serves as a private key.
If we imagine that P (which is just a 256-bit big number) is an UTXO,
then we can prove we own that UTXO, even though the rest of the world
don't know the value of 'v'.

The owner simply does `P - v*H`, because the owner knows 'v', he can
easily compute v*H, and then the owner signs a message with his
private key 'b' and everybody can confirm the signature using `P - v*H`
as the public key. The message to be signed can be any 256-bit number,
e.g. the SHA256 hash of the empty string (or whatever.)

### Double Communication. Unfortunately.

For Mimblewimble transactions we'll see that, unfortunately, there
needs to be a direct connection between sender and recipient of funds.

What will become apparent later is that the owner of an UTXO needs to
split his private key into a sum subset. For example, the sender needs
to be able to rewrite:

```
P = b*G + v*H = (b1+b2+...+bn)*G + (v1+v2+...+vn)*H 
  = b1*G + b2*G + ... + bn*G + v1*H + v2*H + ... + vn*H
```

Some of these bi's (and vi's) need to be sent to the recipient, which
reacts to those and will reply with some additional information.

### Transactions

In Mimblewimble we identify what UTXOs we'd like to spend (each UTXO
is a Pedersen commitment) and then we identify one (or more)
recipients of funds. What we'll do is to compute the difference:

Outputs - Inputs

Where Inputs are just the sum of all UTXOs. And outputs are also a sum
of all new UTXOs. Outputs = Inputs, so the difference becomes exactly
0.

Let's say we have Alice who owns:

```
P = 42*G + 8*H
```

(here 42 is normally a ridiciously large number, which will serve as
the private key for Alice.)

Now she wants to send 3 units to Bob and 4 units to Carol (and keep 1
unit to herself.)  She'll split her 42 into 3 random numbers:

```
20 + 5 + 17 = 42
```

This means that:

```
P = 20*G + 1*H + 5*G + 3*H + 17*G + 4*H ( = 42*G + 8*H)
```

Bob wants his UTXO in `4711*G + 3*H`. So Alice gives Bob 5 (one number
of the splitted sum.) Bob then computes `4711*G+3*H` and `(4711-5)*G`
and gives both these numbers back to Alice, who know just replaces:
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

How can the rest of the world verify that...

1) No new money has been created?
2) That money wasn't stolen?

So let's split the above P into three parts:

```
P1 = 20*G + 1*H
P2 = (4711*G+3*H) - (4711-5)*G
P3 = (123*G+4*H) - (123-17)*G
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

```
P1'+P2'+P3' - P = x*G + 0*H
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
not to become one, so not a very good move.

But it turns out that we can fix this. For a Pedersen commitment:

```
P = b*G + v*H
```

there's something called a range proof that even though we don't
reveal 'v' we can prove that it is within a range of values,
e.g.  [0...1000000] (= only a positive number.)

And it gets better. There's an invention called Bulletproof range
proof, that yields in 674 bytes chunk of data. So to summarize, a
transaction needs:

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
`(4711-5+123-17)*G` and everybody can still confirm it's the same excess
value because `(4711-5+123-17)*G` is still the difference between
Outputs - Inputs. Now imagine that we do this for entire blocks of
transactions, and viola, we got a pruned blockchain. And yet it fully
validates. Unfortunately with a small compromise to privacy.

## TODO

Will talk about how to add Prolog code together with the Mimblewimble transactions...

## Still a 100% bitcoin maximalist

I'm still a 100% hardcore bitcoin maximalist. This is just another
awesome alt-coin for fun only. It might make sense just to get some in
case it catches on.


