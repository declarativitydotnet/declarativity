/*
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 * DESCRIPTION: Lexer class for P2DL (the P2 Dataflow Language)
 *
 */

#ifndef __P2DL_LEXER_H__
#define __P2DL_LEXER_H__

#include <sstream>
#include <iostream>

#ifndef yyFlexLexer
#define yyFlexLexer P2DLBaseFlexLexer
#include <FlexLexer.h>
#endif

#include "p2dlContext.h"
#include "p2dl_parser.H"

class P2DL_Lexer : public P2DLBaseFlexLexer {

private:
  int comment_depth;
  ostringstream *cstring;

  yy_buffer_state *bufstate;
  std::istringstream strb;

public:

  // Default: yyin == std::cin.
  P2DL_Lexer(std::istream *str);
  // Give it a string...
  P2DL_Lexer(const char *prog);
  virtual ~P2DL_Lexer();
  
  int yylex (YYSTYPE *lvalp, compile::p2dl::Context *env);

  int line_num() const { return yylineno; };

  string current_token() const { return string(yytext, yyleng); };

};

#endif /* __P2DL_LEXER_H_ */
