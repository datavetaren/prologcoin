% Aggregate all rewards
%
% Written so that tail recursion optimization kicks in.
%

%
% Standard loop with rewards at height >= 100000
totalsupply1(Height, Acc, Tot, LastHeight) :-
    reward(Height, Coin), Coin =.. ['$coin',Value,_],
    totalsupply2(Value, Height, Acc, Tot, LastHeight).

totalsupply2(0, Height, Acc, Acc, Height).
totalsupply2(Value, Height, Acc, Tot, LastHeight) :-
    Acc1 is Acc + Value*100000,
    Height1 is Height + 100000,
    totalsupply1(Height1, Acc1, Tot, LastHeight).

%
% Add initial reward and rewards from 1..99999 followed by the
% above standard loop.
%
totalsupply(Tot, LastHeight) :-
    reward(0, InitCoin), InitCoin =.. ['$coin',InitValue,_],
    reward(1, Standard), Standard =.. ['$coin',StandardValue,_],
    totalsupply1(100000, 0, Tot1, LastHeight),
    Tot is InitValue + StandardValue*99999 + Tot1.

?- totalsupply(MoneySupply, LastHeight).
% Meta: WAM-only
% Expect: MoneySupply = 4242424242424242, LastHeight = 3500000.
