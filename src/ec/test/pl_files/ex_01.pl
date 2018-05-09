%
% ec:privkey/1
%
% This key was generated from bitaddress.org
%
?- ec:privkey(58'L326Y3N3XHcGWSnhiTPZTb544aGZt6x8sTfLpnWKwoeLr3NWghct).
% Expect: true

%
% Make sure it matches the bitcoin address given by bitaddress.org
%

?- ec:pubkey(58'L326Y3N3XHcGWSnhiTPZTb544aGZt6x8sTfLpnWKwoeLr3NWghct, X), ec:address(X, Y).
% Expect: X = 58'1semYvast3hpYyTLioxNqwwL9WNXJqfrRqUJ5xurzeMrV, Y = 58'1LxVHoMAuAJbznVT7gjdyT1of42fGCrZNZ.
