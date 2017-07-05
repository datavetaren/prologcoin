# Prologcoin (working title)

The purpose of this cryptocurrency is to combine MimbleWimble (the
only known scalable technology for cryptocurrencies) with the
execution engine of Prolog (a logic programming language.)

## What is Prolog

Prolog is a language based on predicates (~= program) and terms (~=
data structures.) Prolog is basically all about "equation solving on
data structures".)

## Why Prolog?

Blockchains are about synchronizing a state among N machines (nodes)
over a non-trusted network. The state is proven to be valid using
various axioms/rules (also known as consensus.) Words such as
"proving," "validation," "axioms," and "logic" fits the Prolog
paradigm very well.

Prolog also has a pecular concept called "logic variables." The word
"logic" should not be confused with true/false, but rather that these
variablescan be bound (to terms) in order to solve a "data structure"
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
stuff you wan't to prove to be true") as:

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
legacy we're just inheriting. "[A|B]" is list construction (where A is
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

The binding of variables in Prolog is particularly interesting for
cryptocurrencies. The concept of an UTXO (unspent transaction output)
can be seen as a variable in an unbound state. Once the variable is
bound, the coin is spent.

By the use of MimbleWimble Pedersen commits, we can define:

```
p(<some UTXO>, <blinding factor>, ValidatedValue)
```

and whenever we want to spend the coin we just bind ValidatedValue to
something that proves we own it. If all variables are named (and have
unique references), then just referring to a prior variable and bind
it to a proper value will spend it. If the heap is based on 64-bit
cells, we can use the address space of 2^64. Referring to an UTXO (a
prior variable) will just be a 64-bit number.

## The State

We'll use an never ending query to define "the blockchain." Each goal
in the query ("a.k.a sub-query") could basically represent a
transaction. We bundle the sub-queries in blocks, so that we can
bundle many sub-queries together and order them in time. We'll use
traditional proof-of-work mining to ensure an immutable ordering
sequence.

Note that MimbleWimble doesn't require you to memorize all preceding
sub-queries. You can randomly select the sub-queries you're interested
in and validate those only. This is the reason why MimbleWimble is
referred as the only scalable cryptocurrency.

While the query (and all goals / sub-queries) are evaluated, a "heap"
is constructed. You can view the heap as the memory state. The heap is
keeping all the terms together (as data structures.)

Prolog has also a concept of back-tracking which is similar to
"exception handling" in traditional languages. This amounts to go back
in time and undo things. Again, this is a perfect fit for the so
called reorganization problem that also occurs in blockchains.

## Smart Contracts?

I believe it should be possible to combine MimbleWimble with Prolog
smart contracts. Imagine you have the Pedersen commit:

```
p(Utxo,TransactionKernel,...,Predicate)
```

and you commit it to the blockchain and its meaning is:

"If the blinding factor AND the Predicate evaluate to true, then you
can spend it."

Once the Pedersen commit is in the blockchain (represented as a Prolog
term,) you'll now reveal the blinding factor to the world which means
anyone can spend it that can make Predicate become true. It feels that
this is a viable path forward and it still preserves the transaction
cut-through optimizations as we don't really care about spent coins,
so there's no need to validate old smart contracts.

Bear in mind as I'm not 100% sure about this. But it feels doable.

## Summary

The goal is to implement a cool piece of technology. I have no idea
whether it'll work or not. Implementing it is a way to find out for
sure.

Watch my progress. Once I'm getting close to the end game then we'll
see some action.

## Still a 100% bitcoin maximalist

I'm still a 100% hardcore bitcoin maximalist. We can always get this
as a sidechain. Or perhaps as a "half-sidechain" (50% of the currency
is produced by the blockchain itself, and the other 50% as locked in
bitcoin.) E.g. a total supply of 21 million Prologcoin and 21 million
bitcoin.

Let's change the world.


