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
%left OLG_APPEND
%left OLG_BITXOR
%left OLG_BITAND
%left OLG_BITNOT
%left OLG_EQ OLG_NEQ
%nonassoc OLG_GT OLG_GTE OLG_LT OLG_LTE
%left OLG_LSHIFT OLG_RSHIFT
%left OLG_PLUS OLG_MINUS 
%left OLG_EXP OLG_TIMES OLG_DIVIDE OLG_MODULUS
%right OLG_NOT
%right OLG_NOTIN
%left OLG_IN
%nonassoc OLG_ASSIGN
%nonassoc OLG_QUESTION
%nonassoc OLG_COLON

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
%token OLG_INDEX
%token OLG_SAYSMATERIALIZE
%token OLG_SAYS
%token OLG_KEYS
%token OLG_WATCH
%token OLG_WATCHFINE
%token OLG_STAGE
%token OLG_TRACE
%token OLG_TRACETABLE
%token OLG_REF
%token OLG_WEAK
%token OLG_STRONG
%token OLG_WEAKSAYS
%token OLG_STRONGSAYS
%token OLG_NEW

%start program
%file-prefix="olg_parser"
%name-prefix="olg_parser_"
%parse-param { compile::parse::Context *ctxt }
%lex-param { compile::parse::Context *ctxt }
%union {
  compile::parse::Bool::Operator             u_boper;
  compile::parse::Ref::RefType               u_refType;
  compile::parse::TermList                  *u_termlist;
  compile::parse::Term                      *u_term;
  compile::parse::ExpressionList            *u_exprlist;
  compile::parse::ExpressionListList        *u_exprlistlist;
  compile::parse::Expression                *v;
  compile::parse::Statement                 *u_statement;
  compile::parse::StatementList             *u_statementlist;
}

%type<u_statement>     statement nameSpace rule materialize index watch watchfine stage fact ref;
%type<u_statementlist> statements;
%type<u_termlist>      termlist newtermlist;
%type<u_exprlist>      factbody functorbody functorargs functionargs vectorentries;
%type<u_exprlist>      matrixentry keys keylist ifexpr; 
%type<u_exprlistlist>  matrixentries;
%type<u_term>          term functor assign select says newFunctor headTerms;
%type<v>               functorarg locationarg functionarg tablearg atom;
%type<v>               function ifthenelse ifpred ifcase range_expr math_expr bool_expr aggregate;
%type<v>               vectorentry vector_expr matrix_expr;
%type<u_boper>         rel_oper;
%type<u_refType>       refType;
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
           | index       { $$ = $1; }
           | watch       { $$ = $1; }
           | watchfine   { $$ = $1; }
           | stage       { $$ = $1; }
           | fact        { $$ = $1; }
           | ref         { $$ = $1; }
         ;

nameSpace: OLG_NAMESPACE OLG_NAME OLG_LCURB statements OLG_RCURB
             { $$ = new compile::parse::Namespace($2->toString(), $4); }
         ;

materialize: OLG_MATERIALIZE OLG_LPAR OLG_NAME OLG_COMMA
             tablearg OLG_COMMA tablearg OLG_COMMA keys OLG_RPAR OLG_DOT 
               { $$ = new compile::parse::Table($3, $5, $7, $9); } |
             OLG_SAYSMATERIALIZE OLG_LPAR OLG_NAME OLG_COMMA
             tablearg OLG_COMMA tablearg OLG_COMMA keys OLG_RPAR OLG_DOT 
               { $$ = new compile::parse::Table($3, $5, $7, $9, true); } 
          ;

index: OLG_INDEX OLG_LPAR OLG_NAME OLG_COMMA OLG_STRING OLG_COMMA keys OLG_RPAR OLG_DOT
       { $$ = new compile::parse::Index($3, $5, $7); }
     ;

ref: refType OLG_REF OLG_LPAR OLG_NAME OLG_COMMA
             OLG_NAME OLG_COMMA tablearg OLG_RPAR OLG_DOT 
             { $$ = new compile::parse::Ref($1, $4, $6, $8); } 
    ;

refType: OLG_WEAK { $$ = compile::parse::Ref::WEAK; } |
         OLG_STRONG { $$ = compile::parse::Ref::STRONG; } |
         OLG_WEAKSAYS { $$ = compile::parse::Ref::WEAKSAYS; } |
         OLG_STRONGSAYS { $$ = compile::parse::Ref::STRONGSAYS; }
       ;

tablearg: OLG_VALUE
          { $$ = $1; }
          ;

keys: OLG_KEYS OLG_LPAR keylist OLG_RPAR 
      { $$ = $3; }
    |
      OLG_KEYS OLG_LPAR OLG_RPAR 
      { $$ = new compile::parse::ExpressionList(); 
        /*$$->push_back(new compile::parse::Value(Val_Int64::mk(0))); */}
    ;

