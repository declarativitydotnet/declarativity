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
 * DESCRIPTION: Test suite for PEL lexer
 *
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include "pel_lexer.h"
#include "pel_program.h"
#include "pel_vm.h"

#include <iostream>
#include <stdlib.h>


//
// The follow set of tests is for the VM.  Each program should leave
// the given value on the top of the stack.
//
#define _nullv TupleField::NULLV
#define _int32 TupleField::INT32
#define _uint32 TupleField::UINT32
#define _int64 TupleField::INT64
#define _uint64 TupleField::UINT64
#define _string TupleField::STRING
#define _double TupleField::DOUBLE
  

struct ValTest {
  TupleField::Type t;	// Type of expected result
  Pel_VM::Error err;	// Error (0 = success)
  char *val;		// String representation of value
  char *src;		// Source code
};
static const ValTest vtests[] = {
  // Literal values
  { _uint64,Pel_VM::PE_SUCCESS,		"0xffffffffffffffff","0xffffffffffffffffU" },
  // drop
  { _nullv, Pel_VM::PE_STACK_UNDERFLOW, "",	"drop" },
  { _int64, Pel_VM::PE_SUCCESS,		"1",	"1 2 drop"},
  { _double,Pel_VM::PE_SUCCESS,		"1.0",  "1.0 \"Hello\" drop"},
  // swap
  { _int64, Pel_VM::PE_STACK_UNDERFLOW, "",	"1 swap"},
  { _int64, Pel_VM::PE_SUCCESS,		"1",	"1 2 swap"},
  { _int64, Pel_VM::PE_SUCCESS,		"2",	"1 2 swap swap"},
  // dup
  { _int64, Pel_VM::PE_STACK_UNDERFLOW, "",	"1 dup drop drop drop"},
  { _nullv, Pel_VM::PE_SUCCESS,		"",	"1 dup drop drop"},
  { _int64, Pel_VM::PE_SUCCESS, 	"2",	"2 1 dup drop drop"},
  // pop XXX more tests here on the resulting tuple!!
  { _nullv, Pel_VM::PE_STACK_UNDERFLOW, "",	"pop" },
  { _int64, Pel_VM::PE_SUCCESS,		"1",	"1 2 pop"},
  { _nullv, Pel_VM::PE_SUCCESS,		"",     "1.0 \"Hello\" pop pop"},
  { _nullv, Pel_VM::PE_SUCCESS,		"",     "1.0 pop \"Hello\" pop"},
  // not
  { _int32, Pel_VM::PE_STACK_UNDERFLOW, "",	"not" },
  { _int32, Pel_VM::PE_SUCCESS,		"0",     "1 not"},
  { _int32, Pel_VM::PE_SUCCESS,		"1",     "0 not"},
  { _int32, Pel_VM::PE_SUCCESS,		"0",     "300 not"},
  { _int32, Pel_VM::PE_TYPE_CONVERSION,	"0",     "300.0 not"},
  { _int32, Pel_VM::PE_TYPE_CONVERSION,	"0",     "\"Hello\" not"},
  { _int32, Pel_VM::PE_TYPE_CONVERSION,	"0",     "null not"},
  // and
  { _int32, Pel_VM::PE_STACK_UNDERFLOW, "",	"and" },
  { _int32, Pel_VM::PE_STACK_UNDERFLOW, "",	"1 and" },
  { _int32, Pel_VM::PE_SUCCESS,		"0",     "0 0 and"},
  { _int32, Pel_VM::PE_SUCCESS,		"0",     "1 0 and"},
  { _int32, Pel_VM::PE_SUCCESS,		"0",     "0 1 and"},
  { _int32, Pel_VM::PE_SUCCESS,		"1",     "1 1 and"},
  { _int32, Pel_VM::PE_SUCCESS,		"1",     "1 300 and"},
  { _int32, Pel_VM::PE_TYPE_CONVERSION,	"0",     "300.0 1 and"},
  { _int32, Pel_VM::PE_TYPE_CONVERSION,	"0",     "\"Hello\" 1 and"},
  { _int32, Pel_VM::PE_TYPE_CONVERSION,	"0",     "null 1 and"},
  { _int32, Pel_VM::PE_TYPE_CONVERSION,	"0",     "0 300.0 and"},
  { _int32, Pel_VM::PE_TYPE_CONVERSION,	"0",     "1 \"Hello\" and"},
  { _int32, Pel_VM::PE_TYPE_CONVERSION,	"0",     "1 null and"},
  // or
  { _int32, Pel_VM::PE_STACK_UNDERFLOW, "",	"or" },
  { _int32, Pel_VM::PE_STACK_UNDERFLOW, "",	"1 or" },
  { _int32, Pel_VM::PE_SUCCESS,		"0",     "0 0 or"},
  { _int32, Pel_VM::PE_SUCCESS,		"1",     "1 0 or"},
  { _int32, Pel_VM::PE_SUCCESS,		"1",     "0 1 or"},
  { _int32, Pel_VM::PE_SUCCESS,		"1",     "1 1 or"},
  { _int32, Pel_VM::PE_SUCCESS,		"1",     "1 300 or"},
  { _int32, Pel_VM::PE_TYPE_CONVERSION,	"0",     "300.0 1 or"},
  { _int32, Pel_VM::PE_TYPE_CONVERSION,	"0",     "\"Hello\" 1 or"},
  { _int32, Pel_VM::PE_TYPE_CONVERSION,	"0",     "null 1 or"},
  { _int32, Pel_VM::PE_TYPE_CONVERSION,	"0",     "0 300.0 or"},
  { _int32, Pel_VM::PE_TYPE_CONVERSION,	"0",     "1 \"Hello\" or"},
  { _int32, Pel_VM::PE_TYPE_CONVERSION,	"0",     "1 null or"},
  // >> (logical shift right)
  { _uint64,Pel_VM::PE_STACK_UNDERFLOW, "",	">>" },
  { _uint64,Pel_VM::PE_STACK_UNDERFLOW, "",	"1 >>" },
  { _uint64,Pel_VM::PE_SUCCESS,		"0",	"1 1 >>" },
  { _uint64,Pel_VM::PE_SUCCESS,		"1",	"2 1 >>" },
  { _uint64,Pel_VM::PE_SUCCESS,		"4",	"16 2 >>" },
  { _uint64,Pel_VM::PE_SUCCESS,		"0x7fffffffffffffff","0xffffffffffffffffU 1 >>" },
  { _uint64,Pel_VM::PE_SUCCESS,		"0x3fffffffffffffff","-1 2 >>" },

  // >>> (arithmetic shift right)
  { _int64,Pel_VM::PE_STACK_UNDERFLOW, "",	">>>" },
  { _int64,Pel_VM::PE_STACK_UNDERFLOW, "",	"1 >>>" },
  { _int64,Pel_VM::PE_SUCCESS,		"0",	"1 1 >>>" },
  { _int64,Pel_VM::PE_SUCCESS,		"1",	"2 1 >>>" },
  { _int64,Pel_VM::PE_SUCCESS,		"4",	"16 2 >>>" },
  { _int64,Pel_VM::PE_SUCCESS,		"-1",	"0xffffffffffffffffU 1 >>>" },
  { _int64,Pel_VM::PE_SUCCESS,		"-1",	"-1 2 >>>" },
  // << (logical shift left)
  { _uint64,Pel_VM::PE_STACK_UNDERFLOW, "",	"<<" },
  { _uint64,Pel_VM::PE_STACK_UNDERFLOW, "",	"1 <<" },
  { _uint64,Pel_VM::PE_SUCCESS,		"2",	"1 1 <<" },
  { _uint64,Pel_VM::PE_SUCCESS,		"4",	"2 1 <<" },
  { _uint64,Pel_VM::PE_SUCCESS,		"64",	"16 2 <<" },
  { _uint64,Pel_VM::PE_SUCCESS,		"0xfffffffffffffffeU",	"0xffffffffffffffffU 1 <<" },
  { _uint64,Pel_VM::PE_SUCCESS,		"0xfffffffffffffffcU",	"-1 2 <<" },
  // & (bitwise AND)
  { _uint64,Pel_VM::PE_STACK_UNDERFLOW, "",	"&" },
  { _uint64,Pel_VM::PE_STACK_UNDERFLOW, "",	"1 &" },
  { _uint64,Pel_VM::PE_SUCCESS,		"1",	"1 1 &" },
  { _uint64,Pel_VM::PE_SUCCESS,		"0",	"0 3 &" },
  { _uint64,Pel_VM::PE_SUCCESS,		"0",	"3 0 &" },
  { _uint64,Pel_VM::PE_SUCCESS,		"3",	"3 3 &" },
  { _uint64,Pel_VM::PE_SUCCESS,		"3",	"7 3 &" }, 
  { _uint64,Pel_VM::PE_SUCCESS,		"0","0xf0f0f 0xf0f0f0 &" },
  { _uint64,Pel_VM::PE_SUCCESS,		"0",	"0xffffffff00000000U 0xffffffff &" },
  { _uint64,Pel_VM::PE_SUCCESS,		"0x1010101010101010",	"0xffffffffffffffffU 0x1010101010101010 &" },
  // | (bitwise inclusive OR)
  { _uint64,Pel_VM::PE_STACK_UNDERFLOW, "",	"|" },
  { _uint64,Pel_VM::PE_STACK_UNDERFLOW, "",	"1 |" },
  { _uint64,Pel_VM::PE_SUCCESS,		"1",	"1 1 |" },
  { _uint64,Pel_VM::PE_SUCCESS,		"3",	"0 3 |" },
  { _uint64,Pel_VM::PE_SUCCESS,		"3",	"3 0 |" },
  { _uint64,Pel_VM::PE_SUCCESS,		"3",	"3 3 |" },
  { _uint64,Pel_VM::PE_SUCCESS,		"7",	"7 3 |" }, 
  { _uint64,Pel_VM::PE_SUCCESS,		"0xffffff","0xf0f0f 0xf0f0f0 |" },
  { _uint64,Pel_VM::PE_SUCCESS,		"0xffffffffffffffff",	"0xffffffff00000000U 0xffffffff |" },
  { _uint64,Pel_VM::PE_SUCCESS,		"0xffffffffffffffff",	"0xffffffffffffffffU 0x1010101010101010 |" },
  // ^ (bitwise exclusive OR)
  { _uint64,Pel_VM::PE_STACK_UNDERFLOW, "",	"^" },
  { _uint64,Pel_VM::PE_STACK_UNDERFLOW, "",	"1 ^" },
  { _uint64,Pel_VM::PE_SUCCESS,		"0",	"1 1 ^" },
  { _uint64,Pel_VM::PE_SUCCESS,		"3",	"0 3 ^" },
  { _uint64,Pel_VM::PE_SUCCESS,		"3",	"3 0 ^" },
  { _uint64,Pel_VM::PE_SUCCESS,		"0",	"3 3 ^" },
  { _uint64,Pel_VM::PE_SUCCESS,		"4",	"7 3 ^" }, 
  { _uint64,Pel_VM::PE_SUCCESS,		"0xffffff","0xf0f0f 0xf0f0f0 ^" },
  { _uint64,Pel_VM::PE_SUCCESS,		"0xffffffffffffffff",	"0xffffffff00000000U 0xffffffff ^" },
  { _uint64,Pel_VM::PE_SUCCESS,		"0xefefefefefefefef",	"0xffffffffffffffffU 0x1010101010101010 ^" },
  // ~ (bitwise NOT)
  { _uint64,Pel_VM::PE_STACK_UNDERFLOW, "",	"~" },
  { _uint64,Pel_VM::PE_TYPE_CONVERSION, "",	"1.0 ~" },
  { _uint64,Pel_VM::PE_TYPE_CONVERSION, "",	"\"Hello\" ~" },
  { _uint64,Pel_VM::PE_SUCCESS,		"0xfffffffffffffffe",	"1 1 ~" },
  { _uint64,Pel_VM::PE_SUCCESS,		"0xffff0f0f0f0f0f0f","0xf0f0f0f0f0f0 ~" },
  { _uint64,Pel_VM::PE_SUCCESS,		"0", "0xffffffffffffffffU ~" },
  { _uint64,Pel_VM::PE_SUCCESS,		"0xefefefefefefefef",	"0x1010101010101010 ~" },
  // % (integer modulus)
  { _int64,Pel_VM::PE_STACK_UNDERFLOW,  "",	"%" },
  { _int64,Pel_VM::PE_STACK_UNDERFLOW,  "",	"1 %" },
  { _int64,Pel_VM::PE_TYPE_CONVERSION,	"",	"1.0 1 %" },
  { _int64,Pel_VM::PE_TYPE_CONVERSION,	"",	"\"Hello\" 1 %" },
  { _int64,Pel_VM::PE_TYPE_CONVERSION,	"",	"1 1.0 %" },
  { _int64,Pel_VM::PE_DIVIDE_BY_ZERO,	"",	"1 0 %" },
  { _int64,Pel_VM::PE_SUCCESS,		"0",	"0 2 %"},
  { _int64,Pel_VM::PE_SUCCESS,		"0",	"0 1 %"},
  { _int64,Pel_VM::PE_SUCCESS,		"0",	"0 500 %"},
  { _int64,Pel_VM::PE_SUCCESS,		"499",	"499 500 %"},
  { _int64,Pel_VM::PE_SUCCESS,		"1",	"13 2 %"},
  { _int64,Pel_VM::PE_SUCCESS,		"2",	"602 5 %"},
  // <s (string less-than)
  { _int32,Pel_VM::PE_STACK_UNDERFLOW,  "",	"<s" },
  { _int32,Pel_VM::PE_STACK_UNDERFLOW,  "",	"1 <s" },
  { _int32,Pel_VM::PE_TYPE_CONVERSION,	"",	"1.0 \"Hello\" <s" },
  { _int32,Pel_VM::PE_TYPE_CONVERSION,	"",	"1 \"Hello\" <s" },
  { _int32,Pel_VM::PE_TYPE_CONVERSION,	"",	"\"Hello\" 1.0 <s" },
  { _int32,Pel_VM::PE_TYPE_CONVERSION,	"",	"\"Hello\" 1 <s" },
  { _int32,Pel_VM::PE_SUCCESS,		"1",	"\"A\" \"B\" <s" },
  { _int32,Pel_VM::PE_SUCCESS,		"0",	"\"B\" \"A\" <s" },
  { _int32,Pel_VM::PE_SUCCESS,		"0",	"\"A\" \"A\" <s" },
  { _int32,Pel_VM::PE_SUCCESS,		"1",	"\"A\" \"a\" <s" },
  { _int32,Pel_VM::PE_SUCCESS,		"0",	"\"a\" \"A\" <s" },
  { _int32,Pel_VM::PE_SUCCESS,		"0",	"\"AAB\" \"AAA\" <s" },
  { _int32,Pel_VM::PE_SUCCESS,		"1",	"\"AAA\" \"AAB\" <s" },
  // <=s (string less-than-or-equal)
  { _int32,Pel_VM::PE_STACK_UNDERFLOW,  "",	"<=s" },
  { _int32,Pel_VM::PE_STACK_UNDERFLOW,  "",	"1 <=s" },
  { _int32,Pel_VM::PE_TYPE_CONVERSION,	"",	"1.0 \"Hello\" <=s" },
  { _int32,Pel_VM::PE_TYPE_CONVERSION,	"",	"1 \"Hello\" <=s" },
  { _int32,Pel_VM::PE_TYPE_CONVERSION,	"",	"\"Hello\" 1.0 <=s" },
  { _int32,Pel_VM::PE_TYPE_CONVERSION,	"",	"\"Hello\" 1 <=s" },
  { _int32,Pel_VM::PE_SUCCESS,		"1",	"\"A\" \"B\" <=s" },
  { _int32,Pel_VM::PE_SUCCESS,		"0",	"\"B\" \"A\" <=s" },
  { _int32,Pel_VM::PE_SUCCESS,		"1",	"\"A\" \"A\" <=s" },
  { _int32,Pel_VM::PE_SUCCESS,		"1",	"\"A\" \"a\" <=s" },
  { _int32,Pel_VM::PE_SUCCESS,		"0",	"\"a\" \"A\" <=s" },
  { _int32,Pel_VM::PE_SUCCESS,		"0",	"\"AAB\" \"AAA\" <=s" },
  { _int32,Pel_VM::PE_SUCCESS,		"1",	"\"AAA\" \"AAB\" <=s" },
  // >s (string greater-than)
  { _int32,Pel_VM::PE_STACK_UNDERFLOW,  "",	">s" },
  { _int32,Pel_VM::PE_STACK_UNDERFLOW,  "",	"1 >s" },
  { _int32,Pel_VM::PE_TYPE_CONVERSION,	"",	"1.0 \"Hello\" >s" },
  { _int32,Pel_VM::PE_TYPE_CONVERSION,	"",	"1 \"Hello\" >s" },
  { _int32,Pel_VM::PE_TYPE_CONVERSION,	"",	"\"Hello\" 1.0 >s" },
  { _int32,Pel_VM::PE_TYPE_CONVERSION,	"",	"\"Hello\" 1 >s" },
  { _int32,Pel_VM::PE_SUCCESS,		"0",	"\"A\" \"B\" >s" },
  { _int32,Pel_VM::PE_SUCCESS,		"1",	"\"B\" \"A\" >s" },
  { _int32,Pel_VM::PE_SUCCESS,		"0",	"\"A\" \"A\" >s" },
  { _int32,Pel_VM::PE_SUCCESS,		"0",	"\"A\" \"a\" >s" },
  { _int32,Pel_VM::PE_SUCCESS,		"1",	"\"a\" \"A\" >s" },
  { _int32,Pel_VM::PE_SUCCESS,		"1",	"\"AAB\" \"AAA\" >s" },
  { _int32,Pel_VM::PE_SUCCESS,		"0",	"\"AAA\" \"AAB\" >s" },
  // >=s (string greater-than-or-equal)
  { _int32,Pel_VM::PE_STACK_UNDERFLOW,  "",	">=s" },
  { _int32,Pel_VM::PE_STACK_UNDERFLOW,  "",	"1 >=s" },
  { _int32,Pel_VM::PE_TYPE_CONVERSION,	"",	"1.0 \"Hello\" >=s" },
  { _int32,Pel_VM::PE_TYPE_CONVERSION,	"",	"1 \"Hello\" >=s" },
  { _int32,Pel_VM::PE_TYPE_CONVERSION,	"",	"\"Hello\" 1.0 >=s" },
  { _int32,Pel_VM::PE_TYPE_CONVERSION,	"",	"\"Hello\" 1 >=s" },
  { _int32,Pel_VM::PE_SUCCESS,		"0",	"\"A\" \"B\" >=s" },
  { _int32,Pel_VM::PE_SUCCESS,		"1",	"\"B\" \"A\" >=s" },
  { _int32,Pel_VM::PE_SUCCESS,		"1",	"\"A\" \"A\" >=s" },
  { _int32,Pel_VM::PE_SUCCESS,		"0",	"\"A\" \"a\" >=s" },
  { _int32,Pel_VM::PE_SUCCESS,		"1",	"\"a\" \"A\" >=s" },
  { _int32,Pel_VM::PE_SUCCESS,		"1",	"\"AAB\" \"AAA\" >=s" },
  { _int32,Pel_VM::PE_SUCCESS,		"0",	"\"AAA\" \"AAB\" >=s" },
  // ==s (string equality)
  { _int32,Pel_VM::PE_STACK_UNDERFLOW,  "",	"==s" },
  { _int32,Pel_VM::PE_STACK_UNDERFLOW,  "",	"1 ==s" },
  { _int32,Pel_VM::PE_TYPE_CONVERSION,	"",	"1.0 \"Hello\" ==s" },
  { _int32,Pel_VM::PE_TYPE_CONVERSION,	"",	"1 \"Hello\" ==s" },
  { _int32,Pel_VM::PE_TYPE_CONVERSION,	"",	"\"Hello\" 1.0 ==s" },
  { _int32,Pel_VM::PE_TYPE_CONVERSION,	"",	"\"Hello\" 1 ==s" },
  { _int32,Pel_VM::PE_SUCCESS,		"0",	"\"A\" \"B\" ==s" },
  { _int32,Pel_VM::PE_SUCCESS,		"0",	"\"B\" \"A\" ==s" },
  { _int32,Pel_VM::PE_SUCCESS,		"1",	"\"A\" \"A\" ==s" },
  { _int32,Pel_VM::PE_SUCCESS,		"0",	"\"A\" \"a\" ==s" },
  { _int32,Pel_VM::PE_SUCCESS,		"0",	"\"a\" \"A\" ==s" },
  { _int32,Pel_VM::PE_SUCCESS,		"1",	"\"AAA\" \"AAA\" ==s" },
  { _int32,Pel_VM::PE_SUCCESS,		"0",	"\"AAB\" \"AAA\" ==s" },
  { _int32,Pel_VM::PE_SUCCESS,		"0",	"\"AAA\" \"AAB\" ==s" },
  // strcat (string concatenation)
  { _string,Pel_VM::PE_STACK_UNDERFLOW,  "",	"strcat" },
  { _string,Pel_VM::PE_STACK_UNDERFLOW,  "",	"1 strcat" },
  { _string,Pel_VM::PE_TYPE_CONVERSION,	"",	"1.0 \"Hello\" strcat" },
  { _string,Pel_VM::PE_TYPE_CONVERSION,	"",	"1 \"Hello\" strcat" },
  { _string,Pel_VM::PE_TYPE_CONVERSION,	"",	"\"Hello\" 1.0 strcat" },
  { _string,Pel_VM::PE_TYPE_CONVERSION,	"",	"\"Hello\" 1 strcat" },
  { _string,Pel_VM::PE_SUCCESS,		"AB",	"\"A\" \"B\" strcat" },
  { _string,Pel_VM::PE_SUCCESS,		"B",	"\"B\" \"\" strcat" },
  { _string,Pel_VM::PE_SUCCESS,		"B",	"\"\" \"B\" strcat" },
  { _string,Pel_VM::PE_SUCCESS,		"",	"\"\" \"\" strcat" },
  { _string,Pel_VM::PE_SUCCESS,		"Aa",	"\"A\" \"a\" strcat" },
  { _string,Pel_VM::PE_SUCCESS,		"aA",	"\"a\" \"A\" strcat" },
  { _string,Pel_VM::PE_SUCCESS,		"AAAAAA", "\"AAA\" \"AAA\" strcat" },
  // strlen (string length)
  { _uint32,Pel_VM::PE_STACK_UNDERFLOW,  "",	"strlen" },
  { _uint32,Pel_VM::PE_TYPE_CONVERSION,	"",	"1.0 strlen" },
  { _uint32,Pel_VM::PE_TYPE_CONVERSION,	"",	"1 strlen" },
  { _uint32,Pel_VM::PE_SUCCESS,		"1",	"\"A\" strlen" },
  { _uint32,Pel_VM::PE_SUCCESS,		"0",	"\"\" strlen" },
  { _uint32,Pel_VM::PE_SUCCESS,		"1",	"\"\\0\" strlen" },
  { _uint32,Pel_VM::PE_SUCCESS,		"5",	"\"Hello\" strlen" },
  { _uint32,Pel_VM::PE_SUCCESS,		"20","\"String\\nwith\\rcontrols\" strlen" },
  // upper (string to upper case)
  { _string,Pel_VM::PE_STACK_UNDERFLOW,  "",	"upper" },
  { _string,Pel_VM::PE_TYPE_CONVERSION,	"",	"1.0 upper" },
  { _string,Pel_VM::PE_TYPE_CONVERSION,	"",	"1 upper" },
  { _string,Pel_VM::PE_SUCCESS,		"A",	"\"A\" upper" },
  { _string,Pel_VM::PE_SUCCESS,		"A",	"\"a\" upper" },
  { _string,Pel_VM::PE_SUCCESS,		"",	"\"\" upper" },
  { _string,Pel_VM::PE_SUCCESS,		"HELLO", "\"Hello\" upper" },
  { _string,Pel_VM::PE_SUCCESS,		"STRING\nWITH\rCONTROLS","\"String\\nwith\\rcontrols\" upper" },
  // lower (string to lower case)
  { _string,Pel_VM::PE_STACK_UNDERFLOW,  "",	"lower" },
  { _string,Pel_VM::PE_TYPE_CONVERSION,	"",	"1.0 lower" },
  { _string,Pel_VM::PE_TYPE_CONVERSION,	"",	"1 lower" },
  { _string,Pel_VM::PE_SUCCESS,		"a",	"\"A\" lower" },
  { _string,Pel_VM::PE_SUCCESS,		"a",	"\"a\" lower" },
  { _string,Pel_VM::PE_SUCCESS,		"",	"\"\" lower" },
  { _string,Pel_VM::PE_SUCCESS,		"hello", "\"Hello\" lower" },
  { _string,Pel_VM::PE_SUCCESS,		"string\nwith\rcontrols","\"String\\nwith\\rcontrols\" lower" },

  // ->dbl
  { _double,Pel_VM::PE_STACK_UNDERFLOW, "",	"->dbl" },
  { _double,Pel_VM::PE_SUCCESS,		"1",	"1 ->dbl" },
  { _double,Pel_VM::PE_SUCCESS,		"-1",	"-1 ->dbl" },
  { _double,Pel_VM::PE_SUCCESS,		"9999999999999","9999999999999 ->dbl"},
  { _double,Pel_VM::PE_SUCCESS,		"9999999999999","9999999999999 ->dbl"},
  { _double,Pel_VM::PE_SUCCESS,		"1",	"1.0 ->dbl" },
  { _double,Pel_VM::PE_SUCCESS,		"1",	"\"1\" ->dbl" },
  { _double,Pel_VM::PE_SUCCESS,		"0",	"1 not ->dbl" },
  { _double,Pel_VM::PE_TYPE_CONVERSION,	"1",	"null ->dbl" }
  
};
static const size_t num_vtests = sizeof(vtests) / sizeof(ValTest);


