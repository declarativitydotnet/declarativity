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
  static int ol_parser_lex (YYSTYPE *lvalp, compile::parse::Context *ctxt);
  static void ol_parser_error (compile::parse::Context *ctxt, string msg);

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
%left OL_BITNOT
%left OL_EQ OL_NEQ
%nonassoc OL_GT OL_GTE OL_LT OL_LTE
%left OL_LSHIFT OL_RSHIFT
%left OL_PLUS OL_MINUS 
%left OL_EXP OL_TIMES OL_DIVIDE OL_MODULUS
%right OL_NOT
%left OL_IN
%nonassoc OL_ASSIGN

%token<v> OL_NAME 
%token OL_NAMESPACE
%token OL_COMMA
%token OL_DOT
%token OL_EOF 
%token OL_IF 
%token<v> OL_STRING
%token<v> OL_VALUE
%token<v> OL_VAR
%token<v> OL_AGGFUNCNAME
%token<v> OL_FUNCTION
%token<v> OL_NULL
%token OL_LPAR
%token OL_RPAR
%token OL_LSQUB
%token OL_RSQUB
%token OL_LCURB
%token OL_RCURB
%token OL_DEL
%token OL_QUERY
%token OL_MATERIALIZE
%token OL_KEYS
%token OL_WATCH
%token OL_WATCHFINE
%token OL_STAGE
%token OL_TRACE
%token OL_TRACETABLE

%start program
%file-prefix="ol_parser"
%name-prefix="ol_parser_"
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

program: OL_EOF { YYACCEPT; }
         | 
         statements OL_EOF { ctxt->program($1); YYACCEPT; }
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

nameSpace: OL_NAMESPACE OL_NAME OL_LCURB statements OL_RCURB
             { $$ = new compile::parse::Namespace($2->toString(), $4); }
         ;

materialize: OL_MATERIALIZE OL_LPAR OL_NAME OL_COMMA
             tablearg OL_COMMA tablearg OL_COMMA primarykeys OL_RPAR OL_DOT 
             { $$ = new compile::parse::Table($3, $5, $7, $9); } 
    ;

tablearg: OL_VALUE
          { $$ = $1; }
          ;

primarykeys: OL_KEYS OL_LPAR keylist OL_RPAR 
             { $$ = $3; }
             ;

keylist: OL_VALUE { $$ = new compile::parse::ExpressionList(); 
                    $$->push_front($1); }
         | 
         OL_VALUE OL_COMMA keylist 
         { $3->push_front($1); $$=$3; }
         ; 

watch: OL_WATCH OL_LPAR OL_NAME OL_RPAR OL_DOT 
       {
	 $$ = new compile::parse::Watch($3->toString(), "");
       }
       ;

watchfine: OL_WATCHFINE OL_LPAR OL_NAME OL_COMMA OL_STRING OL_RPAR OL_DOT
           {
             $$ = new compile::parse::Watch($3->toString(), $5->toString());
           }
         ;


stage: OL_STAGE OL_LPAR OL_STRING OL_COMMA OL_NAME OL_COMMA OL_NAME OL_RPAR OL_DOT
       {
         $$ = new compile::parse::Stage($3->toString(), $5->toString(), $7->toString());
       }
     ;

rule: OL_NAME functor OL_IF termlist OL_DOT 
        { $$ = new compile::parse::Rule($2, $4, false, $1); } 
      | OL_NAME OL_DEL functor OL_IF termlist OL_DOT 
        { $$ = new compile::parse::Rule($3, $5, true, $1); } 
      | functor OL_IF termlist OL_DOT 
        { $$ = new compile::parse::Rule($1, $3, false); } 
      | OL_DEL functor OL_IF termlist OL_DOT 
        { $$ = new compile::parse::Rule($2, $4, true); } 
      | OL_NAME functor OL_IF aggview OL_DOT 
        { $$ = new compile::parse::Rule($2, $4, false, $1); }
      | functor OL_IF aggview OL_DOT 
        { $$ = new compile::parse::Rule($1, $3, false); }
    ;

