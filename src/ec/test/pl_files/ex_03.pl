%
% Test MuSig
%
pkey1(58'L326Y3N3XHcGWSnhiTPZTb544aGZt6x8sTfLpnWKwoeLr3NWghct).
pkey2(58'KzC9JAPZfcsmzU1EF3yqs39a4dq7jUYXCLAMhQTGBwrbXeSJhgsi).
pkey3(58'Kwd33XZRL1qSSTaxdhPSKF8xtg5DbBpfuq6NhQxbwuD7DoEEmF1X).

combiner(PubKeyCombined, PubKeyCombinedHash) :-
    pkey1(X1), pkey2(X2), pkey3(X3),
    ec:pubkey(X1,P1), ec:pubkey(X2,P2), ec:pubkey(X3,P3),
    ec:musig_combine([P1,P2,P3],PubKeyCombined,PubKeyCombinedHash).
    
?- combiner(PubKey, PubKeyHash).
% Expect: PubKey = 58'1uvCiduRL5GbS25LkrefndgjWbUjsk6f9EJpMYEPN1Ruu, PubKeyHash = 58'AhNPU9P3s3MNsZqSYTsJ1Skyf1Sy4gT4E8eoh7ZWYLVx.
% Expect: end
