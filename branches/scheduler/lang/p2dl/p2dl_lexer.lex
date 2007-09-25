/*
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 * DESCRIPTION: Lexer for P2DL
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
%option yyclass="P2DL_Lexer"
%option prefix="P2DLBase"
%start CCOMMENT CSTRING
%{
#include "p2dl_lexer.h"

#include <limits.h>
#include <stdlib.h>
#include "val_null.h"
#include "val_int64.h"
#include "val_double.h"
#include "val_id.h"

#ifdef YY_DECL
#undef YY_DECL
#endif
#define YY_DECL int P2DL_Lexer::yylex (YYSTYPE *lvalp, compile::p2dl::Context *ctxt)

int dcvar = 0;

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

<INITIAL><<EOF>> { return P2DL_EOF; }
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
<CSTRING>\\\" {
  assert(cstring != NULL);
  (*cstring) << "\"";
}
<CSTRING>\" { 
  assert(cstring != NULL);
  lvalp->v = new compile::p2dl::Value(Val_Str::mk(cstring->str()));
  delete cstring;
  cstring = NULL;
  BEGIN(INITIAL); 
  return P2DL_STRING;
}
<CSTRING>\\.	{
  assert(cstring != NULL);
  // An escaped character literal
  switch (yytext[2]) {
  case 'n': (*cstring) << "\n"; break;
  case 'r': (*cstring) << "\r"; break;
  case 't': (*cstring) << "\t"; break;
  case 'f': (*cstring) << "\f"; break;
  default: (*cstring) << yytext[2]; break;
  }
}
<CSTRING>[^\\\"]+ { 
    assert(cstring != NULL);
    (*cstring) << yytext; 
}

 /* Misc globals and keywords */
<INITIAL>graph   { return P2DL_GRAPH; }
<INITIAL>install { return P2DL_INSTALL; }
<INITIAL>edit    { return P2DL_EDIT; }
<INITIAL>table   { return P2DL_TABLE; }
<INITIAL>watch   { return P2DL_WATCH; }
<INITIAL>fact    { return P2DL_FACT; }
<INITIAL>"->"    { return P2DL_LINK; }
<INITIAL>","     { return P2DL_COMMA; }
<INITIAL>"."     { return P2DL_DOT; }
<INITIAL>"("     { return P2DL_LPAR; }
<INITIAL>")"     { return P2DL_RPAR; }
<INITIAL>"["     { return P2DL_LSQUB; }
<INITIAL>"]"     { return P2DL_RSQUB; }
<INITIAL>"{"     { return P2DL_LCURB; }
<INITIAL>"}"     { return P2DL_RCURB; }
<INITIAL>"<"     { return P2DL_LT; }
<INITIAL>">"     { return P2DL_GT; }
<INITIAL>";"     { return P2DL_SEMICOLON; }

 /* Arithmetic operations */
<INITIAL>"+" { return P2DL_PLUS; }
<INITIAL>"-" { return P2DL_MINUS; }

<INITIAL>"=" { return P2DL_ASSIGN; }

<INITIAL>"null" { 
  lvalp->v = new compile::p2dl::Value(Val_Null::mk()); return P2DL_NULL; }

<INITIAL>"true" { 
  lvalp->v = new compile::p2dl::Value(Val_Int64::mk(1)); return P2DL_UNSIGNED; }
<INITIAL>"false" { 
  lvalp->v = new compile::p2dl::Value(Val_Int64::mk(0)); return P2DL_UNSIGNED; }

<INITIAL>[a-z]{ALNUM}* { 
  lvalp->v = new compile::p2dl::Variable(Val_Str::mk(yytext)); 
  return P2DL_VAR; }

<INITIAL>[A-Z]{ALNUM}* { 
  lvalp->v = new compile::p2dl::Value(Val_Str::mk(yytext)); 
  return P2DL_NAME; 
}

 <INITIAL>({DIGIT}+|0[xX]{HEXDIGIT}+) {
  // Unsigned integer literal (including octal and/or hex
  lvalp->v = new compile::p2dl::Value(Val_Int64::mk(strtoul(yytext,NULL,0)));
  return P2DL_UNSIGNED;
}

<INITIAL>(-?{DIGIT}+|0[xX]{HEXDIGIT}+) {
  // Some integer literal (including octal and/or hex
  lvalp->v = new compile::p2dl::Value(Val_Int64::mk(strtol(yytext,NULL,0)));
  return P2DL_NUMBER;
}

<INITIAL>-?{DIGIT}+(({DECIM}{EXP}?)|{EXP}) {
  // Double-precision literal
  lvalp->v = new compile::p2dl::Value(Val_Double::mk(strtod(yytext,NULL)));
  return P2DL_DOUBLE;
}

<INITIAL>0[xX]{HEXDIGIT}+I {
  // IDs are read in only in hexadecimal with an I appended to the end
  std::string hex(yytext);
  std::string choppedString = hex.substr(2, hex.size() - 3);
  lvalp->v = new compile::p2dl::Value(Val_ID::mk(choppedString));
  return P2DL_ID;
}

%%

// Default: yyin == std::cin.
P2DL_Lexer::P2DL_Lexer(std::istream *str) 
  : comment_depth(0), cstring(NULL) 
{
  bufstate = yy_create_buffer( str , YY_BUF_SIZE);
  bufstate->yy_is_our_buffer = 0;
  yy_switch_to_buffer( bufstate );
};

// Give it a string...
P2DL_Lexer::P2DL_Lexer(const char *prog) 
  : comment_depth(0), cstring(NULL), strb(prog)
{
  bufstate = yy_create_buffer( &strb, strlen(prog));
  yy_switch_to_buffer( bufstate );
};

P2DL_Lexer::~P2DL_Lexer() { 
  if (bufstate) yy_delete_buffer(bufstate); 
};
