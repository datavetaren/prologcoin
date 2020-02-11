master_privkey(Master) :-
    system : password(Password), 
    ec : 
    encrypt(WordList, Password, 2048, 
            encrypt(58'2CRoRtMZj5hJDk7fmhY2KDagPFBkQocU2MsEsvDjkywHwiTqEyMpp52ynwbtc6U34wLpGioUTBn8MPx6MoNjKipU3N3winQucSTskEgv6ZidXpn53bLs3kEfojB552KNRZB7B2wMLCyUZKSZdTzGbAJngeXg9UsSmxKv2kn66n1EW39Hf2vREqQB2wBbYLbLiJQ6vAzD6nFzQ5LhTiKPibeTqBTEoi7NDPWnFHLJJrCK3JSG2xwHbZVt27K28hoiLx9sXRMSv6tt2rpr5j5LKE1ftMSEXpzWpbuvmrCNemkgwAWWWrigqnASQ26V13UcnpeMrkop2oQqstPTP3YbdT6jv3GcxQKxsbQdxbRVNjSPe68Mwt9qKAhmWaaugrXUsY8CqJ8RpgUG7BXTADCEcVbVUqrjsBPqtdVUTqQTVaJvgBqtUnJp4wQHgwaED1VPDyy3pwaRR7X638bj3mowBoGbfEUzNe5Z72pSzJh1iqheyPKdgHyzRjKdfNcitR2pThS43HsozQzkcjp7futpDJWKtzFm5G1KpykhMqJSD2kYJMFfLBaV3JeW3pbACAQhJUrnWS4pvfn1XwSwYjCG4orcevedhacVpdkJGx1chxqCXphfo4GcEjwFQVUkTyw8oPPZUrZjTPm7h23Y63ywa4KCMQX3yXJu23AwiTdL3c7RT8fgadeb7JqQxwcKkn8MGiQwfa3CXzDq9P2WvUrX4kDRvPVwaP86MMnNq2Tt1Liwwthg3kjcspMJnRx8zR3rbgaoi8jMdVmFNc9w6v3pnDkgofxQD6ypQpaes538s2HeGAAKHJq6jtYSNnXjRGqgTnxpwPcHjGwGZS3uMvcDY
                    )), ec : master_key(WordList, Master, _).

master_pubkey(58'xpub661MyMwAqRbcEkwDrzAqs99ZDoZjDycpy779WReFu1PhJtPTethiXCukW71VdzzXzcmHhwbip1moNiwDxMyc25HA1ZhVmYUQpoFR5bap3Ux
              ).

pubkey(Count, PubKey) :-
    master_pubkey(Master), 
    ec : child_pubkey(Master, Count, ExtPubKey), 
    ec : normal_key(ExtPubKey, PubKey).

privkey(Count, PrivKey) :-
    master_privkey(Master), 
    ec : child_privkey(Master, Count, ExtPrivKey), 
    ec : normal_key(ExtPrivKey, PrivKey).

lastheap(1680).

numkeys(4).

utxo(1565, tx1, 21000000000, 58'1F1krUUBG9dQbLahcGbEEPsiiCffMTRs8v).
utxo(1603, tx1, 21000000000, 58'1F1krUUBG9dQbLahcGbEEPsiiCffMTRs8v).
utxo(1676, tx1, 31000000000, 58'1DgKvJZtrs8KEqWEcrPXW2a7A5TYDoYwKm).
utxo(1680, tx1, 10999968000, 58'1FnTXpFbJfsoGLLt5gvzXzT3Jy2xf35qnz).