//
// The follow table of tests is for the compiler.   Right now, it's
// way too small.  If you discover a bug in the compiler, *first*
// write a test which exposes it, and *then* fix the bug. 
// 
struct CompilerTest {
  char *src;		// Source code
  char *disassembly;	// Expected output of disassembler
  int   num_consts;	// How many constants end up in the pool
  int	num_opcodes;    // How many instructions are generated
};

static const CompilerTest ctests[] = {
  { "  ", "", 0, 0 },
  { "1 2 3", "1 2 3 ", 3, 3},
  { "1 1.2 \"String\" swap dup ", "1 1.2 \"String\" swap dup ", 3, 5},
  { "null\t\n pop $2 $4  ->u32", "null pop $2 $4 ->u32 ", 1, 5},
  { "1 2 /* This is a comment */ pop pop", "1 2 pop pop ", 2, 4}
};
static const size_t num_ctests = sizeof(ctests) / sizeof(CompilerTest);



static double time_fn(cbv cb) 
{
  timespec before_ts;
  timespec after_ts;
  double elapsed;
  
  if (clock_gettime(CLOCK_REALTIME,&before_ts)) {
    fatal << "clock_gettime:" << strerror(errno) << "\n";
  }
  (cb)();
  if (clock_gettime(CLOCK_REALTIME,&after_ts)) {
    fatal << "clock_gettime:" << strerror(errno) << "\n";
  }
  
  after_ts = after_ts - before_ts;
  elapsed = after_ts.tv_sec + (1.0 / 1000 / 1000 / 1000 * after_ts.tv_nsec);
  std::cout << elapsed << " secs (";
  std::cout << after_ts.tv_sec << " secs " << (after_ts.tv_nsec/1000) << " usecs)\n";
  return elapsed;
}

