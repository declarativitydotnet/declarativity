#!/usr/bin/env python
"""
 @(#)$Id$

Copyright (c) 2005 Intel Corporation. All rights reserved.

This file is distributed under the terms in the attached INTEL-LICENSE file.
If you do not find these files, copies can be found by writing to:
Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
Berkeley, CA, 94704.  Attention:  Intel License Inquiry.

DESCRIPTION: Generate the defintion of the PEL (P2 Expression
             Language) virtual machine
"""

import sys
import string

decls=[]
curop = 0

def emit_opcode( op, ar, va, desc ):
  global curop
  decls.append((curop, op, ar, va, desc))
  curop += 1
  
for op, ar, va, desc in [
  ("drop",1,    "DROP",         "Discard the top of stack"),
  ("swap",2,	"SWAP",         "Swap top two stack values"),
  ("dup",1,	"DUP",          "Duplicate the top stack value"),
  ("",0,        "PUSH_CONST",   "Push a constant"),
  ("",0,        "PUSH_FIELD",   "Push a field of the tuple"),
  ("pop",1,     "POP",          "Pop result tuple field"),
  ("peek", 1,   "PEEK",         "Push a duplicate of a stack element"),
  ("ifelse",3,  "IFELSE",       "If first arg, then return second, else third"),
  ("ifpop",2,   "IFPOP",        "If first arg, then pop second, else nothing"),
  ("ifpoptuple",1,"IFPOP_TUPLE","If first arg, then pop entire tuple, else nothing"),
  ("ifstop",1,   "IFSTOP",      "If first arg, then stop execution"),
  ("dumpStack",1,"DUMPSTACK",   "Dump the contents of the stack prefixed by the string arg"),
 
  ("->t", 1,    "T_MKTUPLE",    "Create a tuple out of the argument"),
  ("append", 2, "T_APPEND",     "Append first argument to second tuple"),
  ("unbox", 1,  "T_UNBOX",      "Replaces a tuple value with its fields top to bottom"),
  ("field", 2,  "T_FIELD",      "Extracts a field of a tuple value"),
  
  ("not",1, 	"NOT",          "Boolean negation"),
  ("and",2,	"AND",          "Boolean AND"),
  ("or",2,	"OR",           "Boolean inclusive-OR"),

  (">>",2,	"LSR",          "Integer logical shift right"),
  (">>>",2,	"ASR",          "Integer arithmetic shift right"),
  ("<<",2,	"LSL",          "Integer arithmetic shift left"),
  ("&",2,       "BIT_AND",      "Bitwise AND"),
  ("|",2,       "BIT_OR",       "Bitwise inclusive-OR"),
  ("^",2,       "BIT_XOR",      "Bitwise exclusive-OR"),
  ("~",1,       "BIT_NOT",      "1's complement"),
  ("%",2,       "MOD",          "Integer modulus"),

  ("<time",2,      "TIME_LT",       "Time less-than comparison"),
  ("<=time",2,     "TIME_LTE",      "Time less-than-or-eq comparison"),
  (">time",2,      "TIME_GT",       "Time greater-than comparison"),
  (">=time",2,     "TIME_GTE",      "Time greater-than-or-eq comparison"),
  ("==time",2,     "TIME_EQ",       "Time compare equality"),
  ("+time",2,      "TIME_PLUS",     "Time addition"),
  ("-time",2,      "TIME_MINUS",    "Time subtraction"),
  ("now",0,        "TIME_NOW",      "The current time token"),

  ("<id",2,      "ID_LT",       "ID less-than comparison"),
  ("<=id",2,     "ID_LTE",      "ID less-than-or-eq comparison"),
  (">id",2,      "ID_GT",       "ID greater-than comparison"),
  (">=id",2,     "ID_GTE",      "ID greater-than-or-eq comparison"),
  ("==id",2,     "ID_EQ",       "ID compare equality"),
  ("+id",2,      "ID_PLUS",     "ID addition"),
  ("distance",2, "ID_DIST",     "ID subtraction"),
  ("<<id",2,	 "ID_LSL",      "ID arithmetic shift left"),
  ("()id",3,	 "ID_BTWOO",    "ID interval open-open containment"),
  ("(]id",2,	 "ID_BTWOC",    "ID interval open-closed containment"),
  ("[)id",3,	 "ID_BTWCO",    "ID interval closed-open containment"),
  ("[]id",2,	 "ID_BTWCC",    "ID interval closed-closed containment"),

  ("<s",2,      "STR_LT",       "String less-than comparison"),
  ("<=s",2,     "STR_LTE",      "String less-than-or-eq comparison"),
  (">s",2,      "STR_GT",       "String greater-than comparison"),
  (">=s",2,     "STR_GTE",      "String greater-than-or-eq comparison"),
  ("==s",2,     "STR_EQ",       "Compare equality"),
  ("strcat",2,	"STR_CAT",      "String concatenation"),
  ("strlen",1,	"STR_LEN",      "String length"),
  ("upper",1,	"STR_UPPER",    "Convert string to upper case"),
  ("lower",1,	"STR_LOWER",    "Convert string to lower case"),
  ("substr",3,	"STR_SUBSTR",   "Extract substring"),
  ("match",2,	"STR_MATCH",    "Perl regular expression matching"),
  ("tostr",1,   "STR_CONV",     "Convert to a string (not ->s)")
  ]:  emit_opcode(op, ar, va, desc)

