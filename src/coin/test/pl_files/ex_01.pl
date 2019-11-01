%
% reward/2
%
% Make sure the reward at genesis is 42445243724242
%
?- reward(0, Q1).
% Expect: Q1 = '$coin'(42445243724242, _).

% Make sure next height is the standard one
?- reward(1, Q2).
% Expect: Q2 = '$coin'(21000000000, _).

% Track halvening
?- reward(99999, Q3), reward(100000, Q4).
% Expect: Q3 = '$coin'(21000000000, _), Q4 = '$coin'(10500000000, _).

% Track next halvening
?- reward(100001, Q5), reward(200000, Q6).
% Expect: Q5 = '$coin'(10500000000, _), Q6 = '$coin'(5250000000, _).

