%, copy_term(X,Y), writeln('....'), writeln(X), writeln(Y).

:- linitialization(main).

:- op(700, xfx, :=: ).

main :-
	main( cyclic_term(X), X).
main :-
	writeln('--- cyclic_term/1 --------------------'),
	fail.
main :-
	main( ground(X), X).
main :-
	writeln('--- ground/1 ------------------'),
	fail.
main :-
	main2( (terms:variables_in_term(X, O), writeln(X=O) ), X, L, O).
main :-
	writeln('--------variables_in_term/2, writeln/1 ---------------'),
	fail.

main :-
    main2( (terms:new_variables_in_term(L,X, O), writeln(X+L=O) ), X, L, O).
main :-
	writeln('-----------------------'),
	fail.
main :-
    main2( (terms:variables_within_term(L,X, O), writeln(X+L=O) ), X, L, O).
main :-
	writeln('-----------------------'),
	fail.
main :-
    main( writeln(X), X).
main :-
	writeln('------rational_term_to_tree(X,A,B,[]),\
	      writeln((A->B) -----------------'),
	fail.
main :-
	main((rational_term_to_tree(X,A,B,[]),
	      writeln((A->B))), X).
main :-
	writeln('------ numbervars(A+B,1,_),\
	      writeln((A->B) -----------------'),
	fail.
main :-
	main(( numbervars(A+B,1,_),
	      writeln((A->B))), X).
main :-
	writeln('------rational_term_to_tree(X,A,B,[]), numbervars(A+B,1,_),\
	      writeln((A->B) -----------------'),
	fail.
main :-
	main((rational_term_to_tree(X,A,B,[]), numbervars(A+B,1,_),
	      writeln((A->B))), X).
main.

main(G, X) :-
	d(X),
	m(G).


main2(G, X, L, O) :-
	e(X,L),
	m(G).

m( G ) :-
	G,
	!,
	writeln(yes),
	end.
m( G ) :-
	writeln(no),
	end.

d(X) :- X :=: [_A].
d(X) :- X :=: [a,_A].
d(X) :- X :=: [X].
d(X) :- X :=: [_|X].
d(X) :- X :=: [_,X].
d(X) :- X :=: [_,x].
d(X) :- X :=: [_,x(X)].
d(X) :- X:=: f(X).
d(X) :- X:=: f(X,X).
d(X) :- X:=: f(_,X).
d(X) :- X:=: f(A,A,X).
d(X) :- X:=: f(A,A,g(A)).
d(X) :- X:=: f(A,g(X,[A|A]),X).
d(X) :- X:=: f(X,[X,X]).
d(X) :- X:=: f(X,[X,g(X)]).
d(X) :- X:=: f(_,X/[X]).
d(X) :- X:=: f(_,A/[A]), A:=: f(X,[X,g(X)]).
d(X) :- X:=: f(_,A/[A]), A:=: f(X,[A,g(X)]).
d(X) :- X:=: f(_,A/[A]), A:=: f(B,[X,g(A)]), B:=:[C|B], C:=:[X].

end :- writeln('....'), fail.

e(X,Y) :- X :=: t(_A,B,_C,D), Y :=: [B,E].
e(X,Y) :- X :=: t(_A,_B,_C,_D), Y :=: [_,_E].
e(X,Y) :- X :=: t(A,_B,C,_D), Y :=: [ A,C].
e(X,Y) :- X :=: t(A,[X,_D]), Y :=: [A,_C,_E].
e(X,Y) :- X :=: t(A,[X,C]), Y :=: [A,C,_E].
e(X,Y) :- X :=: t(A,X,_B,[X,C,_D]), Y :=: [A,C,_E].

a(no,   no).
a(no,   no).
a(yes, yes).
a(yes,  no).
a(yes,  no).
a( no,  no).
a(yes,  no).
a(yes, yes).
a(yes, yes).
a(yes,  no).
a(yes,  no).
a( no,  no).
a(yes,  no).
a(yes, yes).
a(yes, yes).
a(yes,  no).
a(yes,  no).

X :-: Y :- writeln(X), fail.
X :=: X.
