% Meta: fileio on
% Meta: WAM-only

%
% Encryption
%

?- ec:encrypt(hello(world(42)), "mypassword", 2048, Encrypted).
% Expect: Encrypted = encrypt(58'39fj6wK3XqLUJcvq6nCRV5mAjQgXR3tzVCA9xSDviUQWTyUJQAMY3EaNLCHbx5w5mHuHthGSXhcnruFiWqTkEUXEJ6XC364HZd6QZCwCqhaY9U).

?- ec:encrypt(encrypt(58'39fj6wK3XqLUJcvq6nCRV5mAjQgXR3tzVCA9xSDviUQWTyUJQAMY3EaNLCHbx5w5mHuHthGSXhcnruFiWqTkEUXEJ6XC364HZd6QZCwCqhaY9U), "mypassword", 2048, Decrypted).
% Expect: Decrypted = hello(world(42))

?- \+ ec:encrypt(encrypt(58'39fj6wK3XqLUJcvq6nCRV5mAjQgXR3tzVCA9xSDviUQWTyUJQAMY3EaNLCHbx5w5mHuHthGSXhcnruFiWqTkEUXEJ6XC364HZd6QZCwCqhaY9U), "mypassword", 2049, _).
% Expect: true

?- \+ ec:encrypt(encrypt(58'39fj6wK3XqLUJcvq6nCRV5mAjQgXR3tzVCA9xSDviUQWTyUJQAMY3EaNLCHbx5w5mHuHthGSXhcnruFiWqTkEUXEJ6XC364HZd6QZCwCqhaY9U), "mypassworx", 2048, _).
% Expect: true

?- \+ ec:encrypt(encrypt(58'39fj6wK3XqLUJcvq6nCRV5mAjQgXR3tzVCA9xSDviUQWTyUJQAMY3EaNLCHbx5w5mHuHthGSXhcnruFiWqTkEUXEJ6XC364HZd6QZCwCqhax9U), "mypassworx", 2048, _).
% Expect: true

?- ec:encrypt([letter,advice,cage,absurd,amount,doctor,acoustic,avoid,letter,advice,cage,above],"TREZOR",2048,Encrypted).
% Expect: Encrypted = encrypt(58'AZLhA4VruJuN5JqYCc9H2Hx4q88WqC2Tm9XNSjCvR4x8jSfhZXVbnmsf45nwZCM46vVLQQLShBkjdmygeBboWLvEo5nDey4ttsXvR8cCDQinaQttcEFVVNYxtwzse59G3utV5Qh3SKtkzQzkW4TG7uxfUn1Z7i9dBGE7M7eQwKf7fWKe75HiXZhmQtrsiSs3x3oeoMXmdzSEsfSxAVMJ4L4QUinZ4tjbT8Z8nwY3BFx8Sa47FusAzaTYf1dmfRRuLJS8cwStd2L94L4Va6Ku7peVe1fKcxVYTjDEBgJhVnjWLjwNKy1F1UvreUe8Agvyt7SKFn5neJNbLmwYMwKwzGtRoSSJU5xNeqnTPXfbxZG78yyrRJbGTopHGRjuEcSLBpxZghn2LtEzjEMpk358KXfFVFVAVtT32EE2aLYT1Pmf4Ajz7hJkoY8wEu4juPDPAu4Gurq5eVsQpHsZcJSsPdrxGT91qJvyHaj7x3AqV4AzEbrMeZFmNTP)

?- ec:encrypt(encrypt(58'AZLhA4VruJuN5JqYCc9H2Hx4q88WqC2Tm9XNSjCvR4x8jSfhZXVbnmsf45nwZCM46vVLQQLShBkjdmygeBboWLvEo5nDey4ttsXvR8cCDQinaQttcEFVVNYxtwzse59G3utV5Qh3SKtkzQzkW4TG7uxfUn1Z7i9dBGE7M7eQwKf7fWKe75HiXZhmQtrsiSs3x3oeoMXmdzSEsfSxAVMJ4L4QUinZ4tjbT8Z8nwY3BFx8Sa47FusAzaTYf1dmfRRuLJS8cwStd2L94L4Va6Ku7peVe1fKcxVYTjDEBgJhVnjWLjwNKy1F1UvreUe8Agvyt7SKFn5neJNbLmwYMwKwzGtRoSSJU5xNeqnTPXfbxZG78yyrRJbGTopHGRjuEcSLBpxZghn2LtEzjEMpk358KXfFVFVAVtT32EE2aLYT1Pmf4Ajz7hJkoY8wEu4juPDPAu4Gurq5eVsQpHsZcJSsPdrxGT91qJvyHaj7x3AqV4AzEbrMeZFmNTP),"TREZOR", 2048, WordList).
% Expect: WordList = [letter,advice,cage,absurd,amount,doctor,acoustic,avoid,letter,advice,cage,above]
