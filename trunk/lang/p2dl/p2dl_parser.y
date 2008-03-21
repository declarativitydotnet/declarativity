%{
/*
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 * DESCRIPTION: Bison grammar for P2DL (the P2 Dataflow Language)
 *
 */

  // Prolog[ue]
  #include <deque>
  #include <iostream>
  #include "val_str.h"
  #include "val_list.h"
  #include "val_int64.h"
  #include "val_tuple.h"
  #include "p2dlContext.h"

  union YYSTYPE;
  static int p2dl_parser_lex (YYSTYPE *lvalp, compile::p2dl::Context *ctxt);
  static void p2dl_parser_error (compile::p2dl::Context *ctxt, string msg);

%}
%debug
%defines
%verbose
%pure_parser

%token<v> P2DL_NAME 
%token<v> P2DL_STRING
%token<v> P2DL_VAR
%token<v> P2DL_NULL
%token<v> P2DL_UNSIGNED
%token<v> P2DL_NUMBER
%token<v> P2DL_DOUBLE
%token<v> P2DL_ID
%token P2DL_GRAPH
%token P2DL_INSTALL
%token P2DL_EDIT
%token P2DL_LINK
%token P2DL_COMMA
%token P2DL_DOT
%token P2DL_LPAR
%token P2DL_RPAR
%token P2DL_LSQUB
%token P2DL_RSQUB
%token P2DL_LCURB
%token P2DL_RCURB
%token P2DL_LT
%token P2DL_GT
%token P2DL_SEMICOLON
%token P2DL_PLUS
%token P2DL_MINUS
%token P2DL_ASSIGN
%token P2DL_WATCH
%token P2DL_FACT
%token P2DL_TABLE
%token P2DL_KEYS
%token P2DL_EOF 

%start program
%file-prefix="p2dl_parser"
%name-prefix="p2dl_parser_"
%parse-param { compile::p2dl::Context *ctxt }
%lex-param { compile::p2dl::Context *ctxt }
%union {
  compile::p2dl::Expression     *v;
  compile::p2dl::Tuple          *u_tuple;
  compile::p2dl::ExpressionList *u_exprlist;
  compile::p2dl::Variable       *u_var;
  compile::p2dl::Element        *u_element;
  compile::p2dl::Port           *u_port;
  compile::p2dl::Link           *u_link;
  compile::p2dl::Statement      *u_stmt;
  compile::p2dl::StatementList  *u_stmtlist;
  compile::p2dl::Graph          *u_graph;
  compile::p2dl::Edit           *u_edit;
  compile::p2dl::Watch          *u_watch;
  compile::p2dl::Fact           *u_fact;
  compile::p2dl::Table          *u_table;
  ValueList                     *u_valuelist;
}

%type<u_graph>     graph;
%type<u_edit>      edit;
%type<u_stmtlist>  graphbody editbody;
%type<u_watch>     watch;
%type<u_fact>      fact;
%type<u_table>     table;
%type<u_valuelist> factbody elementargs;
%type<u_valuelist> valuelist valuelistargs;
%type<u_valuelist> keys keylist;
%type<u_stmt>      assign;
%type<u_stmt>      strand editstrand;
%type<u_link>      strandlink editstrandlink;
%type<u_port>      port;
%type<u_exprlist>  strandlist editstrandlist editref;
%type<u_element>   element;
%type<u_tuple>     tuplelist;
%type<v>           value tuple elementarg strandatom editstrandatom;

%%

program: clauselist P2DL_EOF { ctxt->commit(); YYACCEPT; }

clauselist: clause
            | 
            clause clauselist
            ;

clause:  graph P2DL_SEMICOLON  { ctxt->graph($1); }
      |  edit  P2DL_SEMICOLON  { ctxt->edit($1); }
      |  fact  P2DL_SEMICOLON  { ctxt->fact($1); }
      |  table P2DL_SEMICOLON  { ctxt->table($1); }
      |  watch P2DL_SEMICOLON  { ctxt->watch($1); }
      ; 

edit: P2DL_EDIT P2DL_VAR
      P2DL_LCURB editbody P2DL_RCURB
        { $$ = new compile::p2dl::Edit($2, $4); }
    ;

editbody: assign P2DL_SEMICOLON
            { $$ = new compile::p2dl::StatementList(); $$->push_front($1); }
        | assign P2DL_SEMICOLON editbody
            { $$ = $3; $$->push_front($1); }
        | editstrand P2DL_SEMICOLON
            { $$ = new compile::p2dl::StatementList(); $$->push_back($1); }
        | editstrand P2DL_SEMICOLON editbody 
            { $$ = $3; $$->push_back($1); }
        ;

