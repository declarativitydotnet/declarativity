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
  #include "parseContext.h"
  #include "reporting.h"
  #include "aggFactory.h"

  union YYSTYPE;
  static int olg_parser_lex (YYSTYPE *lvalp, compile::parse::Context *ctxt);
  static void olg_parser_error (compile::parse::Context *ctxt, string msg);

%}
%debug
%defines
%verbose
%pure_parser

%left OLG_OR
%left OLG_AND
%left OLG_BITOR
%left OLG_BITXOR
%left OLG_BITAND
%left OLG_BITNOT
%left OLG_EQ OLG_NEQ
%nonassoc OLG_GT OLG_GTE OLG_LT OLG_LTE
%left OLG_LSHIFT OLG_RSHIFT
%left OLG_PLUS OLG_MINUS 
%left OLG_EXP OLG_TIMES OLG_DIVIDE OLG_MODULUS
%right OLG_NOT
%left OLG_IN
%nonassoc OLG_ASSIGN

%token<v> OLG_NAME 
%token OLG_NAMESPACE
%token OLG_COMMA
%token OLG_DOT
%token OLG_EOF 
%token OLG_IF 
%token<v> OLG_STRING
%token<v> OLG_VALUE
%token<v> OLG_VAR
%token<v> OLG_AGGFUNCNAME
%token<v> OLG_FUNCTION
%token<v> OLG_NULL
%token OLG_LPAR
%token OLG_RPAR
%token OLG_LSQUB
%token OLG_RSQUB
%token OLG_LCURB
%token OLG_RCURB
%token OLG_DEL
%token OLG_QUERY
%token OLG_MATERIALIZE
%token OLG_KEYS
%token OLG_WATCH
%token OLG_WATCHFINE
%token OLG_STAGE
%token OLG_TRACE
%token OLG_TRACETABLE

%start program
%file-prefix="olg_parser"
%name-prefix="olg_parser_"
%parse-param { compile::parse::Context *ctxt }
%lex-param { compile::parse::Context *ctxt }
%union {
  compile::parse::Bool::Operator             u_boper;
  compile::parse::Math::Operator             u_moper;
  compile::parse::TermList                  *u_termlist;
  compile::parse::Term                      *u_term;
  compile::parse::ExpressionList            *u_exprlist;
  compile::parse::ExpressionListList        *u_exprlistlist;
  compile::parse::Expression                *v;
  compile::parse::Statement                 *u_statement;
  compile::parse::StatementList             *u_statementlist;
}

%type<u_statement>     statement nameSpace rule materialize watch watchfine stage fact;
%type<u_statementlist> statements;
%type<u_termlist>      termlist aggview;
%type<u_exprlist>      factbody functorbody functorargs functionargs vectorentries
%type<u_exprlist>      matrixentry primarykeys keylist; 
%type<u_exprlistlist>  matrixentries;
%type<u_term>          term functor assign select;
%type<v>               functorarg locationarg functionarg tablearg atom rel_atom math_atom;
%type<v>               function math_expr bool_expr range_expr range_atom aggregate;
%type<v>               vectorentry vector_expr matrix_expr;
%type<u_boper>         rel_oper;
%type<u_moper>         math_oper;
%type<v>               agg_oper;
%%

program: OLG_EOF { YYACCEPT; }
         | 
         statements OLG_EOF { ctxt->program($1); YYACCEPT; }
         ;

statements: statement
              { $$ = new compile::parse::StatementList(); $$->push_back($1); }
            | 
            statement statements
              { $$ = $2; $$->push_front($1); }
            ;

statement:   nameSpace   { $$ = $1; }
           | rule        { $$ = $1; }
           | materialize { $$ = $1; }
           | watch       { $$ = $1; }
           | watchfine   { $$ = $1; }
           | stage       { $$ = $1; }
           | fact        { $$ = $1; }
         ;

nameSpace: OLG_NAMESPACE OLG_NAME OLG_LCURB statements OLG_RCURB
             { $$ = new compile::parse::Namespace($2->toString(), $4); }
         ;

materialize: OLG_MATERIALIZE OLG_LPAR OLG_NAME OLG_COMMA
             tablearg OLG_COMMA tablearg OLG_COMMA primarykeys OLG_RPAR OLG_DOT 
             { $$ = new compile::parse::Table($3, $5, $7, $9); } 
    ;

tablearg: OLG_VALUE
          { $$ = $1; }
          ;

primarykeys: OLG_KEYS OLG_LPAR keylist OLG_RPAR 
             { $$ = $3; }
             ;

keylist: OLG_VALUE { $$ = new compile::parse::ExpressionList(); 
                    $$->push_front($1); }
         | 
         OLG_VALUE OLG_COMMA keylist 
         { $3->push_front($1); $$=$3; }
         ; 

watch: OLG_WATCH OLG_LPAR OLG_NAME OLG_RPAR OLG_DOT 
       {
	 $$ = new compile::parse::Watch($3->toString(), "");
       }
       ;

