%{
/*
 * @(#)$Id$
 *
 * Copyright (c) 2005 Intel Corporation. All rights reserved.
 *
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
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
  static void ol_parser_error (OL_Context *ctxt, str msg);

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

%token OL_AT 
%token<v> OL_ATOM 
%token OL_COMMA
%token OL_DONTCARE
%token OL_DOT
%token OL_EOF 
%token OL_IF 
%token<v> OL_VALUE
%token<v> OL_VAR
%token OL_LPAR
%token OL_RPAR
%token OL_LSQUB
%token OL_RSQUB
%token OL_RULE
%token OL_EVENT
%token OL_PERIOD
%token OL_DEL

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
  Parse_FunctorName	*u_functorname;
  Parse_Term		*u_term;
  Parse_TermList	*u_termlist;
  Parse_Expr		*u_expr;
  Parse_ExprList	*u_exprlist;
  Parse_Range		*u_range;
  Parse_Val		*v;
}
%type<u_termlist> termlist;
%type<u_exprlist> exprlist termbody; 
%type<u_term> term;
%type<u_expr> expr;
%type<u_expr> infix_expr;
%type<u_functorname> functorname;
%type<u_range> range_expr;
%%
program:	OL_EOF { YYACCEPT; }
		| clauselist OL_EOF { YYACCEPT; }
		;

clauselist:	clause 
		| clause clauselist;

clause:		OL_RULE rule 
		| fact 
                | event
		;

fact:		term OL_DOT { ctxt->add_fact($1); } ;

rule:	        OL_VAR term OL_IF termlist OL_DOT { 
                    ctxt->add_rule(New Parse_Expr($1), $2, $4, false); } 
                | OL_VAR OL_DEL term OL_IF termlist OL_DOT { 
		    ctxt->add_rule(New Parse_Expr($1), $3, $5, true); } 
                ;

event:          OL_EVENT term OL_DOT { ctxt->add_event($2); } ;

termlist:	term { $$ = New Parse_TermList(); $$->push_front($1); }
		| term OL_COMMA termlist { $3->push_front($1); $$=$3; } ;

term:		functorname termbody { $$=New Parse_Term($1, $2); } ;
		| infix_expr { $$=New Parse_Term($1); };

termbody:	OL_LPAR OL_RPAR { 
			$$=New Parse_ExprList(); }
		| OL_LPAR exprlist OL_RPAR { 
			$$=$2; };

functorname:	OL_ATOM { 
			$$ = New Parse_FunctorName($1); }
		| OL_ATOM OL_AT OL_VAR { 
			$$ = New Parse_FunctorName($1,$3); }
		;

exprlist:	infix_expr { 
			$$ = New Parse_ExprList(); 
			$$->push_front($1); }
		| infix_expr OL_COMMA exprlist { 
			$3->push_front($1); 
			$$=$3; }
		;

infix_expr:	OL_LPAR infix_expr OL_RPAR 
			{ $$ = $2; }
		| OL_VAR OL_IN range_expr 
			{ $$ = New Parse_Expr( $1, $3 ); } 
		| expr 
			{ $$ = $1; }
		| OL_PLUS infix_expr 
			{ $$ = New Parse_Expr( OL_PLUS, $2 ); } 
		| OL_MINUS infix_expr 
			{ $$ = New Parse_Expr( OL_MINUS, $2 ); } 
		| OL_NOT infix_expr 
			{ $$ = New Parse_Expr( OL_NOT, $2 ); } 
		| infix_expr OL_OR infix_expr
			{ $$ = New Parse_Expr( OL_OR, $1, $3 ); }
		| infix_expr OL_AND infix_expr
			{ $$ = New Parse_Expr( OL_AND, $1, $3 ); }
		| infix_expr OL_EQ infix_expr
			{ $$ = New Parse_Expr( OL_EQ, $1, $3 ); }
		| infix_expr OL_NEQ infix_expr
			{ $$ = New Parse_Expr( OL_NEQ, $1, $3 ); }
		| infix_expr OL_GT infix_expr
			{ $$ = New Parse_Expr( OL_GT, $1, $3 ); }
		| infix_expr OL_LT infix_expr
			{ $$ = New Parse_Expr( OL_LT, $1, $3 ); }
		| infix_expr OL_GTE infix_expr
			{ $$ = New Parse_Expr( OL_GTE, $1, $3 ); }
		| infix_expr OL_LTE infix_expr
			{ $$ = New Parse_Expr( OL_LTE, $1, $3 ); }
		| infix_expr OL_REGEXP infix_expr
			{ $$ = New Parse_Expr( OL_REGEXP, $1, $3 ); }
		| infix_expr OL_REGIEXP infix_expr
			{ $$ = New Parse_Expr( OL_REGIEXP, $1, $3 ); }
		| infix_expr OL_NOTREGEXP infix_expr
			{ $$ = New Parse_Expr( OL_NOTREGEXP, $1, $3 ); }
		| infix_expr OL_NOTREGIEXP infix_expr
			{ $$ = New Parse_Expr( OL_NOTREGIEXP, $1, $3 ); }
		| infix_expr OL_LSHIFT infix_expr
			{ $$ = New Parse_Expr( OL_LSHIFT, $1, $3 ); }
		| infix_expr OL_RSHIFT infix_expr
			{ $$ = New Parse_Expr( OL_RSHIFT, $1, $3 ); }
		| infix_expr OL_PLUS infix_expr
			{ $$ = New Parse_Expr( OL_PLUS, $1, $3 ); }
		| infix_expr OL_MINUS infix_expr
			{ $$ = New Parse_Expr( OL_MINUS, $1, $3 ); }
		| infix_expr OL_TIMES infix_expr
			{ $$ = New Parse_Expr( OL_TIMES, $1, $3 ); }
		| infix_expr OL_DIVIDE infix_expr
			{ $$ = New Parse_Expr( OL_DIVIDE, $1, $3 ); }
		| infix_expr OL_MODULUS infix_expr
			{ $$ = New Parse_Expr( OL_MODULUS, $1, $3 ); }
		| infix_expr OL_EXP infix_expr
			{ $$ = New Parse_Expr( OL_EXP, $1, $3 ); }
		;

range_expr:	OL_LPAR infix_expr OL_COMMA infix_expr OL_RPAR 
			{ $$ = New Parse_Range(OL_RANGEOO, $2, $4); } 
		| OL_LPAR infix_expr OL_COMMA infix_expr OL_RSQUB
			{ $$ = New Parse_Range(OL_RANGEOC, $2, $4); } 
		| OL_LSQUB infix_expr OL_COMMA infix_expr OL_RPAR
			{ $$ = New Parse_Range(OL_RANGECO, $2, $4); } 
		| OL_LSQUB infix_expr OL_COMMA infix_expr OL_RSQUB
			{ $$ = New Parse_Range(OL_RANGECC, $2, $4); } 
		;

expr:		OL_ATOM {
			$$= New Parse_Expr($1); }
		| OL_VALUE { 
			$$ = New Parse_Expr($1); } 
		| OL_VAR { 
		        $$ = New Parse_Expr($1, true); }
		| OL_DONTCARE { 
			$$ = New Parse_Expr(); }
		| OL_ATOM OL_LT OL_VAR OL_GT {
			$$ = New Parse_Expr($3, $1); }
		;

%%

// Epilog
		
#undef yylex
#include "ol_lexer.h"

static int ol_parser_lex (YYSTYPE *lvalp, OL_Context *ctxt)
{
  return ctxt->lexer->yylex(lvalp, ctxt);
}
static void ol_parser_error(OL_Context *ctxt, str msg)
{
  ctxt->error(msg);
}

