/*
 * @(#)$Id$
 *
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 * DESCRIPTION: Lexer for OverLog, the P2 Datalog variant
 *
 * Originally hacked up from a C-like Lexer by David Gay and Gustav
 * Hållberg (thanks David!):
 *
 */

/*
  What are our tokens?  
  Atoms, Quoted Atoms, 
  Strings, Int literals, Float literals, 
  Variables, Localized Variables.
  Syntax tokens: ( ) [ ] , ; . :- < >
*/


%option c++
%option yylineno
%option noyywrap
%option yyclass="OLG_Lexer"
%option prefix="OLBase"
%start CCOMMENT CSTRING
%{
#include "olg_lexer.h"

#include <limits.h>
#include <stdlib.h>

#ifdef YY_DECL
#undef YY_DECL
#endif
#define YY_DECL int OLG_Lexer::yylex (YYSTYPE *lvalp, compile::parse::Context *ctxt)

int unNamedVariableCounter = 0;

%}

DIGIT           [0-9]
EXP		[eE][+-]?{DIGIT}+
DECIM		\.{DIGIT}+
ALNUM		[_0-9a-zA-Z]
HEXDIGIT	[0-9a-fA-F]

VARIABLE	[A-Z]{ALNUM}*
LOC_VARIABLE	_{
QATOM		\'[^\']*\'
WHITESPACE	[ \t\r\n]+

%%

<INITIAL><<EOF>> { return OLG_EOF; }
<INITIAL>{WHITESPACE}		;
<INITIAL>%%.*			; // Ignore %% comments
<INITIAL>^#!.*			; // Ignore '#' directives

<INITIAL>\/\* { 
  if ( comment_depth == 0 ) {
    BEGIN(CCOMMENT); 
  }
  ++comment_depth;
}
 
<CCOMMENT>[^\*]|(\*[^/]) ;
<CCOMMENT>\*+\/	{
  if ( comment_depth > 0 ) {
   if( --comment_depth == 0 ) {
     BEGIN(INITIAL);
   }
 } else {
   REJECT;
 }
}
<INITIAL>\" { 
  assert(cstring == NULL);
  cstring = new ostringstream();
  BEGIN(CSTRING); 
}
<CSTRING>\" { 
  assert(cstring != NULL);
  lvalp->v = new compile::parse::Value(Val_Str::mk(cstring->str()));
  delete cstring;
  cstring = NULL;
  BEGIN(INITIAL); 
  return OLG_STRING; 
}
<CSTRING>\\.	{
  assert(cstring != NULL);
  // An escaped character literal
  switch (yytext[1]) {
  case 'n': (*cstring) << "\n"; break;
  case 'r': (*cstring) << "\r"; break;
  case 't': (*cstring) << "\t"; break;
  case 'f': (*cstring) << "\f"; break;
  default: (*cstring) << yytext[1]; break;
  }
}
<CSTRING>[^\\\"]+ { 
    assert(cstring != NULL);
    (*cstring) << yytext; 
}

<INITIAL>materialize { return OLG_MATERIALIZE; }
<INITIAL>namespace { return OLG_NAMESPACE; }
<INITIAL>keys { return OLG_KEYS; }
<INITIAL>in { return OLG_IN; }
<INITIAL>"," { return OLG_COMMA; }
<INITIAL>"(" { return OLG_LPAR; }
<INITIAL>")" { return OLG_RPAR; }
<INITIAL>"[" { return OLG_LSQUB; }
<INITIAL>"]" { return OLG_RSQUB; }
<INITIAL>"{" { return OLG_LCURB; }
<INITIAL>"}" { return OLG_RCURB; }

 /* Relational operators */
<INITIAL>"<" { return OLG_LT; }
<INITIAL>">" { return OLG_GT; }
<INITIAL>"<=" { return OLG_LTE; }
<INITIAL>">=" { return OLG_GTE; }
<INITIAL>"!=" { return OLG_NEQ; }
<INITIAL>"<>" { return OLG_NEQ; }
<INITIAL>"==" { return OLG_EQ; }

 /* Arithmetic operations */
<INITIAL>"+" { return OLG_PLUS; }
<INITIAL>"-" { return OLG_MINUS; }
<INITIAL>"*" { return OLG_TIMES; }
<INITIAL>"/" { return OLG_DIVIDE; }
<INITIAL>"%" { return OLG_MODULUS; }
<INITIAL>"**" { return OLG_EXP; }
<INITIAL>"^" { return OLG_BITXOR; }
<INITIAL>"&" { return OLG_BITAND; }
<INITIAL>"|" { return OLG_BITOR; }
<INITIAL>"~" { return OLG_BITNOT; }
<INITIAL>">>" { return OLG_RSHIFT; }
<INITIAL>"<<" { return OLG_LSHIFT; }

 /* Boolean operations */
