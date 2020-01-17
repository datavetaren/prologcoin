% Meta: fileio on
% Meta: WAM-only

%
% Test hierachical determinic key generation from word list (BIP39)
%
% Test vectors from BIP39 specification.
%

%
% Test vector 1
%

?- ec:master_key([abandon,abandon,abandon,abandon,abandon,abandon,abandon,abandon,abandon,abandon,abandon,about],"TREZOR",XPriv,_).
% Expect: XPriv = 58'xprv9s21ZrQH143K3h3fDYiay8mocZ3afhfULfb5GX8kCBdno77K4HiA15Tg23wpbeF1pLfs1c5SPmYHrEpTuuRhxMwvKDwqdKiGJS9XFKzUsAF
% Expect: end

?- ec:master_key([legal,winner,thank,year,wave,sausage,worth,useful,legal,winner,thank,yellow],"TREZOR",XPriv,_).
% Expect: XPriv = 58'xprv9s21ZrQH143K2gA81bYFHqU68xz1cX2APaSq5tt6MFSLeXnCKV1RVUJt9FWNTbrrryem4ZckN8k4Ls1H6nwdvDTvnV7zEXs2HgPezuVccsq
% Expect: end

?- ec:master_key([letter,advice,cage,absurd,amount,doctor,acoustic,avoid,letter,advice,cage,above],"TREZOR",XPriv,_).
% Expect: XPriv = 58'xprv9s21ZrQH143K2shfP28KM3nr5Ap1SXjz8gc2rAqqMEynmjt6o1qboCDpxckqXavCwdnYds6yBHZGKHv7ef2eTXy461PXUjBFQg6PrwY4Gzq

?- ec:master_key([zoo,zoo,zoo,zoo,zoo,zoo,zoo,zoo,zoo,zoo,zoo,wrong],"TREZOR",XPriv,_).
% Expect: XPriv = 58'xprv9s21ZrQH143K2V4oox4M8Zmhi2Fjx5XK4Lf7GKRvPSgydU3mjZuKGCTg7UPiBUD7ydVPvSLtg9hjp7MQTYsW67rZHAXeccqYqrsx8LcXnyd

?- ec:master_key([abandon,abandon,abandon,abandon,abandon,abandon,abandon,abandon,abandon,abandon,abandon,abandon,abandon,abandon,abandon,abandon,abandon,agent],"TREZOR",XPriv,_).
% Expect: XPriv = 58'xprv9s21ZrQH143K3mEDrypcZ2usWqFgzKB6jBBx9B6GfC7fu26X6hPRzVjzkqkPvDqp6g5eypdk6cyhGnBngbjeHTe4LsuLG1cCmKJka5SMkmU

?- ec:master_key([ozone,drill,grab,fiber,curtain,grace,pudding,thank,cruise,elder,eight,picnic],"TREZOR",XPriv,_).
% Expect: XPriv = 58'xprv9s21ZrQH143K2oZ9stBYpoaZ2ktHj7jLz7iMqpgg1En8kKFTXJHsjxry1JbKH19YrDTicVwKPehFKTbmaxgVEc5TpHdS1aYhB2s9aFJBeJH
    
?- ec:master_key([panda,eyebrow,bullet,gorilla,call,smoke,muffin,taste,mesh,discover,soft,ostrich,alcohol,speed,nation,flash,devote,level,hobby,quick,inner,drive,ghost,inside],"TREZOR",XPriv,_).
% Expect: XPriv = 58'xprv9s21ZrQH143K2WNnKmssvZYM96VAr47iHUQUTUyUXH3sAGNjhJANddnhw3i3y3pBbRAVk5M5qUGFr4rHbEWwXgX4qrvrceifCYQJbbFDems

    