editstrand: editstrandlist 
              { $$ = new compile::p2dl::EditStrand($1); }
          ;

editstrandatom: element
                  { $$ = $1; }
              | editref
                  { $$ = new compile::p2dl::Reference($1); }
              ;

editref: P2DL_VAR 
           { $$ = new compile::p2dl::ExpressionList(); $$->push_front($1); }
       | P2DL_VAR P2DL_DOT editref
          { $$ = $3; $$->push_front($1); }
       ;

editstrandlist: editstrandlink 
                  { $$ = new compile::p2dl::ExpressionList(); $$->push_front($1); }
              | editstrandlink P2DL_LINK editstrandlist
                  { $$ = $3; $$->push_front($1); }
              ;


editstrandlink: editstrandatom 
                  { $$ = new compile::p2dl::Link($1, 
                    new compile::p2dl::Port(new compile::p2dl::Value(Val_Int64::mk(0))), 
                    new compile::p2dl::Port(new compile::p2dl::Value(Val_Int64::mk(0)))); }
              | P2DL_LSQUB port P2DL_RSQUB editstrandatom
                  { $$ = new compile::p2dl::Link($4, $2, 
                    new compile::p2dl::Port(new compile::p2dl::Value(Val_Int64::mk(0)))); }
              | editstrandatom P2DL_LSQUB port P2DL_RSQUB
                  { $$ = new compile::p2dl::Link($1, 
                    new compile::p2dl::Port(new compile::p2dl::Value(Val_Int64::mk(0))), $3); }
              | P2DL_LSQUB port P2DL_RSQUB 
                editstrandatom P2DL_LSQUB port P2DL_RSQUB
                  { $$ = new compile::p2dl::Link($4, $2, $6); }
              ;

graph: P2DL_GRAPH P2DL_VAR 
       P2DL_LPAR P2DL_NUMBER P2DL_COMMA P2DL_NUMBER 
       P2DL_COMMA P2DL_STRING P2DL_COMMA P2DL_STRING P2DL_RPAR
       P2DL_LCURB graphbody P2DL_RCURB
         { $$ = new compile::p2dl::Graph($2, $4, $6, $8, $10, $13); }
     ;

graphbody: assign P2DL_SEMICOLON 
             { $$ = new compile::p2dl::StatementList(); $$->push_front($1); }
         | strand P2DL_SEMICOLON
             { $$ = new compile::p2dl::StatementList(); $$->push_front($1); }
         | graph P2DL_SEMICOLON
             { $$ = new compile::p2dl::StatementList(); $$->push_front($1); }
         |
           assign P2DL_SEMICOLON graphbody
             { $$ = $3; $$->push_front($1); }
         |
           strand P2DL_SEMICOLON graphbody
             { $$ = $3; $$->push_back($1); }
         |
           graph P2DL_SEMICOLON graphbody
             { $$ = $3; $$->push_front($1); }
         ;

watch: P2DL_WATCH P2DL_LPAR P2DL_VAR P2DL_RPAR
         { $$ = new compile::p2dl::Watch($3); }
       ;

table: P2DL_TABLE P2DL_LPAR P2DL_VAR P2DL_COMMA
       value P2DL_COMMA value P2DL_COMMA keys P2DL_RPAR
         { $$ = new compile::p2dl::Table($3,$5,$7,$9); }
     ;

keys: P2DL_KEYS P2DL_LPAR keylist P2DL_RPAR
      { $$ = $3; }
    ;

keylist: value
         { $$ = new ValueList();
           $$->push_back(dynamic_cast<compile::p2dl::Value*>($1)->value()); }
         |
         value P2DL_COMMA keylist
         { $3->insert($3->begin(), dynamic_cast<compile::p2dl::Value*>($1)->value()); 
           $$=$3; }
         ;

fact: P2DL_FACT P2DL_LPAR factbody P2DL_RPAR
        { $$ = new compile::p2dl::Fact($3); }
    ;

factbody: value
          { $$ = new ValueList(); 
            $$->push_back(dynamic_cast<compile::p2dl::Value*>($1)->value()); }
        | value P2DL_COMMA factbody
          { $$ = $3; 
            $$->insert($$->begin(), dynamic_cast<compile::p2dl::Value*>($1)->value()); }
        ;


