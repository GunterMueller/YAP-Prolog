
/**
  * @file jupyter.yap
  *
  * @brief JUpyter support.
  */

%:- yap_flag(gc_trace,verbose).
:- module( jupyter,
           [
	       op(100,fy,('$')),
	       op(950,fy,:=),
	       op(950,yfx,:=),
	       %	   op(950,fx,<-),
	       %	   op(950,yfx,<-),
	       op(50, yf, []),
	       op(50, yf, '()'),
	       op(100, xfy, '.'),
	       op(100, fy, '.'),
	       jupyter/3,
	       jupyter_consult/1,
	       jupyter_call/2,
	       blank/1,
	       streams/1
           ]
         ).


:-	 use_module(library(lists)).
:-	 use_module(library(maplist)).
:-	 use_module(library(real)).
:- use_module(library(hacks)).
:-	 reexport(library(yapi)).
:-	 reexport(library(python)).
:- reexport(library(complete)).
:- reexport(library(verify)).


:- set_prolog_flag(verbose_load, false).

:- python_import(sys).

:- python_import(yap4py.yapi as yapi).
:- python_import(builtins as builtin_mod).
%:- python_import(yap_ipython.utils.capture).


next_streams( _Caller, exit, _Bindings ) :-
    %    Caller.answer := Bindings,
    !.
next_streams( _Caller, answer, _Bindings ) :-
    %    Caller.answer := Bindings,

    !.
next_streams(_, redo, _ ) :-
    !.
next_streams( _, _, _ ).

user:jupyter_cell(A,B,C) :- jupyter(A,B,C).


jupyter(MCell, MLine, Query ) :-
    j_consult(MCell),
    j_call(MLine,Query).
    %O := IO,outputs,
    %forall(O,(:= display(O))),


j_consult(MCell) :-
    (
	MCell == ""
    ->
    true;
	MCell == ''
    ->
	true;
	jupyter_consult(MCell)
    ).

j_call(Cell,Caller) :-
   (
	Cell == ""
    ->
    true
   ;
	Cell == ''
    ->
    true;
	jupyter_call(Cell,Caller)
    ).


streams(true) :-
    set_stream(python_output, alias(user_output)),
    set_stream(python_error, alias(user_error)).
streams(false) :-
    set_stream(sys_output, alias(user_output)),
    set_stream(sys_error, alias(user_error)).

/**
  * @pred jupyter_query(Cell, PythonEnvironment)
  *
  * how the YAP Jupyter kernels calls a goal in the cell.
  */

user:jupyter_query(Query, Self) :-
    catch(
        jupyter_call(Query, Self),
        Error,
        system_error(Error)
    ).
    
jupyter_call(Line,Self) :-
    %start_low_level_trace,
    read_term_from_atomic(Line, G, [variable_names(Vs)]),
    query_to_answer(G,Vs,Port, GVs, LGs),

    Self.q.port := Port,
    print_message(help, answer(Vs, GVs,LGs,'.~n')),
    flush_output,
			       (Port == exit-> ! ; true ),
	    term_to_dict(Vs,LGs,Dict,_NGs),
		 Self.q.answer := Dict.
%:= print("oo").
jupyter_call(_,Self) :-
    Self.q.answer := fail,
	   fail.

/**
  * @pred jupyter_consult(Cell)
  *
  * how the YAP Jupyter kernels consults the text in cell.
  */

jupyter_consult(Cell) :-
    Options = [],
     yap_flag(verbose_load, false),
   current_source_module(_,user),
    catch(
	ll(Cell,Options),
	    _,fail).

ll(Cell,Options) :-
	    open_mem_read_stream( Cell, Stream),
	    load_files(Stream,[stream(Stream)| Options]  ),
	    catch(close(Stream),_,fail).

:- if(  current_prolog_flag(apple, true) ).

:- putenv( 'LC_ALL', 'en_us:UTF-8').

plot_inline :-
    X := self.inline_plotting,
	      nb_setval(inline, X ),
	      X = true,
	      !,
	      := (
		  import( matplotlib ),
		  matplotlib.use( `nbagg` )
	      ).

:- endif.


%:- ( start_low_level_trace ).


:-  set_stream(user_output, alias(std_output)),
        set_stream(user_error, alias(std_error)),
    open('/python/sys.stdout', append, Output, [alias(python_output)]),
    open('/python/sys.stderr', append, Error, [alias(python_error)]).



