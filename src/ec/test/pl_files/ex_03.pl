%
% Note that perfect randomness is disabled to get deterministic behavior.
%

% Meta: fileio on


%
% Test MuSig Combine
%
pkey1(58'L326Y3N3XHcGWSnhiTPZTb544aGZt6x8sTfLpnWKwoeLr3NWghct).
pkey2(58'KzC9JAPZfcsmzU1EF3yqs39a4dq7jUYXCLAMhQTGBwrbXeSJhgsi).
pkey3(58'Kwd33XZRL1qSSTaxdhPSKF8xtg5DbBpfuq6NhQxbwuD7DoEEmF1X).

akey(Secret) :-
    String = "01 this is a secret! 12345678901",
    bytes_number(String, Secret).

get_public_keys([P1,P2,P3]) :-
    pkey1(X1), ec:pubkey(X1,P1), 
    pkey2(X2), ec:pubkey(X2,P2),
    pkey3(X3), ec:pubkey(X3,P3).
    

combiner(PubKeyCombined, PubKeyCombinedHash) :-
    get_public_keys(PubKeys),
    ec:musig_combine(PubKeys,PubKeyCombined,PubKeyCombinedHash).
    
?- combiner(PubKey, PubKeyHash).
% Expect: PubKey = 58'1uvCiduRL5GbS25LkrefndgjWbUjsk6f9EJpMYEPN1Ruu, PubKeyHash = 58'AhNPU9P3s3MNsZqSYTsJ1Skyf1Sy4gT4E8eoh7ZWYLVx.
% Expect: end

%
% Test MuSig Session
%

session1(Session) :-
   combiner(CombPubKey, CombPubKeyHash),
   pkey1(X1),
   ec:musig_start(Session, CombPubKey, CombPubKeyHash, 0, 3, X1, hello(world(42))).
session2(Session) :-
   combiner(CombPubKey, CombPubKeyHash),
   pkey2(X2),
   ec:musig_start(Session, CombPubKey, CombPubKeyHash, 1, 3, X2, hello(world(42))).
session3(Session) :-
   combiner(CombPubKey, CombPubKeyHash),
   pkey3(X3),
   ec:musig_start(Session, CombPubKey, CombPubKeyHash, 2, 3, X3, hello(world(42))).


?- session1(Session1), session2(Session2), session3(Session3).
% Only run this with WAM to avoid second round of  sessions being created.
% Meta: WAM-only
% Expect: Session1 = '$musig'(1), Session2 = '$musig'(2), Session3 = '$musig'(3).
% Expect: end

%
% Get commitments
%

commitments([C1,C2,C3]) :-
    ec:musig_nonce_commit('$musig'(1), C1),
    ec:musig_nonce_commit('$musig'(2), C2),
    ec:musig_nonce_commit('$musig'(3), C3).

?- commitments([C1,C2,C3]).
% Expect: C1 = 58'8YTqNERgggcngz7pKQGk7cEydnTGVdFBSovBn248ndFn, C2 = 58'Hu5QpcUNU4wKCLteFRhistYhg39BCXMPkrwmULVRC4je, C3 = 58'3Qq45KwNwqfvoNjNcjMY6jNcfidgRkedjQSuGZLZdFqL.
% Expect: end

%
% get nonce
%

nonces([N1,N2,N3]) :-
    commitments(Cs),
    ec:musig_prepare('$musig'(1), Cs, N1),
    ec:musig_prepare('$musig'(2), Cs, N2),
    ec:musig_prepare('$musig'(3), Cs, N3).

?- nonces([N1,N2,N3]).
% Expect: N1 = 58'24t5Gk7aR5VCHr2z3HSqA9ih9EmGrgU4RP8FHvf2ZMHd4, N2 = 58'27R5ovLdMGJd19tUaGo3fpvBDj2ha23jMc2Yfubdnwtbv, N3 = 58'1u2CUx97TSSLidX8jiTBY9KTDemPF2JhaaamWNA2cUo8E.
% Expect: end

%
% Combine nonces
%

combine_nonces :-
    nonces(Ns),
    ec:musig_nonces('$musig'(1), Ns),
    ec:musig_nonces('$musig'(2), Ns),
    ec:musig_nonces('$musig'(3), Ns).

?- combine_nonces.
% Expect: true

set_public_keys :-
    get_public_keys([P1,P2,P3]),
    ec:musig_set_public_key('$musig'(1), 0, P1),
    ec:musig_set_public_key('$musig'(2), 1, P2),
    ec:musig_set_public_key('$musig'(3), 2, P3).
?- set_public_keys.
% Expect: true

%
% Partial signatures
%

partial_sigs([Sig1,Sig2,Sig3]) :-
    ec:musig_partial_sign('$musig'(1), Sig1),
    ec:musig_partial_sign('$musig'(2), Sig2),
    ec:musig_partial_sign('$musig'(3), Sig3).

?- partial_sigs([Sig1,Sig2,Sig3]).
% Expect: Sig1 = 58'6sHsrBVyxAdD1G13AVVL3JxbK784HJ7iS5iDrTPVEeDh, Sig2 = 58'Dcjcv2HiWyjoazfeexL8eE6JMfs7W2u3icWuiurj2yp2, Sig3 = 58'FjkF6hvsXAdgoiype63edY7t3Q7ASZCYF4JgSTZY4QRp.
% Expect: end

final_sigs([Fin1,Fin2,Fin3]) :-
	partial_sigs(Sigs),
	ec:musig_final_sign('$musig'(1), Sigs, Fin1),
	ec:musig_final_sign('$musig'(2), Sigs, Fin2),
	ec:musig_final_sign('$musig'(3), Sigs, Fin3).

?- final_sigs([Fin,Fin,Fin]).
% Expect: Fin =  58'3Z5hXQPQMM6QYGZJFsFG3e44bKZr8cTpCoWhJF1rX9UpRZs65j3bZN8DjGWZzrtwMHWNr9CBRkjWEyA3rjNWJAF6.
% Expect: end

%
% Let's that the signature verifies with combined public key and the provided data.
%
?- ec:musig_verify(hello(world(42)),
	           58'1uvCiduRL5GbS25LkrefndgjWbUjsk6f9EJpMYEPN1Ruu,
	           58'3Z5hXQPQMM6QYGZJFsFG3e44bKZr8cTpCoWhJF1rX9UpRZs65j3bZN8DjGWZzrtwMHWNr9CBRkjWEyA3rjNWJAF6).
% Expect: true

%
% Try with adaptor
%

%
% New MuSig Sessions
%

session4(Session) :-
   combiner(CombPubKey, CombPubKeyHash),
   pkey1(X1),
   ec:musig_start(Session, CombPubKey, CombPubKeyHash, 0, 3, X1, hello(world(42))).
session5(Session) :-
   combiner(CombPubKey, CombPubKeyHash),
   pkey2(X2),
   ec:musig_start(Session, CombPubKey, CombPubKeyHash, 1, 3, X2, hello(world(42))).
session6(Session) :-
   combiner(CombPubKey, CombPubKeyHash),
   pkey3(X3),
   ec:musig_start(Session, CombPubKey, CombPubKeyHash, 2, 3, X3, hello(world(42))).

adaptor_sig(Sigs, Fin, Negated, Nonces) :-
    session4(Session4),
    session5(Session5),
    session6(Session6),
    ec:musig_nonce_commit(Session4, C4),
    ec:musig_nonce_commit(Session5, C5),
    ec:musig_nonce_commit(Session6, C6),
    Cs = [C4,C5,C6],
    ec:musig_prepare(Session4, Cs, N1),
    ec:musig_prepare(Session5, Cs, N2),
    ec:musig_prepare(Session6, Cs, N3),
    Ns = [N1,N2,N3],
    akey(AdaptorSecret), ec:pubkey(AdaptorSecret, Adaptor),
    ec:musig_nonces(Session4, Ns, Adaptor),
    ec:musig_nonces(Session5, Ns, Adaptor),
    ec:musig_nonces(Session6, Ns, Adaptor),
    ec:musig_partial_sign(Session4, Sig1),
    ec:musig_partial_sign(Session5, Sig2),
    ec:musig_partial_sign(Session6, Sig3),
    Sigs = [Sig1,Sig2,Sig3],
    ec:musig_partial_sign_adapt(Session5, Sig2, AdaptorSecret, ASig2),
    ASigs = [Sig1,ASig2,Sig3],
    ec:musig_final_sign(Session4, ASigs, Fin),
    ec:musig_final_sign(Session5, ASigs, Fin),
    ec:musig_final_sign(Session6, ASigs, Fin),
    ec:musig_nonce_negated(Session5, Negated),
    Nonces = Ns.

verify_adaptor_sig(Fin, Negated, Secret) :-
    write('Compute adaptor signature.'), nl,
    combiner(CombinedPubKey, _),
    adaptor_sig(Sigs, Fin, Negated, Nonces),
    write('Verify adaptor signature.'), nl,
    ec:musig_verify(hello(world(42)),
	            CombinedPubKey,
		    Fin),
    write('Extract secret.'), nl,
    get_public_keys(PubKeys),
    akey(AdaptorSecret), ec:pubkey(AdaptorSecret, Adaptor),
    ec:musig_secret(hello(world(42)),
                    Fin, Nonces, PubKeys, Sigs, Adaptor, SecretNum),
   bytes_number(Secret, SecretNum).

?- verify_adaptor_sig(Fin, Negated, Secret),
   write('Check if secret is correct.'), nl,
   write(Secret),
   nl,
   write('Everything is ok'), nl.
% Expect: Fin = 58'5crp8rpA3rx19D6fLsoMELfdgwKsGQb1ZAZdgrj23ngEjwnAfyc1TsLf7bErNBHBXq7WpWb21mfodiCZ57pqi9q5, Negated = false, Secret = "01 this is a secret! 12345678901"
% Expect: end

close_sessions :-
    ec:musig_end('$musig'(1)),
    ec:musig_end('$musig'(2)),
    ec:musig_end('$musig'(3)),
    ec:musig_end('$musig'(4)),
    ec:musig_end('$musig'(5)),
    ec:musig_end('$musig'(6)).

?- close_sessions.
% Expect: true