for op, ar, va, desc in [
  ("neg",1,     "NEG",          "negation"),
  ("+",2,       "PLUS",         "addition"),
  ("-",2,       "MINUS",        "subtraction"),
  ("*",2,       "MUL",          "multiplication"),
  ("/",2,       "DIV",          "division"),
  ("==",2,      "EQ",           "Compare equality"), 
  ("<",2,       "LT",           "less-than comparison"),
  ("<=",2,      "LTE",          "less-than-or-eq comparison"),
  (">",2,       "GT",           "greater-than comparison"),
  (">=",2,      "GTE",          "greater-than-or-eq comparison")
  ]: 
  emit_opcode(op+"i", ar, "INT_" + va, "Integer " + desc)
  emit_opcode(op+"f", ar, "DBL_" + va, "Float " + desc)

for op, ar, va, desc in [
  ("abs",1,     "INT_ABS",      "Absolute value"),
  ("floor",1,   "DBL_FLOOR",    "Next lowest integer"),
  ("ceil",1,    "DBL_CEIL",     "Next highest integer"),
  ("hash",1,	"HASH",         "Generic value hashing")
  ]:
  emit_opcode(op, ar, va, desc)



for i in [ "i32", "u32", "i64", "u64", "dbl", "str", "time", "id" ]:
  emit_opcode("->"+i, 1, "CONV_" + i.upper(), "Convert to type "+i)

warning="""

/*
 * DO NOT EDIT THIS FILE.
 *
 * It is generated by %s
 *
 */
""" % sys.argv[0]


f = open("pel_opcode_decls.gen.h","w+")
f.write(warning)
f.write('public:\n')
map(lambda (n,o,a,v,d): f.write("  static const u_int32_t OP_%s = %d;\n" % (v, n)),
    decls)
f.write('  static const size_t NUM_OPCODES= %d;\n' % curop)

# f.write('private:\n')
map(lambda (n,o,a,v,d): f.write("  void op_%s(u_int32_t inst);\n" % v ),decls)
f.close()


f = open("pel_opcode_defns.gen.h","w+")
f.write(warning)
f.write('static JumpTableEnt_t jump_table[] = {\n')
f.write(string.join(map(lambda (n,o,a,v,d): '  {"%s",\t%d, \t&Pel_VM::op_%s}' % (o,a,v), decls),
                    ',\n'))
f.write('\n};\n')
f.write('#define DEF_OP(_name) void Pel_VM::op_##_name(u_int32_t inst)\n')
f.close()

f = open("pel_opcode_tokens.gen.h","w+")
f.write(warning)
f.write(string.join(map(lambda (n,o,a,v,d): '  {"%s", \tPel_VM::OP_%s}' % (o,v), decls),
                    ',\n'))
f.write('\n')
f.close()

f = open("pel_opcode_descriptions.gen.txt","w+")
f.write(warning)
f.write("mnemonic arity\tdescription\n");
f.write(string.join(map(lambda (n,o,a,v,d): '%s\t%s\t%s' % (o,a,d), decls),
                    ',\n'))
f.write('\n')
f.close()
