%
% ec:pproof & ec:pverify
%
?- B = 58'L326Y3N3XHcGWSnhiTPZTb544aGZt6x8sTfLpnWKwoeLr3NWghct, V = 1234, ec:pproof(B,V,P), ec:pverify(P).
% Expect: true/*
