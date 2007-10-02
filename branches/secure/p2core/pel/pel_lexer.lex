/*
 * @(#)$Id: pel_lexer.lex 1243 2007-07-16 19:05:00Z maniatis $
 *
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 *
 * DESCRIPTION: Lexer for the P2 Expression Language. 
 *
 * Originally hacked up from a C-like Lexer by David Gay and Gustav
 * H�llberg (thanks David!):
 *
 */

/* 
 * SO WHAT IS THIS? 
 *
 * This is a lexer for the P2 Expression Language (PEL).  PEL is a
 * simple RPN-based stack language for writing computations to be
 * performed on tuples.  It's designed to be easy to extend, and easy
 * to compile down to (from portions of a more human-readable query
 * language, for example).  As a compiler, the only non-trivial
 * translation this lexer does it to convert an occurrance of a
 * constant literal to the opcode to load it from the Pel_Program's
 * constant pool, and adds it to the constant pool. 
 * It doesn't get much simpler than this. 
 */

%option c++
%option noyywrap
%option yyclass="Pel_Lexer"
%option prefix="PelBase"
%{
#include "pel_lexer.h"

#include <limits.h>
#include <stdlib.h>
#include "math.h"
#include "reporting.h"

#define LOG_ERROR(_x) { TELL_ERROR << "Pel_Lexer\"" << _programText << "\": " << _x << "\n"; }

//
// The opcode token table, generated by pel_gen.py.
//
Pel_Lexer::opcode_token Pel_Lexer::tokens[] = {
#include "pel_opcode_tokens.gen.h"
};
const uint32_t Pel_Lexer::num_tokens = 
(sizeof(Pel_Lexer::tokens) / sizeof(Pel_Lexer::opcode_token));

%}

DIGIT           [0-9]
EXP		[eE][+-]?{DIGIT}+
DECIM		\.{DIGIT}+
HEXDIGIT	[0-9a-fA-F]

%%

[ \t\r\n]+  {
  // Ignore whitespace 
}

\/\/.*\n  {
  // Ignore // comments
}

^\#!.*\n  { 
  // Ignore '#' directives
}

\/\* { 
  // Skip over multi-line C-style comments
  int depth = 1, c, star = FALSE, slash = FALSE;
  while (depth > 0 && (c = yyinput()) != EOF) {
    if (c == '*' && slash) depth++;
    else if (c == '/' && star) depth--;
    star = c == '*';
    slash = c == '/';
  }
}

'\\.'	{
  // An escaped character literal
  switch (yytext[2]) {
  case 'n': add_const_int('\n'); break;
  case 'r': add_const_int('\r'); break;
  case 't': add_const_int('\t'); break;
  case 'f': add_const_int('\f'); break;
  default: add_const_int(yytext[2]); break;
  }
}

'[^\\]'	{ 
  // An unescaped character literal
  add_const_int(yytext[1]);
}

({DIGIT}+|0[xX]{HEXDIGIT}+)U {
  // Unsigned integer literal (including octal and/or hex
  uint64_t v = strtoull(yytext,NULL,0);
  add_const_int(v);
}

-?({DIGIT}+|0[xX]{HEXDIGIT}+) {
  // Some integer literal (including octal and/or hex
  int64_t v = strtoll(yytext,NULL,0);
  add_const_int(v);
}

0[xX]{HEXDIGIT}+I {
  string id = string(yytext+2);
  add_const(Val_ID::mk(id.substr(0, id.length()-1)));
}

-?{DIGIT}+(({DECIM}{EXP}?)|{EXP}) {
  // Double-precision literal
  double v = strtod(yytext,NULL);
  if ( v == HUGE_VAL || v == -HUGE_VAL) {
    LOG_ERROR("Double precision literal '" << yytext << "' out of bounds");
  } else {
    add_const_dbl(v);
  }
}

\"([^\n\\"]*(\\(.|\n))?)+\"  {
  // C-style string literal
  ostringstream b;
  const char *text = yytext + 1;
  char c = '\0';
  while (text[1]) {
    if (*text == '\\' && text[1]) {
      switch (text[1]) 	{
      case '\n': break;
      case 'n': c = '\n'; break;
      case 'r': c = '\r'; break;
      case 't': c = '\t'; break;
      default: c = text[1]; break;
      }
      text += 2;
    } else {
      c = *text++;
    }
    b << c; // b.fmt("%c",c);
  }
  add_const_str(b.str());
}