watchfine: OLG_WATCHFINE OLG_LPAR OLG_NAME OLG_COMMA OLG_STRING OLG_RPAR OLG_DOT
           {
             $$ = new compile::parse::Watch($3->toString(), $5->toString());
           }
         ;


stage: OLG_STAGE OLG_LPAR OLG_STRING OLG_COMMA OLG_NAME OLG_COMMA OLG_NAME OLG_RPAR OLG_DOT
       {
         $$ = new compile::parse::Stage($3->toString(), $5->toString(), $7->toString());
       }
     ;

rule: OLG_NAME functor OLG_IF termlist OLG_DOT 
        { $$ = new compile::parse::Rule($2, $4, false, $1); } 
      | OLG_NAME OLG_DEL functor OLG_IF termlist OLG_DOT 
        { $$ = new compile::parse::Rule($3, $5, true, $1); } 
      | functor OLG_IF termlist OLG_DOT 
        { $$ = new compile::parse::Rule($1, $3, false); } 
      | OLG_DEL functor OLG_IF termlist OLG_DOT 
        { $$ = new compile::parse::Rule($2, $4, true); } 
      | OLG_NAME functor OLG_IF aggview OLG_DOT 
        { $$ = new compile::parse::Rule($2, $4, false, $1); }
      | functor OLG_IF aggview OLG_DOT 
        { $$ = new compile::parse::Rule($1, $3, false); }
    ;

term: functor | assign | select 
        { $$=$1; }
      ;

termlist:  term 
           { $$ = new compile::parse::TermList(); $$->push_front($1); }
         | term OLG_COMMA termlist 
           { $3->push_front($1); $$=$3; } 
         ;

fact: OLG_NAME factbody OLG_DOT
        { $$ = new compile::parse::Fact($1, $2); }
    ;

factbody: OLG_LPAR atom OLG_RPAR
          { $$ = new compile::parse::ExpressionList(); $$->push_front($2); }
        | OLG_LPAR atom OLG_COMMA functorargs OLG_RPAR
          { $4->push_front($2); $$=$4; }
        ;

functor: OLG_NAME functorbody 
         { $$=new compile::parse::Functor($1, $2); } 
       ;

functorbody: OLG_LPAR locationarg OLG_RPAR 
               { $$ = new compile::parse::ExpressionList; $$->push_front($2); }
             | OLG_LPAR locationarg OLG_COMMA functorargs OLG_RPAR
               { $4->push_front($2); $$=$4; }
             ;

locationarg: OLG_VAR | aggregate
             { $$ = $1; }
           ;

functorargs: functorarg 
             { $$ = new compile::parse::ExpressionList(); $$->push_front($1); }
           | functorarg OLG_COMMA functorargs 
             { $3->push_front($1); $$=$3; }
           ;

functorarg: atom | OLG_VAR | aggregate | math_expr | function
            { $$ = $1; }
          ;

aggview: agg_oper OLG_LPAR functorbody OLG_COMMA functorbody OLG_COMMA functor OLG_RPAR 
           { $$ = new compile::parse::TermList(); 
             $$->push_back(new compile::parse::AggregationView($1->toString(),
                                                               $3, $5, $7)); }
         ;

function:  OLG_FUNCTION OLG_LPAR functionargs OLG_RPAR
           { $$ = new compile::parse::Function($1, $3); }
         | OLG_FUNCTION OLG_LPAR OLG_RPAR 
           { $$ = new compile::parse::Function($1, new compile::parse::ExpressionList()); }
         ;

functionargs: functionarg 
              { $$ = new compile::parse::ExpressionList(); 
                $$->push_front($1); }
            | functionarg OLG_COMMA functionargs 
              { $3->push_front($1); $$=$3; }
            ;

functionarg: math_expr | atom | OLG_VAR | function
             { $$ = $1; }
           ;

select: bool_expr 
        { $$ = new compile::parse::Select($1); }
      ;

assign: OLG_VAR OLG_ASSIGN rel_atom
        { $$ = new compile::parse::Assign($1, $3); }
      | OLG_VAR OLG_ASSIGN bool_expr
        { $$ = new compile::parse::Assign($1, $3); }
      ;

bool_expr: OLG_LPAR bool_expr OLG_RPAR 
           { $$ = $2; }
         | OLG_VAR OLG_IN range_expr 
           { $$ = new compile::parse::Bool(compile::parse::Bool::RANGEI, $1, $3); } 
         | OLG_NOT bool_expr 
           { $$ = new compile::parse::Bool(compile::parse::Bool::NOT, $2); } 
         | bool_expr OLG_OR bool_expr
           { $$ = new compile::parse::Bool(compile::parse::Bool::OR, $1, $3 ); }
         | bool_expr OLG_AND bool_expr
           { $$ = new compile::parse::Bool(compile::parse::Bool::AND, $1, $3 ); }
         | rel_atom rel_oper rel_atom
           { $$ = new compile::parse::Bool($2, $1, $3 ); }
         ;