term: functor | assign | select 
        { $$=$1; }
      ;

termlist:  term 
           { $$ = new compile::parse::TermList(); $$->push_front($1); }
         | term OL_COMMA termlist 
           { $3->push_front($1); $$=$3; } 
         ;

fact: OL_NAME factbody OL_DOT
        { $$ = new compile::parse::Fact($1, $2); }
    ;

factbody: OL_LPAR atom OL_RPAR
          { $$ = new compile::parse::ExpressionList(); $$->push_front($2); }
        | OL_LPAR atom OL_COMMA functorargs OL_RPAR
          { $4->push_front($2); $$=$4; }
        ;

functor: OL_NAME functorbody 
         { $$=new compile::parse::Functor($1, $2); } 
       ;

functorbody: OL_LPAR locationarg OL_RPAR 
               { $$ = new compile::parse::ExpressionList; $$->push_front($2); }
             | OL_LPAR locationarg OL_COMMA functorargs OL_RPAR
               { $4->push_front($2); $$=$4; }
             ;

locationarg: OL_VAR | aggregate
             { $$ = $1; }
           ;

functorargs: functorarg 
             { $$ = new compile::parse::ExpressionList(); $$->push_front($1); }
           | functorarg OL_COMMA functorargs 
             { $3->push_front($1); $$=$3; }
           ;

functorarg: atom | OL_VAR | aggregate | math_expr | function
            { $$ = $1; }
          ;

aggview: agg_oper OL_LPAR functorbody OL_COMMA functorbody OL_COMMA functor OL_RPAR 
           { $$ = new compile::parse::TermList(); 
             $$->push_back(new compile::parse::AggregationView($1->toString(),
                                                               $3, $5, $7)); }
         ;

function:  OL_FUNCTION OL_LPAR functionargs OL_RPAR
           { $$ = new compile::parse::Function($1, $3); }
         | OL_FUNCTION OL_LPAR OL_RPAR 
           { $$ = new compile::parse::Function($1, new compile::parse::ExpressionList()); }
         ;

functionargs: functionarg 
              { $$ = new compile::parse::ExpressionList(); 
                $$->push_front($1); }
            | functionarg OL_COMMA functionargs 
              { $3->push_front($1); $$=$3; }
            ;

functionarg: math_expr | atom | OL_VAR | function
             { $$ = $1; }
           ;

select: bool_expr 
        { $$ = new compile::parse::Select($1); }
      ;

assign: OL_VAR OL_ASSIGN rel_atom
        { $$ = new compile::parse::Assign($1, $3); }
      | OL_VAR OL_ASSIGN bool_expr
        { $$ = new compile::parse::Assign($1, $3); }
      ;

bool_expr: OL_LPAR bool_expr OL_RPAR 
           { $$ = $2; }
         | OL_VAR OL_IN range_expr 
           { $$ = new compile::parse::Bool(compile::parse::Bool::RANGEI, $1, $3); } 
         | OL_NOT bool_expr 
           { $$ = new compile::parse::Bool(compile::parse::Bool::NOT, $2); } 
         | bool_expr OL_OR bool_expr
           { $$ = new compile::parse::Bool(compile::parse::Bool::OR, $1, $3 ); }
         | bool_expr OL_AND bool_expr
           { $$ = new compile::parse::Bool(compile::parse::Bool::AND, $1, $3 ); }
         | rel_atom rel_oper rel_atom
           { $$ = new compile::parse::Bool($2, $1, $3 ); }
         ;

rel_atom: math_expr | function | atom | OL_VAR
          { $$ = $1; }
        ;

rel_oper: OL_EQ  { $$ = compile::parse::Bool::EQ; } 
        | OL_NEQ { $$ = compile::parse::Bool::NEQ; }
        | OL_GT  { $$ = compile::parse::Bool::GT; }
        | OL_LT  { $$ = compile::parse::Bool::LT; }
        | OL_GTE { $$ = compile::parse::Bool::GTE; }
        | OL_LTE { $$ = compile::parse::Bool::LTE; }
        ;

