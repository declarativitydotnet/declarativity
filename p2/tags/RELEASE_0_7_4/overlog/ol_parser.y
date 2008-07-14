%{
/*
 * @(#)$Id$
 *
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 * DESCRIPTION: Bison grammar for OverLog (the P2 dialect of datalog)
 *
 */

  // Prolog[ue]
  #include <deque>
  #include <iostream>
  #include "val_str.h"
  #include "ol_context.h"

  union YYSTYPE;
  static int ol_parser_lex (YYSTYPE *lvalp, OL_Context *ctxt);
  static void ol_parser_error (OL_Context *ctxt, string msg);

%}
%debug
%defines
%verbose
%pure_parser

%left OL_OR
%left OL_AND
%left OL_BITOR
%left OL_BITXOR
%left OL_BITAND
%left OL_EQ OL_NEQ
%nonassoc OL_GT OL_GTE OL_LT OL_LTE
%nonassoc OL_REGEXP OL_REGIEXP OL_NOTREGEXP OL_NOTREGIEXP
%left OL_LSHIFT OL_RSHIFT
%left OL_PLUS OL_MINUS 
%left OL_TIMES OL_DIVIDE OL_MODULUS
%left OL_EXP
%right OL_NOT
%left OL_IN
%left OL_ID
%nonassoc OL_ASSIGN

%token OL_AT 
%token<v> OL_NAME 
%token OL_MAX
%token OL_MIN
%token OL_COUNT
%token OL_COMMA
%token OL_DONTCARE
%token OL_DOT
%token OL_EOF 
%token OL_IF 
%token<v> OL_STRING
%token<v> OL_VALUE
%token<v> OL_VAR
%token<v> OL_FUNCTION
%token<v> OL_NULL
%token OL_LPAR
%token OL_RPAR
%token OL_LSQUB
%token OL_RSQUB
%token OL_LCURB
%token OL_RCURB
%token OL_PERIOD
%token OL_DEL
%token OL_QUERY
%token OL_NOW
%token OL_LOCAL
%token OL_MATERIALIZE
%token OL_KEYS
%token OL_WATCH
%token OL_TRACE
%token OL_TRACETABLE

%token OL_RANGEOO
%token OL_RANGEOC
%token OL_RANGECO
%token OL_RANGECC

%start program
%file-prefix="ol_parser"
%name-prefix="ol_parser_"
%parse-param { OL_Context *ctxt }
%lex-param { OL_Context *ctxt }
%union {
  Parse_Bool::Operator  u_boper;
  Parse_Math::Operator  u_moper;
  char*                 u_aoper;
  Parse_TermList	    *u_termlist;
  Parse_Term		    *u_term;
  Parse_FunctorName	    *u_functorname;
  Parse_ExprList	    *u_exprlist;
  Parse_ExprListList	*u_exprlistlist;
  Parse_Expr		    *v;
  Parse_AggTerm         *u_aggterm;
  Parse_Vector          *u_vector;
  Parse_Matrix          *u_matrix;
}

%type<u_termlist>    termlist;
%type<u_exprlist>    functorbody functorargs functionargs vectorentries matrixentry primarykeys keylist; 
%type<u_exprlistlist> matrixentries;
%type<u_functorname> functorname;
%type<u_term>        term functor assign select;
%type<u_aggterm>     aggview;
%type<v>             functorarg functionarg tablearg atom rel_atom math_atom function math_expr bool_expr range_expr range_atom aggregate vectorentry vectoratom matrixatom;
%type<u_boper>       rel_oper;
%type<u_moper>       math_oper;
%type<u_aoper>       agg_oper;
%type<u_vector>  vector_expr;
%type<u_matrix>  matrix_expr;
%%

program:	OL_EOF { YYACCEPT; }
		| clauselist OL_EOF { YYACCEPT; }
		;

clauselist:	clause
		| clause clauselist
                ;

clause:		  rule
		| fact
                | materialize
                | watch
		| trace
                | traceTable
                | query               
		;

materialize:	OL_MATERIALIZE OL_LPAR OL_NAME OL_COMMA
		tablearg OL_COMMA tablearg OL_COMMA primarykeys OL_RPAR OL_DOT 
			{ ctxt->table($3, $5, $7, $9); } 
		;

tablearg:	OL_VALUE
			{ $$ = $1; }
		;

primarykeys:	OL_KEYS OL_LPAR keylist OL_RPAR {
			$$ = $3;
		}
		;

keylist:	OL_VALUE { $$ = new Parse_ExprList(); $$->push_front($1); }
		| 
		OL_VALUE OL_COMMA keylist { $3->push_front($1); $$=$3; }
		; 

watch:		OL_WATCH OL_LPAR OL_NAME OL_RPAR OL_DOT {
			ctxt->watch($3);
		}
		;

trace:		OL_TRACE OL_LPAR OL_NAME OL_RPAR OL_DOT {
			ctxt->traceTuple($3);
		}
		;

traceTable:	OL_TRACETABLE OL_LPAR OL_NAME OL_RPAR OL_DOT {
			ctxt->traceTable($3);
		}
		;

