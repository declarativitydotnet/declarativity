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
%token OL_MAT
%token OL_RULE
%token OL_EVENT
%token OL_PERIOD
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
  Parse_Val		*v;
}
%type<u_termlist> termlist;
%type<u_exprlist> exprlist termbody; 
%type<u_term> term;
%type<u_expr> expr;
%type<u_functorname> functorname;
%%
program:	OL_EOF { YYACCEPT; }
		| clauselist OL_EOF { YYACCEPT; }
		;

clauselist:	clause 
		| clause clauselist;

clause:		OL_RULE rule 
		| fact 
                | materialize
                | event
		;

materialize:	OL_MAT termbody OL_DOT { ctxt->materialize($2); };

fact:		term OL_DOT { ctxt->add_fact($1); } ;

rule:	        OL_ATOM term OL_IF termlist OL_DOT { ctxt->add_rule(New Parse_Expr($1), $2, $4); } ;

event:          OL_EVENT term OL_DOT { ctxt->add_event($2); } ;

termlist:	term { $$ = New Parse_TermList(); $$->push_front($1); }
		| term OL_COMMA termlist { $3->push_front($1); $$=$3; } ;

term:		functorname termbody { $$=New Parse_Term($1, $2); };

termbody:	OL_LPAR OL_RPAR { 
			$$=New Parse_ExprList(); }
		| OL_LPAR exprlist OL_RPAR { 
			$$=$2; };

functorname:	OL_ATOM { 
			$$ = New Parse_FunctorName($1); }
		| OL_ATOM OL_AT OL_VAR { 
			$$ = New Parse_FunctorName($1,$3); }
		;

exprlist:	expr { 
			$$ = New Parse_ExprList(); 
			$$->push_front($1); }
		| expr OL_COMMA exprlist { 
			$3->push_front($1); 
			$$=$3; }
		;

expr:		OL_ATOM {
			$$= New Parse_Expr($1); }
		| OL_VALUE { 
			$$ = New Parse_Expr($1); } 
		| OL_VAR { 
		        $$ = New Parse_Expr($1, true); }
		| OL_DONTCARE { 
			$$ = New Parse_Expr(); }
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