math_expr: math_expr math_oper math_atom
           { $$ = new compile::parse::Math($2, $1, $3 ); }
         | math_atom math_oper math_atom
           { $$ = new compile::parse::Math($2, $1, $3 ); }
         ;

math_atom: atom | OL_VAR | function
           { $$ = $1; }
         | OL_LPAR math_expr OL_RPAR 
           { $$ = $2; }
         ;

math_oper: OL_LSHIFT  { $$ = compile::parse::Math::LSHIFT; } 
         | OL_RSHIFT  { $$ = compile::parse::Math::RSHIFT; }
         | OL_PLUS    { $$ = compile::parse::Math::PLUS; }
         | OL_MINUS   { $$ = compile::parse::Math::MINUS; }
         | OL_TIMES   { $$ = compile::parse::Math::TIMES; }
         | OL_DIVIDE  { $$ = compile::parse::Math::DIVIDE; }
         | OL_MODULUS { $$ = compile::parse::Math::MODULUS; }
         ;


range_expr: OL_LPAR range_atom OL_COMMA range_atom OL_RPAR 
            { $$ = new compile::parse::Range(compile::parse::Range::RANGEOO, $2, $4); } 
          | OL_LPAR range_atom OL_COMMA range_atom OL_RSQUB
            { $$ = new compile::parse::Range(compile::parse::Range::RANGEOC, $2, $4); } 
          | OL_LSQUB range_atom OL_COMMA range_atom OL_RPAR
            { $$ = new compile::parse::Range(compile::parse::Range::RANGECO, $2, $4); } 
          | OL_LSQUB range_atom OL_COMMA range_atom OL_RSQUB
            { $$ = new compile::parse::Range(compile::parse::Range::RANGECC, $2, $4); } 
          ;

range_atom: math_expr | atom | OL_VAR
            { $$ = $1; }
          ;

vector_expr: OL_LSQUB vectorentries OL_RSQUB
             { $$ = new compile::parse::Vector($2); }

matrix_expr: OL_LCURB matrixentries OL_RCURB
             { $$ = new compile::parse::Matrix($2); }

vectorentries: vectorentry 
               { $$ = new compile::parse::ExpressionList();
                 $$->push_front($1); } 
             | vectorentry OL_COMMA vectorentries 
               { $3->push_front($1); $$=$3; }
             ;

matrixentries: matrixentry 
               { $$ = new compile::parse::ExpressionListList(); 
                 $$->push_front($1); } 
             | matrixentry OL_COMMA matrixentries 
               { $3->push_front($1); $$=$3; }
             ;

vectorentry: OL_VALUE | OL_STRING | OL_NULL 
             { $$ = $1; }
           ;  

matrixentry: OL_LSQUB vectorentries OL_RSQUB 
             { $$ = $2; }
           ;

atom: OL_VALUE | OL_STRING | OL_NULL | vector_expr | matrix_expr
      { $$ = $1; }
    ;

aggregate: agg_oper OL_LT OL_VAR OL_GT 
             {
               $$ = new compile::parse::Aggregation($1->toString(),
                                                    $3);
             }
             | agg_oper OL_LT OL_TIMES OL_GT 
             {
              $$ = new compile::parse::Aggregation($1->toString(),
                                                   NULL);
             }
;

agg_oper:	OL_AGGFUNCNAME
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
#include "ol_lexer.h"

static int ol_parser_lex (YYSTYPE *lvalp, compile::parse::Context *ctxt)
{
  return ctxt->lexer->yylex(lvalp, ctxt);
}
static void ol_parser_error(compile::parse::Context *ctxt, string msg)
{
  TELL_ERROR << "PARSE ERROR " << msg << ": line " << (ctxt->lexer->line_num() - 2) 
             << ", token '" << ctxt->lexer->text() << "'" << std::endl;
  ctxt->error(msg);
}