rel_atom: math_expr | function | atom | OLG_VAR
          { $$ = $1; }
        ;

rel_oper: OLG_EQ  { $$ = compile::parse::Bool::EQ; } 
        | OLG_NEQ { $$ = compile::parse::Bool::NEQ; }
        | OLG_GT  { $$ = compile::parse::Bool::GT; }
        | OLG_LT  { $$ = compile::parse::Bool::LT; }
        | OLG_GTE { $$ = compile::parse::Bool::GTE; }
        | OLG_LTE { $$ = compile::parse::Bool::LTE; }
        ;

math_expr: math_expr math_oper math_atom
           { $$ = new compile::parse::Math($2, $1, $3 ); }
         | math_atom math_oper math_atom
           { $$ = new compile::parse::Math($2, $1, $3 ); }
         ;

math_atom: atom | OLG_VAR | function
           { $$ = $1; }
         | OLG_LPAR math_expr OLG_RPAR 
           { $$ = $2; }
         ;

math_oper: OLG_LSHIFT  { $$ = compile::parse::Math::LSHIFT; } 
         | OLG_RSHIFT  { $$ = compile::parse::Math::RSHIFT; }
         | OLG_PLUS    { $$ = compile::parse::Math::PLUS; }
         | OLG_MINUS   { $$ = compile::parse::Math::MINUS; }
         | OLG_TIMES   { $$ = compile::parse::Math::TIMES; }
         | OLG_DIVIDE  { $$ = compile::parse::Math::DIVIDE; }
         | OLG_MODULUS { $$ = compile::parse::Math::MODULUS; }
         ;


range_expr: OLG_LPAR range_atom OLG_COMMA range_atom OLG_RPAR 
            { $$ = new compile::parse::Range(compile::parse::Range::RANGEOO, $2, $4); } 
          | OLG_LPAR range_atom OLG_COMMA range_atom OLG_RSQUB
            { $$ = new compile::parse::Range(compile::parse::Range::RANGEOC, $2, $4); } 
          | OLG_LSQUB range_atom OLG_COMMA range_atom OLG_RPAR
            { $$ = new compile::parse::Range(compile::parse::Range::RANGECO, $2, $4); } 
          | OLG_LSQUB range_atom OLG_COMMA range_atom OLG_RSQUB
            { $$ = new compile::parse::Range(compile::parse::Range::RANGECC, $2, $4); } 
          ;

range_atom: math_expr | atom | OLG_VAR
            { $$ = $1; }
          ;

vector_expr: OLG_LSQUB vectorentries OLG_RSQUB
             { $$ = new compile::parse::Vector($2); }

matrix_expr: OLG_LCURB matrixentries OLG_RCURB
             { $$ = new compile::parse::Matrix($2); }

vectorentries: vectorentry 
               { $$ = new compile::parse::ExpressionList();
                 $$->push_front($1); } 
             | vectorentry OLG_COMMA vectorentries 
               { $3->push_front($1); $$=$3; }
             ;

matrixentries: matrixentry 
               { $$ = new compile::parse::ExpressionListList(); 
                 $$->push_front($1); } 
             | matrixentry OLG_COMMA matrixentries 
               { $3->push_front($1); $$=$3; }
             ;

vectorentry: OLG_VALUE | OLG_STRING | OLG_NULL 
             { $$ = $1; }
           ;  

matrixentry: OLG_LSQUB vectorentries OLG_RSQUB 
             { $$ = $2; }
           ;

atom: OLG_VALUE | OLG_STRING | OLG_NULL | vector_expr | matrix_expr
      { $$ = $1; }
    ;

aggregate: agg_oper OLG_LT OLG_VAR OLG_GT 
             {
               $$ = new compile::parse::Aggregation($1->toString(),
                                                    $3);
             }
             | agg_oper OLG_LT OLG_TIMES OLG_GT 
             {
              $$ = new compile::parse::Aggregation($1->toString(),
                                                   NULL);
             }
;

agg_oper:	OLG_AGGFUNCNAME
                {
                  $$ = $1;
                  try {
                    AggFactory::mk($$->toString());
                  } catch (AggFactory::AggregateNotFound anf) {
                    ctxt->error(string("Unknown aggregate name: ") +
                                anf.what());
                  }
                }

%%

// Epilog
    
#undef yylex
#include "olg_lexer.h"

static int olg_parser_lex (YYSTYPE *lvalp, compile::parse::Context *ctxt)
{
  return ctxt->lexer->yylex(lvalp, ctxt);
}
static void olg_parser_error(compile::parse::Context *ctxt, string msg)
{
  TELL_ERROR << "PARSE ERROR " << msg << ": line " << (ctxt->lexer->line_num() - 2) 
             << ", token '" << ctxt->lexer->text() << "'" << std::endl;
  ctxt->error(msg);
}