assign: P2DL_VAR P2DL_ASSIGN element
        { $$ = new compile::p2dl::Assign($1, $3); }
      |
        P2DL_VAR P2DL_ASSIGN value
        { $$ = new compile::p2dl::Assign($1, $3); }
      ;

strand: strandlist { $$ = new compile::p2dl::Strand($1); }
      ;

strandatom: P2DL_VAR | element { $$ = $1; }
          ;

strandlist: strandlink 
              { $$ = new compile::p2dl::ExpressionList(); $$->push_front($1); }
          | strandlink P2DL_LINK strandlist
              { $$ = $3; $$->push_front($1); }
          ;

strandlink: strandatom 
       { $$ = new compile::p2dl::Link($1, 
         new compile::p2dl::Port(new compile::p2dl::Value(Val_Int64::mk(0))), 
         new compile::p2dl::Port(new compile::p2dl::Value(Val_Int64::mk(0)))); }
    | P2DL_LSQUB port P2DL_RSQUB strandatom
       { $$ = new compile::p2dl::Link($4, $2, 
         new compile::p2dl::Port(new compile::p2dl::Value(Val_Int64::mk(0)))); }
    | strandatom P2DL_LSQUB port P2DL_RSQUB
       { $$ = new compile::p2dl::Link($1, 
         new compile::p2dl::Port(new compile::p2dl::Value(Val_Int64::mk(0))), $3); }
    | P2DL_LSQUB port P2DL_RSQUB 
      strandatom P2DL_LSQUB port P2DL_RSQUB
       { $$ = new compile::p2dl::Link($4, $2, $6); }
    ; 

port: P2DL_NUMBER 
        { $$ = new compile::p2dl::Port($1); }
      | P2DL_STRING
        { $$ = new compile::p2dl::Port($1); }
    | P2DL_PLUS
        { $$ = new compile::p2dl::Port(NULL, true); }
    | P2DL_PLUS P2DL_STRING
        { $$ = new compile::p2dl::Port($2, true); }
    ;

element: P2DL_NAME P2DL_LPAR elementargs P2DL_RPAR
           { $$ = new compile::p2dl::Element($1, $3); }
         |
         P2DL_NAME
           { $$ = new compile::p2dl::Element($1, new ValueList()); }
       ;

elementargs: elementarg 
               { $$ = new ValueList(); 
                 $$->push_back(dynamic_cast<compile::p2dl::Value*>($1)->value()); }
           | elementarg P2DL_COMMA elementargs
               { $$ = $3; 
                 $$->insert($$->begin(), dynamic_cast<compile::p2dl::Value*>($1)->value()); }
           ;

elementarg: value | tuple
              { $$ = $1; }
          | valuelist
              { $$ = new compile::p2dl::Value(Val_List::mk(List::mk($1))); }
          ;

value: P2DL_DOUBLE | P2DL_NUMBER | P2DL_UNSIGNED | P2DL_ID | P2DL_STRING 
         { $$ = $1; }
     ;

valuelist: P2DL_LCURB valuelistargs P2DL_RCURB 
           { $$ = $2; }
         |
           P2DL_LCURB P2DL_RCURB
           { $$ = new ValueList(); }
         ;

valuelistargs: value
           { $$ = new ValueList();
             $$->push_front(dynamic_cast<compile::p2dl::Value*>($1)->value()); }
         | value P2DL_COMMA valuelistargs
           { $$ = $3;
             $3->push_front(dynamic_cast<compile::p2dl::Value*>($1)->value()); }
         ;

tuple: P2DL_VAR P2DL_LT tuplelist P2DL_GT
         { TuplePtr tp = Tuple::mk($1->toString()); 
           tp->concat($3->tuple()); 
           $$ = new compile::p2dl::Value(Val_Tuple::mk(tp)); }
     ;

tuplelist: value
             { $$ = new compile::p2dl::Tuple(); $$->prepend($1); }
         |
           value P2DL_COMMA tuplelist
             { $$ = $3; $$->prepend($1); }
         ;

%%

// Epilog
    
#undef yylex
#include "p2dl_lexer.h"

static int p2dl_parser_lex (YYSTYPE *lvalp, compile::p2dl::Context *ctxt)
{
  return ctxt->lexer->yylex(lvalp, ctxt);
}

static void p2dl_parser_error(compile::p2dl::Context *ctxt, string msg)
{
  std::cerr << "P2DL PARSE ERROR: line " << ctxt->lexer->line_num() << ", " 
            << "token: " << ctxt->lexer->current_token() << ". " << msg << std::endl;
  ctxt->error(msg);
}

