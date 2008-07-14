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
 * DESCRIPTION: Lexer class for OverLog (the P2 dialect of datalog)
 *
 */

#ifndef __OL_LEXER_H__
#define __OL_LEXER_H__

#include <async.h>
#include <sstream>
#include <iostream>

#include "val_int32.h"
#include "val_int64.h"
#include "val_uint64.h"
#include "val_str.h"
#include "val_double.h"
#include "val_null.h"

#ifndef yyFlexLexer
#define yyFlexLexer OLBaseFlexLexer
#include <FlexLexer.h>
#endif

class OL_Context;

#include "parser_stuff.h"
#include "ol_parser.tab.h"

class OL_Lexer : public OLBaseFlexLexer {

private:
  int comment_depth;
  strbuf *cstring;

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
