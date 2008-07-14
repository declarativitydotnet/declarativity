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
 * DESCRIPTION:
 *
 */

#ifndef __PEL_LEXER_H__
#define __PEL_LEXER_H__

#include <sstream>
#include "pel_program.h"
#include "pel_vm.h"

#include "val_int64.h"
#include "val_str.h"
#include "val_double.h"
#include "val_null.h"
#include "val_list.h"
#include "val_id.h"

#ifndef yyFlexLexer
#define yyFlexLexer PelBaseFlexLexer
#include <FlexLexer.h>
#endif

class Pel_Lexer : public PelBaseFlexLexer {

private:

  struct opcode_token {
    const char *name;
    u_int32_t	code;
  };
  static opcode_token tokens[];
  static const uint32_t num_tokens;

  yy_buffer_state *bufstate;
  std::istringstream strb;

  boost::shared_ptr< Pel_Program > result;

  const char* _programText;

  virtual int yylex();

  void add_const_int(int64_t v) { add_const(Val_Int64::mk(v));};
  void add_const_str(string s) { add_const(Val_Str::mk(s));};
  void add_const_dbl(double d) { add_const(Val_Double::mk(d));};
  void add_const(ValuePtr f);
  void add_tuple_load(int f);
  void add_opcode(u_int32_t op);

  Pel_Lexer(const char *prog);
  virtual ~Pel_Lexer() { yy_delete_buffer(bufstate); };

public:

  // 
  // Take a string and compile into a PEL program
  //
  static boost::shared_ptr< Pel_Program > compile(const char *prog);

  //
  // Decompile a PEL program back into a string
  //
  static string decompile(Pel_Program &prog);

  //
  // Translate a PEL opcode into a mnemonic for the instruction
  //
  static const char *opcode_mnemonic(u_int32_t opcode);

};

#endif /* __PEL_LEXER_H_ */