keylist: OLG_VALUE 
         { $$ = new compile::parse::ExpressionList(); 
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

rule: OLG_NAME headTerms OLG_IF termlist OLG_DOT 
        { $$ = new compile::parse::Rule($2, $4, false, $1); } 
      | OLG_NAME OLG_DEL functor OLG_IF termlist OLG_DOT 
        { $$ = new compile::parse::Rule($3, $5, true, $1); 
	//std::cout<<std::endl<<"Found Rule: "<<$$->toString(); 
	} 
      | headTerms OLG_IF termlist OLG_DOT 
        { $$ = new compile::parse::Rule($1, $3, false); 
	//std::cout<<std::endl<<"Found Rule: "<<$$->toString(); 
	}
      | OLG_DEL functor OLG_IF termlist OLG_DOT 
        { $$ = new compile::parse::Rule($2, $4, true); 
	//std::cout<<std::endl<<"Found Rule: "<<$$->toString(); 
	} 
      | OLG_NAME headTerms OLG_IF newtermlist OLG_DOT 
        { $$ = new compile::parse::Rule($2, $4, false, $1);
          $$->makeNew();
	  //std::cout<<std::endl<<"Found Rule: "<<$$->toString(); 
	} 
      | headTerms OLG_IF newtermlist OLG_DOT 
        { $$ = new compile::parse::Rule($1, $3, false); 
          $$->makeNew();
	//std::cout<<std::endl<<"Found Rule: "<<$$->toString(); 
	}
      // pmahajan: currently we don't handle delete in constructor rules
    ;

term: functor | assign | select | says
        { $$=$1; }
      ;

newFunctor: OLG_NEW OLG_LT OLG_VAR OLG_COMMA functorarg OLG_COMMA functor OLG_GT
        { 
	  //	  $$ = new compile::parse::NewFunctor($3, $5, $7);
	  $$ = $7;
	  $$->makeNew($3, $5);
	  //std::cout<<std::endl<<"Executing term";
	}
        | OLG_NEW OLG_LT OLG_VAR OLG_COMMA functorarg OLG_COMMA says OLG_GT
        { 
	  //	  $$ = new compile::parse::NewFunctor($3, $5, $7);
	  $$ = $7;
	  $$->makeNew($3, $5);
	  //std::cout<<std::endl<<"Executing term";
	}
      ;

headTerms: says { $$ = $1;} | 
           newFunctor { $$ = $1;} | 
           functor { $$ = $1;} 
      ;

termlist:  term 
           { $$ = new compile::parse::TermList(); $$->push_front($1); }
         | term OLG_COMMA termlist 
           { $3->push_front($1); $$=$3; } 
         ;

newtermlist:  newFunctor
           { $$ = new compile::parse::TermList(); $$->push_front($1);
	   //std::cout<<std::endl<<"Executing termlist";  
	   }
         | newFunctor OLG_COMMA termlist 
           { $3->push_front($1); $$=$3; } 
         | term OLG_COMMA newtermlist 
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
         { 
	   //std::cout<<std::endl<<"starting functor" ; 
	   $$=new compile::parse::Functor($1, $2); 
	   //std::cout<<std::endl<<"Executing functor: found"<<$$->toString();  
	 }  
       | OLG_NOTIN OLG_NAME functorbody
         { $$=new compile::parse::Functor($2, $3, true); }           
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

functorarg: aggregate | math_expr
            { $$ = $1; }
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

functionarg: math_expr
             { $$ = $1; }
           ;

ifthenelse: ifexpr 
              { compile::parse::Variable *fn = 
                  new compile::parse::Variable(Val_Str::mk("f_ifelse")); 
                $$ = new compile::parse::Function(fn, $1); }
         ;

ifpred: bool_expr | OLG_VALUE | OLG_VAR | OLG_STRING
          { $$ = $1; }
        ; 

ifexpr: ifpred OLG_QUESTION ifcase OLG_COLON ifcase
        { compile::parse::ExpressionList *body = new compile::parse::ExpressionList();
          body->push_back($5); body->push_back($3); body->push_back($1); 
          $$ = body; }
        ; 

ifcase: functionarg
          { $$ = $1; }
       |
       OLG_RPAR ifcase OLG_LPAR
          { $$ = $2; }
       ; 

select: bool_expr 
        { $$ = new compile::parse::Select($1); }
      ;

assign: OLG_VAR OLG_ASSIGN math_expr
        { $$ = new compile::parse::Assign($1, $3); }
      ;

bool_expr: OLG_LPAR bool_expr OLG_RPAR 
           { $$ = $2; }
         | math_expr OLG_IN range_expr 
           { $$ = new compile::parse::Bool(compile::parse::Bool::RANGEI, $1, $3); } 
         | OLG_NOT bool_expr 
           { $$ = new compile::parse::Bool(compile::parse::Bool::NOT, $2); } 
         | bool_expr OLG_OR bool_expr
           { $$ = new compile::parse::Bool(compile::parse::Bool::OR, $1, $3 ); }
         | bool_expr OLG_AND bool_expr
           { $$ = new compile::parse::Bool(compile::parse::Bool::AND, $1, $3 ); }
         | math_expr rel_oper math_expr
           { $$ = new compile::parse::Bool($2, $1, $3 ); }
         ;

rel_oper: OLG_EQ  { $$ = compile::parse::Bool::EQ; } 
        | OLG_NEQ { $$ = compile::parse::Bool::NEQ; }
        | OLG_GT  { $$ = compile::parse::Bool::GT; }
        | OLG_LT  { $$ = compile::parse::Bool::LT; }
        | OLG_GTE { $$ = compile::parse::Bool::GTE; }
        | OLG_LTE { $$ = compile::parse::Bool::LTE; }
        ;

math_expr:	  math_expr OLG_LSHIFT math_expr 
                             { $$ = new compile::parse::Math(compile::parse::Math::LSHIFT, $1, $3); } 
		| math_expr OLG_RSHIFT math_expr 
                             { $$ = new compile::parse::Math(compile::parse::Math::RSHIFT, $1, $3); }
		| math_expr OLG_PLUS math_expr   
                             { $$ = new compile::parse::Math(compile::parse::Math::PLUS, $1, $3); }
		| math_expr OLG_MINUS math_expr  
                             { $$ = new compile::parse::Math(compile::parse::Math::MINUS, $1, $3); }
		| math_expr OLG_TIMES math_expr  
                             { $$ = new compile::parse::Math(compile::parse::Math::TIMES, $1, $3); }
		| math_expr OLG_DIVIDE math_expr 
                             { $$ = new compile::parse::Math(compile::parse::Math::DIVIDE, $1, $3); }
		| math_expr OLG_MODULUS math_expr
                             { $$ = new compile::parse::Math(compile::parse::Math::MODULUS, $1, $3); }
		| math_expr OLG_BITXOR math_expr 
                             { $$ = new compile::parse::Math(compile::parse::Math::BITXOR, $1, $3); }
		| math_expr OLG_BITAND math_expr 
                             { $$ = new compile::parse::Math(compile::parse::Math::BITAND, $1, $3); }
		| math_expr OLG_BITOR math_expr  
                             { $$ = new compile::parse::Math(compile::parse::Math::BITOR, $1, $3); }
		| math_expr OLG_APPEND math_expr  
                             { $$ = new compile::parse::Math(compile::parse::Math::APPEND, $1, $3); }
		| OLG_BITNOT math_expr 
                             { $$ = new compile::parse::Math(compile::parse::Math::BITNOT, $2, NULL); }
                | function | atom | OLG_VAR
                        { $$ = $1; }
		| OLG_LPAR math_expr OLG_RPAR 
			{ $$ = $2; }
                | OLG_LPAR ifthenelse OLG_RPAR
                        { $$ = $2; }
		;



range_expr: OLG_LPAR math_expr OLG_COMMA math_expr OLG_RPAR 
            { $$ = new compile::parse::Range(compile::parse::Range::RANGEOO, $2, $4); } 
          | OLG_LPAR math_expr OLG_COMMA math_expr OLG_RSQUB
            { $$ = new compile::parse::Range(compile::parse::Range::RANGEOC, $2, $4); } 
          | OLG_LSQUB math_expr OLG_COMMA math_expr OLG_RPAR
            { $$ = new compile::parse::Range(compile::parse::Range::RANGECO, $2, $4); } 
          | OLG_LSQUB math_expr OLG_COMMA math_expr OLG_RSQUB
            { $$ = new compile::parse::Range(compile::parse::Range::RANGECC, $2, $4); } 
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
                };

says:           OLG_SAYS OLG_LPAR functorargs OLG_RPAR OLG_LT functor OLG_GT{
                //std::cout<<std::endl<<"starting says: "<<" "<<$6->toString();
                compile::parse::Functor *pf = dynamic_cast<compile::parse::Functor*>($6);
                if (!pf || $3->size() != 4 || pf->isComplement()) {
		  ctxt->error(string("functor is not of type Functor or says \
                          parameters incorrect or functor is of complement type"));
                }
                else{
		    // check for the types of the says parameters
		    $$ = new compile::parse::Says(pf, $3);
		    //std::cout<<std::endl<<"Executing says: found"<<$$->toString();  
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
  ostringstream oss;
  oss << "PARSE ERROR " << msg << ": line " << (ctxt->lexer->line_num() - 2) 
      << ", token '" << ctxt->lexer->text() << "'" << std::endl;
  ctxt->error(oss.str());
}