<INITIAL>"!" { return OLG_NOT; }
<INITIAL>"&&" { return OLG_AND; }
<INITIAL>"||" { return OLG_OR; } 

<INITIAL>":=" { return OLG_ASSIGN; }
<INITIAL>"." { return OLG_DOT; }
<INITIAL>":-" { return OLG_IF; }
<INITIAL>"watch" { return OLG_WATCH; }
<INITIAL>"watchmod" { return OLG_WATCHFINE; }
<INITIAL>"stage" { return OLG_STAGE; }
<INITIAL>"traceTable" {return OLG_TRACETABLE;}
<INITIAL>"trace" {return OLG_TRACE;}
<INITIAL>"delete" { return OLG_DEL; }
<INITIAL>"Query" { return OLG_QUERY; }
<INITIAL>"null" { 
  lvalp->v = new compile::parse::Value(Val_Null::mk()); 
  return OLG_NULL; }

<INITIAL>"true" {
  // Unsigned integer literal (including octal and/or hex)
  lvalp->v = new compile::parse::Value(Val_Int32::mk(1));
  return OLG_VALUE;
}

<INITIAL>"false" {
  // Unsigned integer literal (including octal and/or hex)
  lvalp->v = new compile::parse::Value(Val_Int32::mk(0));
  return OLG_VALUE;
}

<INITIAL>a_[_a-zA-Z0-9]+ { 
  string aggName;
  yytext += 2;
  while (*yytext) aggName += char(toupper(*yytext++));
  lvalp->v = new compile::parse::Variable(Val_Str::mk(aggName)); 
  return OLG_AGGFUNCNAME;
 }

<INITIAL>f_[a-zA-Z0-9]+ { 
  lvalp->v = new compile::parse::Variable(Val_Str::mk(yytext)); 
  return OLG_FUNCTION; }

<INITIAL>[A-Z]{ALNUM}* { 
  lvalp->v = new compile::parse::Variable(Val_Str::mk(yytext)); 
  return OLG_VAR; }

<INITIAL>\@[A-Z]{ALNUM}* { 
  lvalp->v = new compile::parse::Variable(Val_Str::mk(yytext+1), true); 
  return OLG_VAR; }

<INITIAL>_ { 
  ostringstream oss;
  oss << "$_" << unNamedVariableCounter++; 
  lvalp->v = new compile::parse::Variable(Val_Str::mk(oss.str())); 
  return OLG_VAR; }

<INITIAL>infinity {
  // Unsigned integer literal (including octal and/or hex)
  lvalp->v = new compile::parse::Value(Val_Int32::mk(-1));
  return OLG_VALUE;
}

<INITIAL>("::"|[a-z]){ALNUM}* { 
  lvalp->v = new compile::parse::Value(Val_Str::mk(yytext)); 
  return OLG_NAME; 
}

<INITIAL>({DIGIT}+|0[xX]{HEXDIGIT}+)U {
  // Unsigned integer literal (including octal and/or hex)
  lvalp->v = new compile::parse::Value(Val_UInt32::mk(strtoull(yytext,NULL,0)));
  return OLG_VALUE;
}

<INITIAL>(-?{DIGIT}+|0[xX]{HEXDIGIT}+) {
  // Some integer literal (including octal and/or hex)
  lvalp->v = new compile::parse::Value(Val_Int32::mk(strtoll(yytext,NULL,0)));
  return OLG_VALUE;
}

<INITIAL>-?{DIGIT}+(({DECIM}{EXP}?)|{EXP}) {
  // Double-precision literal
  lvalp->v = new compile::parse::Value(Val_Double::mk(strtod(yytext,NULL)));
  return OLG_VALUE;
}

<INITIAL>0[xX]{HEXDIGIT}+I {
  // IDs are read in only in hexadecimal with an I appended to the end
  std::string hex(yytext);
  std::string choppedString = hex.substr(2, hex.size() - 3);
  compile::parse::Value *val = new compile::parse::Value(Val_ID::mk(choppedString));
  lvalp->v = val;
  return OLG_VALUE;
}

%%

// Default: yyin == std::cin.
OLG_Lexer::OLG_Lexer(std::istream *str) 
  : comment_depth(0), cstring(NULL) 
{
  bufstate = yy_create_buffer( str , YY_BUF_SIZE);
  bufstate->yy_is_our_buffer = 0;
  yy_switch_to_buffer( bufstate );
};

// Give it a string...
OLG_Lexer::OLG_Lexer(const char *prog) 
  : comment_depth(0), cstring(NULL), strb(prog)
{
  bufstate = yy_create_buffer( &strb, strlen(prog));
  yy_switch_to_buffer( bufstate );
};

OLG_Lexer::~OLG_Lexer() { 
  // if (bufstate) yy_delete_buffer(bufstate); 
};