'([^\n\\"']*(\\(.|\n))?)+'  {
  // C-style string literal
  ostringstream b;
  const char *text = yytext + 1;
  char c = '\0';
  while (text[1]) {
    if (*text == '\\' && text[1]) {
      switch (text[1]) 	{
      case '\n': break;
      case 'n': c = '\n'; break;
      case 'r': c = '\r'; break;
      case 't': c = '\t'; break;
      default: c = text[1]; break;
      }
      text += 2;
    } else {
      c = *text++;
    }
    b << c; // b.fmt("%c",c);
  }
  add_const_str(b.str());
}

null|NULL { 
  add_const(Val_Null::mk());
}

\${DIGIT}+ {
  // $0, $1, $2, etc: loading tuple fields
  add_tuple_load(strtol(yytext+1,NULL,0));
}
  

[^[:space:]]* {
  // Anything else we lookup in the mnemonic list
  unsigned int i;
  for (i = 0; i < num_tokens; i++) {
    if (strcmp(yytext, tokens[i].name) == 0) {
      add_opcode(tokens[i].code);
      break;
    }
  } 
  if (i == num_tokens) {
    LOG_ERROR("Bad token <"
              << yytext
              << ">");
  }
}

%%

//
// The basic function the user calls.  Takes a string, creates an
// ephemeral lexer and compiles the program.  
// 
boost::shared_ptr< Pel_Program > Pel_Lexer::compile(const char *prog) 
{
  Pel_Lexer lex(prog);
  return lex.result;
}

//
// Constructor, including parsing the program.
//
Pel_Lexer::Pel_Lexer(const char *prog) 
    : strb(prog),
    _programText(prog)
{ 
  bufstate = yy_create_buffer( &strb, strlen(prog));
  yy_switch_to_buffer( bufstate );

  result.reset(new Pel_Program());
  yylex();
  result->const_pool.freeze();
}

// 
// Called from the lexer: add a "load constant" instruction.
//  XXX TODO: remove duplicates in the constant 
//
void Pel_Lexer::add_const(ValuePtr f)
{
  result->ops.push_back( (result->const_pool.size() & 0xFFFF) << 16 | Pel_VM::OP_PUSH_CONST );
  result->const_pool.append(f);
}

//
// Called from the lexer: add a "load tuple field" instruction.
//
void Pel_Lexer::add_tuple_load(int f)
{
  result->ops.push_back( (f & 0xFFFF) << 16 | Pel_VM::OP_PUSH_FIELD );
}

//
// Called from the lexer: add an opcode.
//
void Pel_Lexer::add_opcode(u_int32_t op)
{
  result->ops.push_back( op & 0xFFFF );
}

//
// External function to convert an opcode back into a mnemonic token.
//
const char *Pel_Lexer::opcode_mnemonic(u_int32_t opcode) 
{
  opcode &=0xFFFF;
  return opcode < Pel_Lexer::num_tokens ? tokens[opcode].name : NULL;
}

//
// Decompile a PEL program back into a string using the opcode table
//
string Pel_Lexer::decompile(Pel_Program &prog)
{
  ostringstream sb;
  
  std::vector<u_int32_t>::iterator op;
  for(op = prog.ops.begin(); op < prog.ops.end(); op++ ) {
    unsigned opn = (*op) >> 16;
    unsigned opc = (*op) & 0xFFFF;
    if ( opc == Pel_VM::OP_PUSH_FIELD ) {
      sb << "$" << opn << " ";
    } else if ( opc == Pel_VM::OP_PUSH_CONST ) {
      if ( opn < prog.const_pool.size() ) {
	if (prog.const_pool[opn]->typeCode() == Value::STR) { 
	  sb << "\"" << prog.const_pool[opn]->toString() << "\" ";
	} else {
	  sb << prog.const_pool[opn]->toString() << " ";
	}
      } else {
	sb << "<out-of-range-const-" << opn << "> ";
      }
    } else if ( opc < num_tokens ) {
      sb << tokens[opc].name << " ";
    } else {
      sb << "<unknown-op-code-" << opc << "> ";
    }
  }
  return sb.str();
}