fact:           functor OL_DOT { ctxt->fact($1); }

rule:	        OL_NAME functor OL_IF termlist OL_DOT { 
                    ctxt->rule($2, $4, false, $1); } 
                | OL_NAME OL_DEL functor OL_IF termlist OL_DOT { 
		    ctxt->rule($3, $5, true, $1); } 
	        | functor OL_IF termlist OL_DOT { 
                    ctxt->rule($1, $3, false); } 
                | OL_DEL functor OL_IF termlist OL_DOT { 
		    ctxt->rule($2, $4, true); } 
                | OL_NAME functor OL_IF aggview OL_DOT {
		    ctxt->aggRule($2, $4, false, $1); 
		  }
                | functor OL_IF aggview OL_DOT {
                    ctxt->aggRule($1, $3, false);
		  }
                ;

query:          OL_QUERY functorname functorbody OL_DOT {
                  ctxt->query(new Parse_Functor($2, $3)); }
                ;

termlist:	term { $$ = new Parse_TermList(); $$->push_front($1); }
		| term OL_COMMA termlist { $3->push_front($1); $$=$3; } 
		;

term:		functor | assign | select { $$=$1; }
		;

functor:	functorname functorbody 
			{ $$=new Parse_Functor($1, $2); } 
		;

aggview:        agg_oper OL_LPAR functorbody OL_COMMA functorbody OL_COMMA functor OL_RPAR 
                        { $$ = new Parse_AggTerm($1, $3, $5, $7); }
                ;

functorname:	OL_NAME 
			{ $$ = new Parse_FunctorName($1); }
		;

functorbody:	OL_LPAR OL_RPAR 
			{ $$=new Parse_ExprList(); }
		| OL_LPAR functorargs OL_RPAR 
			{ $$=$2; };

functorargs:	functorarg { 
			$$ = new Parse_ExprList(); 
			$$->push_front($1); }
		| functorarg OL_COMMA functorargs {
			$3->push_front($1); 
			$$=$3; }
        | OL_AT atom {
            $$ = new Parse_ExprList(); 
			Parse_Var *pv = dynamic_cast<Parse_Var*>($2);
			if (!pv) {
			  ostringstream oss;
			  oss << "location specifier is not a variable";
			  ctxt->error(oss.str());
			}
			else {
			  pv->locspec = true;
			  $$->push_front($2); 
			}}
        | OL_AT atom OL_COMMA functorargs{
			Parse_Var *pv = dynamic_cast<Parse_Var*>($2);
			if (!pv) {
			  ostringstream oss;
			  oss << "location specifier is not a variable";
			  ctxt->error(oss.str());
			}
			else {
			  pv->locspec = true;
			  $4->push_front($2); 
			  $$=$4; 
			}}
		;

functorarg:	atom
			{ $$ = $1; }
		| aggregate
			{ $$ = $1; }
		;


function:	OL_FUNCTION OL_LPAR functionargs OL_RPAR
			{ $$ = new Parse_Function($1, $3); }
		| OL_FUNCTION OL_LPAR OL_RPAR 
			{ $$ = new Parse_Function($1, new Parse_ExprList()); }
		;

functionargs:	functionarg { 
			$$ = new Parse_ExprList(); 
			$$->push_front($1); }
		| functionarg OL_COMMA functionargs { 
			$3->push_front($1); 
			$$=$3; }
		;

functionarg:	math_expr 
			{ $$ = $1; }
		| atom
			{ $$ = $1; }
		;

select:    	bool_expr
			{ $$ = new Parse_Select($1); }
		;

assign:		OL_VAR OL_ASSIGN rel_atom
			{ $$ = new Parse_Assign($1, $3); }
		| OL_VAR OL_ASSIGN bool_expr
			{ $$ = new Parse_Assign($1, $3); }
		;

bool_expr:	OL_LPAR bool_expr OL_RPAR 
			{ $$ = $2; }
		| OL_VAR OL_IN range_expr 
			{ $$ = new Parse_Bool(Parse_Bool::RANGE, $1, $3); } 
		| OL_VAR OL_IN OL_ID range_expr 
			{ $$ = new Parse_Bool(Parse_Bool::RANGE, $1, $4, true); } 
		| OL_NOT bool_expr 
			{ $$ = new Parse_Bool(Parse_Bool::NOT, $2 ); } 
		| bool_expr OL_OR bool_expr
			{ $$ = new Parse_Bool(Parse_Bool::OR, $1, $3 ); }
		| bool_expr OL_AND bool_expr
			{ $$ = new Parse_Bool(Parse_Bool::AND, $1, $3 ); }
		| rel_atom rel_oper rel_atom
			{ $$ = new Parse_Bool($2, $1, $3 ); }
		| rel_atom rel_oper OL_ID rel_atom
			{ $$ = new Parse_Bool($2, $1, $4, true); }
		;

rel_atom:	math_expr 
			{ $$ = $1; }
		| function
			{ $$ = $1; }
		| atom
			{ $$ = $1; }
		;

