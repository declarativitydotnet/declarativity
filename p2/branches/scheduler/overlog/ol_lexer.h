/*
 * @(#)$Id$
 *
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 * DESCRIPTION: Lexer class for OverLog (the P2 dialect of datalog)
 *
 */

#ifndef __OL_LEXER_H__
#define __OL_LEXER_H__

#include <sstream>
#include <iostream>

#include "val_int64.h"
#include "val_str.h"
#include "val_double.h"
#include "val_null.h"
#include "val_id.h"
#include "ID.h"

#ifndef yyFlexLexer
#define yyFlexLexer OLBaseFlexLexer
#include <FlexLexer.h>
#endif

class OL_Context;

#include "parser_util.h"
#include "ol_parser.H"

class OL_Lexer : public OLBaseFlexLexer {

private:
  int comment_depth;
  ostringstream *cstring;

  yy_buffer_state *bufstate;
  std::istringstream strb;

public:

  // Default: yyin == std::cin.
  OL_Lexer(std::istream *str);
  // Give it a string...
  OL_Lexer(const char *prog);
  virtual ~OL_Lexer();
  
  int yylex (YYSTYPE *lvalp, OL_Context *env);

  int line_num() const { return yylineno; };

};

#endif /* __OL_LEXER_H_ */
