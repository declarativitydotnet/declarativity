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
 * DESCRIPTION: Lexer for OverLog, the P2 Datalog variant
 *
 * Originally hacked up from a C-like Lexer by David Gay and Gustav
 * Hållberg (thanks David!):
 *
 * Copyright (c) 1993-1999 David Gay and Gustav Hållberg
 * All rights reserved.
 * 
 * Permission to use, copy, modify, and distribute this software for any
 * purpose, without fee, and without written agreement is hereby granted,
 * provided that the above copyright notice and the following two paragraphs
 * appear in all copies of this software.
 * 
 * IN NO EVENT SHALL DAVID GAY OR GUSTAV HALLBERG BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
 * OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF DAVID GAY OR
 * GUSTAV HALLBERG HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * DAVID GAY AND GUSTAV HALLBERG SPECIFICALLY DISCLAIM ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS ON AN
 * "AS IS" BASIS, AND DAVID GAY AND GUSTAV HALLBERG HAVE NO OBLIGATION TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
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
%option yyclass="OL_Lexer"
%option prefix="OLBase"
%start CCOMMENT CSTRING
%{
#include "ol_lexer.h"

#include <limits.h>
#include <stdlib.h>

#ifdef YY_DECL
#undef YY_DECL
#endif
#define YY_DECL int OL_Lexer::yylex (YYSTYPE *lvalp, OL_Context *ctxt)

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

<INITIAL><<EOF>> { return OL_EOF; }
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
  cstring = New strbuf();
  BEGIN(CSTRING); 
}
<CSTRING>\" { 
  assert(cstring != NULL);
  lvalp->v = New Parse_Val(Val_Str::mk(*cstring));
  delete cstring;
  cstring = NULL;
  BEGIN(INITIAL); 
  return OL_STRING; 
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

<INITIAL>materialize { return OL_MATERIALIZE; }
<INITIAL>keys { return OL_KEYS; }
<INITIAL>in { return OL_IN; }
<INITIAL>id { return OL_ID; }
<INITIAL>"@" { return OL_AT; }
<INITIAL>"," { return OL_COMMA; }
<INITIAL>"(" { return OL_LPAR; }
<INITIAL>")" { return OL_RPAR; }
<INITIAL>"[" { return OL_LSQUB; }
<INITIAL>"]" { return OL_RSQUB; }

 /* Relational operators */
<INITIAL>"<" { return OL_LT; }
<INITIAL>">" { return OL_GT; }
<INITIAL>"<=" { return OL_LTE; }
<INITIAL>">=" { return OL_GTE; }
<INITIAL>"!=" { return OL_NEQ; }
<INITIAL>"<>" { return OL_NEQ; }
<INITIAL>"==" { return OL_EQ; }

 /* Arithmetic operations */
<INITIAL>"+" { return OL_PLUS; }
<INITIAL>"-" { return OL_MINUS; }
<INITIAL>"*" { return OL_TIMES; }
<INITIAL>"/" { return OL_DIVIDE; }
<INITIAL>"%" { return OL_MODULUS; }
<INITIAL>"**" { return OL_EXP; }
<INITIAL>"^" { return OL_BITXOR; }
<INITIAL>"&" { return OL_BITAND; }
<INITIAL>"|" { return OL_BITOR; }
<INITIAL>">>" { return OL_RSHIFT; }
<INITIAL>"<<" { return OL_LSHIFT; }

 /* Boolean operations */
<INITIAL>"!" { return OL_NOT; }
<INITIAL>"&&" { return OL_AND; }
<INITIAL>"||" { return OL_OR; } 

 /* Regular expressions */
<INITIAL>"~" { return OL_REGEXP; }
<INITIAL>"~*" { return OL_REGIEXP; }
<INITIAL>"!~" { return OL_NOTREGEXP; }
<INITIAL>"!~*" { return OL_NOTREGIEXP; }

<INITIAL>":=" { return OL_ASSIGN; }
<INITIAL>"." { return OL_DOT; }
<INITIAL>":-" { return OL_IF; }
<INITIAL>"period=" { return OL_PERIOD; }
<INITIAL>"watch" { return OL_WATCH; }
<INITIAL>"delete" { return OL_DEL; }
<INITIAL>"range" { return OL_RANGE; }
<INITIAL>"now" { return OL_NOW; }
<INITIAL>[Mm][Aa][Xx] { return OL_MAX; }
<INITIAL>[Mm][Ii][Nn] { return OL_MIN; }
<INITIAL>[Cc][Oo][Uu][Nn][Tt] { return OL_COUNT; }
<INITIAL>"null" { 
  lvalp->v = New Parse_Val(Val_Null::mk()); 
  return OL_NULL; }

<INITIAL>f_[a-zA-Z]+ { 
  lvalp->v = New Parse_Var(Val_Str::mk(yytext)); 
  return OL_FUNCTION; }

<INITIAL>[A-Z]{ALNUM}* { 
  lvalp->v = New Parse_Var(Val_Str::mk(yytext)); 
  return OL_VAR; }

<INITIAL>_ { return OL_DONTCARE; }

<INITIAL>infinity {
  // Unsigned integer literal (including octal and/or hex
  lvalp->v = New Parse_Val(Val_Int64::mk(-1));
  return OL_VALUE;
}

<INITIAL>[a-z]{ALNUM}* { 
  lvalp->v = New Parse_Val(Val_Str::mk(yytext)); 
  return OL_NAME; 
}

<INITIAL>({DIGIT}+|0[xX]{HEXDIGIT}+)U {
  // Unsigned integer literal (including octal and/or hex
  lvalp->v = New Parse_Val(Val_UInt64::mk(strtoull(yytext,NULL,0)));
  return OL_VALUE;
}

<INITIAL>({DIGIT}+|0[xX]{HEXDIGIT}+) {
  // Some integer literal (including octal and/or hex
  lvalp->v = New Parse_Val(Val_Int64::mk(strtoll(yytext,NULL,0)));
  return OL_VALUE;
}

<INITIAL>{DIGIT}+(({DECIM}{EXP}?)|{EXP}) {
  // Double-precision literal
  lvalp->v = New Parse_Val(Val_Double::mk(strtod(yytext,NULL)));
  return OL_VALUE;
}

%%

// Default: yyin == std::cin.
OL_Lexer::OL_Lexer(std::istream *str) 
  : comment_depth(0) 
{
  bufstate = yy_create_buffer( str , YY_BUF_SIZE);
  bufstate->yy_is_our_buffer = 0;
  yy_switch_to_buffer( bufstate );
};

// Give it a string...
OL_Lexer::OL_Lexer(const char *prog) 
  : comment_depth(0), strb(prog)
{
  bufstate = yy_create_buffer( &strb, strlen(prog));
  yy_switch_to_buffer( bufstate );
};

OL_Lexer::~OL_Lexer() { 
  // if (bufstate) yy_delete_buffer(bufstate); 
};