rel_oper:	  OL_EQ  { $$ = Parse_Bool::EQ; } 
		| OL_NEQ { $$ = Parse_Bool::NEQ; }
		| OL_GT  { $$ = Parse_Bool::GT; }
		| OL_LT  { $$ = Parse_Bool::LT; }
		| OL_GTE { $$ = Parse_Bool::GTE; }
		| OL_LTE { $$ = Parse_Bool::LTE; }
		;

math_expr:	math_expr math_oper math_atom
			{ $$ = new Parse_Math($2, $1, $3 ); }
		| math_expr math_oper OL_ID math_atom
			{ $$ = new Parse_Math($2, $1, $4, true ); }
		| math_atom math_oper math_atom
			{ $$ = new Parse_Math($2, $1, $3 ); }
		| math_atom math_oper OL_ID math_atom
			{ $$ = new Parse_Math($2, $1, $4, true ); }
        | vectoratom
            { $$ = $1; }
        | matrixatom
            { $$ = $1; }
		;

math_atom:	atom 
			{ $$ = $1; }
		| function
			{ $$ = $1; }
		| OL_LPAR math_expr OL_RPAR 
			{ $$ = $2; }
		;

math_oper:	  OL_LSHIFT  { $$ = Parse_Math::LSHIFT; } 
		| OL_RSHIFT  { $$ = Parse_Math::RSHIFT; }
		| OL_PLUS    { $$ = Parse_Math::PLUS; }
		| OL_MINUS   { $$ = Parse_Math::MINUS; }
		| OL_TIMES   { $$ = Parse_Math::TIMES; }
		| OL_DIVIDE  { $$ = Parse_Math::DIVIDE; }
		| OL_MODULUS { $$ = Parse_Math::MODULUS; }
		;


range_expr:	OL_LPAR range_atom OL_COMMA range_atom OL_RPAR 
			{ $$ = new Parse_Range(Parse_Range::RANGEOO, $2, $4); } 
		| OL_LPAR range_atom OL_COMMA range_atom OL_RSQUB
			{ $$ = new Parse_Range(Parse_Range::RANGEOC, $2, $4); } 
		| OL_LSQUB range_atom OL_COMMA range_atom OL_RPAR
			{ $$ = new Parse_Range(Parse_Range::RANGECO, $2, $4); } 
		| OL_LSQUB range_atom OL_COMMA range_atom OL_RSQUB
			{ $$ = new Parse_Range(Parse_Range::RANGECC, $2, $4); } 
		;

range_atom:	math_expr
			{ $$ = $1; }
		| atom
			{ $$ = $1; }
		;

vector_expr: OL_LSQUB vectorentries OL_RSQUB
            { $$ = new Parse_Vector($2); }

matrix_expr: OL_LCURB matrixentries OL_RCURB
            { $$ = new Parse_Matrix($2, ctxt); }

vectorentries: vectorentry {
             $$ = new Parse_ExprList();
             $$->push_front($1); } 
       | vectorentry OL_COMMA vectorentries {
       			$3->push_front($1); 
			$$=$3; }
		;

matrixentries: matrixentry {
             $$ = new Parse_ExprListList();
             $$->push_front($1); } 
       | matrixentry OL_COMMA matrixentries {
       			$3->push_front($1); 
			$$=$3; }
		;
vectorentry: math_atom { $$ = $1; };  

matrixentry: OL_LSQUB vectorentries OL_RSQUB
         { $$ = $2; };

vectoratom : OL_VAR OL_LSQUB vectorentry OL_RSQUB {
            $$ = new Parse_VecAtom($1, $3);
		};

matrixatom : OL_VAR OL_LCURB vectorentry OL_COMMA vectorentry OL_RCURB {
            $$ = new Parse_MatAtom($1, $3, $5);
		};

atom:		OL_VALUE | OL_VAR | OL_STRING | OL_NULL
			{ $$ = $1; }
        | vector_expr
            { $$ = $1; }
        | matrix_expr
            { $$ = $1; }
		| OL_NOW
			{ $$ = Parse_Expr::Now; }
		;

aggregate:	agg_oper OL_LT OL_VAR OL_GT 
                        { $$ = new Parse_Agg($3, $1, ValuePtr()); }
		|
		agg_oper OL_LT OL_AT OL_VAR OL_GT 
                        {
                          // Make the variable a location specifier
                          Parse_Var *pv = dynamic_cast<Parse_Var*>($4);
                          pv->locspec = true;
                          $$ = new Parse_Agg($4, $1, ValuePtr());
                        }
		|
		agg_oper OL_LT OL_TIMES OL_GT 
			{ $$ = new Parse_Agg(Parse_Agg::DONT_CARE, $1, ValuePtr()); }
		;

agg_oper:	  OL_MIN   { $$ = "MIN"; }
		| OL_MAX   { $$ = "MAX"; }
		| OL_COUNT { $$ = "COUNT"; }
		;

%%

// Epilog
		
#undef yylex
#include "ol_lexer.h"

static int ol_parser_lex (YYSTYPE *lvalp, OL_Context *ctxt)
{
  return ctxt->lexer->yylex(lvalp, ctxt);
}
static void ol_parser_error(OL_Context *ctxt, string msg)
{
  ctxt->error(msg);
}

