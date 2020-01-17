# Wallet structure

My main idea on how wallets should work:

* The wallet file should be a standard Prolog source file (with .pl extension.)
* The user wallet source file should reside in a module 'wallet', i.e.
  :- module(wallet).
* The API is based on callbacks.

## Key management

* wallet:pubkey(Count, X) X is the public key X using integer Count as identifier
* wallet:numkeys(N) true iff N is the next unused public key identifier. All integers up to N must be valid public keys (i.e. wallet:pubkey(Count, X) should be true for all Count = 0..N-1).
* wallet:privkey(Count, PrivKey) is true iff PrivKey is the private key to use for corresponding PubKey with the same Count.

This allows the user to write a fixed list of facts:

```
:- module(wallet).

pubkey(0, 58'1rSkmhHnLhEn3GKeHWAandkzu2f3Pr9uiJoGdZf39XCJS).
pubkey(1, 58'21hWcGBmPDcML5mcH9PTsJ2cZudoqfW1t99TQwZtmTcpj).
pubkey(2, 58'23WqwSUDKqjHozZusdwk1HXog7ZCo9gS43n7UD5AHgdSu).
...

privkey(0, 58'L2qVN5awRihJN1su4hQMA31TyCmiNdbx8vCrQe4J15AxKa5tpaet).
privkey(1, 58'KxG93NUJHZvtDgWzftn95yLsFkuPvHhJdq33z7b4UUbTP9dS39dr).
privkey(2, 58'L5RMLpiwpFf83gqE1Da9RTG55ctkpzNQqHxE4Qrtt3eBbksCNYtE).
...

numkeys(3).
```

But it is of course also possible to use a more dynamic approach (BIP32+BIP39):

```
:- module(wallet).
pubkey(Count, PubKey) :- ec:master_key([legal,winner,thank,year,wave,sausage,worth,useful,legal,winner,thank,yellow], _, MasterPublic), ec:child_pubkey(MasterPublic, m/Count, ExtPubKey), ec:extkey(ExtPubKey, PubKey).

privkey(Count, PrivKey) :- ec:master_key([legal,winner,thank,year,wave,sausage,worth,useful,legal,winner,thank,yellow], MasterPrivate, _), ec:child_privkey(MasterPrivate, m/Count, ExtPrivKey), ec:extkey(ExtPrivKey, PrivKey).

numkeys(3).
```

To avoid having the sentence in plain text it could also be encrypted:

```
:- module(wallet).
pubkey(Count, PubKey) :- ec:child_pubkey(58'xpub..., ExtPubKey), ec:extkey(ExtPubKey, PubKey).

privkey(Count, PrivKey) :-
    wallet_memcache:password(Password),
    ec:encrypt(encrypted(58'<big constant>), Password, 10000, WordList),
    ec:master_key(WordList, MasterPrivate, _),
    ec:child_key(MasterPrivate, m/Count, ExtPrivKey),
    ec:extkey(ExyPrivKey, PrivKey).
```

To unlock (in memory only) this wallet so that transactions can be
signed you'll do:

```
assert(wallet_memcache:password("my password")),
... use private key ... ,
retract(wallet_memcache:password(_)).
```

Note how extremely flexible and elegant this is.

## UTXOs

Whenever a UTXO (frozen closure) is discovered it'll ask the wallet if
it recognizes it:

wallet:utxo(Closure).

The wallet can also keep track over all relevant UTXOs by listing them as facts:

utxo(HeapId, UTXOType, Value, PubKeys).

This good for the spending algorithm that can then consume one or more
UTXOs.

For example:

```
utxo(12383, tx1, 10005012, [58'1rSkmhHnLhEn3GKeHWAandkzu2f3Pr9uiJoGdZf39XCJS]).
utxo(94822, tx1, 5030210, [58'21hWcGBmPDcML5mcH9PTsJ2cZudoqfW1t99TQwZtmTcpj]).
utxo(123943, tx1, 100000, [58'23WqwSUDKqjHozZusdwk1HXog7ZCo9gS43n7UD5AHgdSu]).

utxo(Closure) :-
     ... check transaction type of Closure
     ... check public key
     assert(utxo(HeapId, tx1, Value, PubKey)).
```

Nothing stops the user from implementing more sophisticated
transaction types.










