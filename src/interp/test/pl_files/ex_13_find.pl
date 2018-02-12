%
% Find
%

replace_state([state(N,_,_)|States], state(N,KernelItems,Actions),
	      [state(N,KernelItems,Actions)|States]).
replace_state([State|States], UpdatedState, [State|NewStates]) :-
    replace_state(States, UpdatedState, NewStates).

get_states([state(0, [(start :- 'DOT', subterm_1200, full_stop)], []), state(1, [(start :- 'DOT', 'DOT')], [])]).

runit(X) :-
    get_states(S),
    R=state(0, [(start :- 'DOT', subterm_1200, full_stop)], [shift('DOT', 1)]),
    replace_state(S,R,X).

%replace([nv(N,_)|Xs],nv(N,V),[nv(N,V)|Xs]).
%replace([nv(N1,V1)|Xs],nv(N,V),[nv(N1,V1)|Ys]) :-
%    replace(Xs,nv(N,V),Ys).
%
%runit(X) :-
%    A = [nv(0,foo),nv(1,bar)],
%    replace(A,nv(0,replaced),X).

?- runit(Q1).
% Expect: Q1 = [state(0, [(start :- 'DOT', subterm_1200, full_stop)], [shift('DOT', 1)]), state(1, [(start :- 'DOT', 'DOT')], [])]
% Expect: end