int main(int argc, char **argv)
{
  std::cout << "PEL\n";
  int i;

  //
  // Test the compiler
  //
  for(i = 0; i < num_ctests; i++) {
    const CompilerTest *t = &ctests[i];
    std::cout << "Compiling: " << t->src << "\n";
    Pel_Program *prog = Pel_Lexer::compile( t->src);
    if (prog->ops.size() != t->num_opcodes) {
      std::cerr << "** Bad # opcodes for '" << t->src << "'; " << prog->ops.size() << " instead of expected " << t->num_opcodes << "\n";
    }
    if (prog->const_pool.size() != t->num_consts) {
      std::cerr << "** Bad # consts for '" << t->src << "'; " << prog->const_pool.size() << " instead of expected " << t->num_consts << "\n";
    }
    str dec = Pel_Lexer::decompile(*prog);
    if (dec != t->disassembly) {
      std::cerr << "** Bad disassembly for '" << t->src << "'; '" << dec << "' instead of expected '" << t->disassembly << "'\n";
    }
    delete prog;
  }

  // 
  // Test the VM
  //
  Pel_VM vm;
  TupleRef tpl = Tuple::mk();
  for(i = 0; i < num_vtests; i++) {
    const ValTest *t = &vtests[i];
    std::cout << "Running: " << t->src << "\n";
    
    Pel_Program *prog = Pel_Lexer::compile( t->src);
    Pel_VM::Error e = vm.execute(*prog, tpl);
    if ( e != t->err ) {
      const char *x = Pel_VM::strerror(t->err);
      const char *r = Pel_VM::strerror(e);
      std::cerr << "** Bad error for '" << t->src << "'; '" 
		<< r << "'(" << e << ") instead of expected '" << x << "'\n";
    }
    if ( t->err != Pel_VM::PE_SUCCESS || e != Pel_VM::PE_SUCCESS) {
      continue;
    }

    TupleFieldRef top = vm.result_val();
    if ( top->get_type() != t->t ) {
      std::cerr << "** Bad result type for '" << t->src << "'; '" 
		<< TupleField::typeName(top->get_type()) 
		<< "' instead of expected '" 
		<< TupleField::typeName(t->t) << "'\n";
      continue;
    }
    
    int eq;
    switch (t->t) {
    case _nullv:
      eq = 1; break;
    case _int32: 
      eq = (strtol(t->val,NULL,0)==top->as_i32()); break;
    case _uint32:  
      eq = (strtoul(t->val,NULL,0)==top->as_ui32()); break;
    case _int64: 
      eq = (strtoll(t->val,NULL,0)==top->as_i64()); break;
    case _uint64:  
      eq = (strtoull(t->val,NULL,0)==top->as_ui64()); break;
    case _double:
      eq = (strtod(t->val,NULL)==top->as_d()); break;
    case _string:
      eq = (top->as_s() == t->val ); break;
    default:
      std::cerr << "** Unknown type " << t->t << "\n";
      eq = 1;
    }
    if (!eq) { 
      std::cerr << "** Bad result value for '" 
		<< t->src << "'; " << top->toTypeString() 
		<< " instead of expected " 
		<< t->val << "\n";
    }
  }
  return 0;
}
  

/*
 * End of file 
 */
