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

#include "val_null.h"
#include "val_str.h"
#include "val_int32.h"
#include "val_uint32.h"
#include "val_int64.h"
#include "val_uint64.h"
#include "val_double.h"
#include "val_opaque.h"

//
// The follow set of tests is for the VM.  Each program should leave
// the given value on the top of the stack.
//
#define _nullv Value::NULLV
#define _int32 Value::INT32
#define _uint32 Value::UINT32
#define _int64 Value::INT64
#define _uint64 Value::UINT64
#define _string Value::STR
#define _double Value::DOUBLE
  

struct ValTest {
  Value::TypeCode t;	// Type of expected result
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
  { _uint32,Pel_VM::PE_STACK_UNDERFLOW, "",	"strlen" },
  { _uint32,Pel_VM::PE_TYPE_CONVERSION,	"",	"1.0 strlen" },
  { _uint32,Pel_VM::PE_TYPE_CONVERSION,	"",	"1 strlen" },
  { _uint32,Pel_VM::PE_SUCCESS,		"1",	"\"A\" strlen" },
  { _uint32,Pel_VM::PE_SUCCESS,		"1",	"\"1\" strlen" },
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
  // substr (extract substring)
  { _string,Pel_VM::PE_STACK_UNDERFLOW, "",	"substr" },
  { _string,Pel_VM::PE_STACK_UNDERFLOW, "",	"\"A\" substr" },
  { _string,Pel_VM::PE_STACK_UNDERFLOW, "",	"\"A\" 1 substr" },
  { _string,Pel_VM::PE_TYPE_CONVERSION,	"",	"1 1 1 substr" },
  { _string,Pel_VM::PE_TYPE_CONVERSION,	"",	"1.0 1 1 substr" },
  { _string,Pel_VM::PE_TYPE_CONVERSION,	"",	"\"abcdefg\" 1.0 1 substr" },
  { _string,Pel_VM::PE_TYPE_CONVERSION,	"",	"\"abcdefg\" 1 1.0 substr" },
  { _string,Pel_VM::PE_TYPE_CONVERSION,	"",	"\"abcdefg\" \"a\" 1 substr" },
  { _string,Pel_VM::PE_TYPE_CONVERSION,	"",	"\"abcdefg\" 1 \"a\" substr" },
  { _string,Pel_VM::PE_TYPE_CONVERSION,	"",	"\"abcdefg\" 1 null substr" },
  { _string,Pel_VM::PE_TYPE_CONVERSION,	"",	"\"abcdefg\" null 1 substr" },
  { _string,Pel_VM::PE_TYPE_CONVERSION,	"",	"null 1 1 substr" },
  { _string,Pel_VM::PE_SUCCESS,		"abc",	"\"abcdefg\" 0 3 substr" },
  { _string,Pel_VM::PE_SUCCESS,		"bcde","\"abcdefg\" 1 4 substr" },
  { _string,Pel_VM::PE_SUCCESS,		"abcdefg","\"abcdefg\" 0 7 substr" },
  { _string,Pel_VM::PE_SUCCESS,		"fg",	"\"abcdefg\" 5 8 substr" },
  { _string,Pel_VM::PE_SUCCESS,		"",	"\"abcdefg\" 0 0 substr" },
  { _string,Pel_VM::PE_SUCCESS,		"",	"\"abcdefg\" 4 0 substr" },
  { _string,Pel_VM::PE_SUCCESS,		"",	"\"abcdefg\" 10 0 substr" },
  { _string,Pel_VM::PE_SUCCESS,		"",	"\"abcdefg\" 10 3 substr" },
  // match (Perl regular expression matching: not much testing here
  // yet :-( )
  { _int32,Pel_VM::PE_STACK_UNDERFLOW,  "",	"match" },
  { _int32,Pel_VM::PE_STACK_UNDERFLOW,  "",	"\"A\" match" },
  { _int32,Pel_VM::PE_TYPE_CONVERSION,  "",	"\"A\" 1 match" },
  { _int32,Pel_VM::PE_TYPE_CONVERSION, 	"",	"1 1 match" },
  { _int32,Pel_VM::PE_TYPE_CONVERSION, 	"",	"1.0 1 match" },
  { _int32,Pel_VM::PE_TYPE_CONVERSION, 	"",	"\"abcdefg\" 1.0 match" },
  { _int32,Pel_VM::PE_TYPE_CONVERSION, 	"",	"\"abcdefg\" 1 match" },
  { _int32,Pel_VM::PE_TYPE_CONVERSION, 	"",	"\"abcdefg\" null match" },
  { _int32,Pel_VM::PE_TYPE_CONVERSION, 	"",	"null \"A\" match" },
  { _int32,Pel_VM::PE_SUCCESS,		"1",	"\"abcd\" \"abcd\" match" },
  { _int32,Pel_VM::PE_SUCCESS,		"1",	"\"abcd\" \"ab.*\" match" },
  { _int32,Pel_VM::PE_SUCCESS,		"1",	"\"abcd\" \".*cd\" match" },
  { _int32,Pel_VM::PE_SUCCESS,		"0",	"\"abcd\" \"ab\" match" },
  { _int32,Pel_VM::PE_SUCCESS,		"0",	"\"abcd\" \"cd\" match" },
  // hash (hashing a string to 32 bits)
  { _uint32,Pel_VM::PE_STACK_UNDERFLOW,  "",	"hash" },
  { _uint32,Pel_VM::PE_SUCCESS,		"0xAED0B875","1 hash" },
  { _uint32,Pel_VM::PE_SUCCESS,		"0x615D13BA","1.0 hash" },
  { _uint32,Pel_VM::PE_SUCCESS,		"0xE2A0E8FE","null hash" },
  { _uint32,Pel_VM::PE_SUCCESS,		"0xBCF5098A","\"A\" hash" },
  { _uint32,Pel_VM::PE_SUCCESS,		"0x81D8E12B","\"\" hash" },
  { _uint32,Pel_VM::PE_SUCCESS,		"0x87364106","\"Hello, world!\" hash" },
  // negi (integer unary negation)
  { _int64,Pel_VM::PE_STACK_UNDERFLOW, "",	"negi" },
  { _int64,Pel_VM::PE_TYPE_CONVERSION, "",	"\"A\" negi" },
  { _int64,Pel_VM::PE_TYPE_CONVERSION,	"",	"1.0 negi" },
  { _int64,Pel_VM::PE_TYPE_CONVERSION,	"",	"null negi" },
  { _int64,Pel_VM::PE_SUCCESS,		"-1",	"1 negi" },
  { _int64,Pel_VM::PE_SUCCESS,		"1",	"-1 negi" },
  { _int64,Pel_VM::PE_SUCCESS,		"-2000","2000 negi" },
  { _int64,Pel_VM::PE_SUCCESS,		"2000", "-2000 negi" },
  { _int64,Pel_VM::PE_SUCCESS,		"0",	"0 negi" },
  // +i (integer addition)
  { _int64,Pel_VM::PE_STACK_UNDERFLOW,  "",	"+i" },
  { _int64,Pel_VM::PE_STACK_UNDERFLOW,  "",	"1 +i" },
  { _int64,Pel_VM::PE_TYPE_CONVERSION,	"",	"1 \"Hello\" +i" },
  { _int64,Pel_VM::PE_TYPE_CONVERSION,	"",	"\"Hello\"  1 +i" },
  { _int64,Pel_VM::PE_TYPE_CONVERSION,	"",	"1.0 1 +i" },
  { _int64,Pel_VM::PE_TYPE_CONVERSION,	"",	"1 1.0 +i" },
  { _int64,Pel_VM::PE_SUCCESS,		"3",	"1 2 +i" },
  { _int64,Pel_VM::PE_SUCCESS,		"0",	"1 -1 +i" },
  { _int64,Pel_VM::PE_SUCCESS,		"0x100000000","0xffffffff 1 +i" },
  { _int64,Pel_VM::PE_SUCCESS,		"-2",	"0xffffffffffffffffU -1 +i" },
  // -i (integer subtraction)
  { _int64,Pel_VM::PE_STACK_UNDERFLOW,  "",	"-i" },
  { _int64,Pel_VM::PE_STACK_UNDERFLOW,  "",	"1 -i" },
  { _int64,Pel_VM::PE_TYPE_CONVERSION,	"",	"1 \"Hello\" -i" },
  { _int64,Pel_VM::PE_TYPE_CONVERSION,	"",	"\"Hello\"  1 -i" },
  { _int64,Pel_VM::PE_TYPE_CONVERSION,	"",	"1.0 1 -i" },
  { _int64,Pel_VM::PE_TYPE_CONVERSION,	"",	"1 1.0 -i" },
  { _int64,Pel_VM::PE_SUCCESS,		"-1",	"1 2 -i" },
  { _int64,Pel_VM::PE_SUCCESS,		"2",	"1 -1 -i" },
  { _int64,Pel_VM::PE_SUCCESS,		"0",	"0xffffffffffffffffU -1 -i" },
  // *i (integer multiplication)
  { _int64,Pel_VM::PE_STACK_UNDERFLOW,  "",	"*i" },
  { _int64,Pel_VM::PE_STACK_UNDERFLOW,  "",	"1 *i" },
  { _int64,Pel_VM::PE_TYPE_CONVERSION,	"",	"1 \"Hello\" *i" },
  { _int64,Pel_VM::PE_TYPE_CONVERSION,	"",	"\"Hello\"  1 *i" },
  { _int64,Pel_VM::PE_TYPE_CONVERSION,	"",	"1.0 1 *i" },
  { _int64,Pel_VM::PE_TYPE_CONVERSION,	"",	"1 1.0 *i" },
  { _int64,Pel_VM::PE_SUCCESS,		"2",	"1 2 *i" },
  { _int64,Pel_VM::PE_SUCCESS,		"1",	"-1 -1 *i" },
  { _int64,Pel_VM::PE_SUCCESS,		"-2",	"0xffffffffffffffffU 2 *i" },
  // /i (integer division)
  { _int64,Pel_VM::PE_STACK_UNDERFLOW,  "",	"/i" },
  { _int64,Pel_VM::PE_STACK_UNDERFLOW,  "",	"1 /i" },
  { _int64,Pel_VM::PE_TYPE_CONVERSION,	"",	"1 \"Hello\" /i" },
  { _int64,Pel_VM::PE_TYPE_CONVERSION,	"",	"\"Hello\"  1 /i" },
  { _int64,Pel_VM::PE_TYPE_CONVERSION,	"",	"1.0 1 /i" },
  { _int64,Pel_VM::PE_TYPE_CONVERSION,	"",	"1 1.0 /i" },
  { _int64,Pel_VM::PE_DIVIDE_BY_ZERO,	"",	"2 0 /i" },
  { _int64,Pel_VM::PE_SUCCESS,		"4",	"8 2 /i" },
  { _int64,Pel_VM::PE_SUCCESS,		"5",	"11 2 /i" },
  { _int64,Pel_VM::PE_SUCCESS,		"0",	"0 30 /i" },
  { _int64,Pel_VM::PE_SUCCESS,		"0",	"0xffffffffffffffffU 3 /i" },
  // ==i (integer equal)
  { _int32,Pel_VM::PE_STACK_UNDERFLOW,  "",	"==i" },
  { _int32,Pel_VM::PE_STACK_UNDERFLOW,  "",	"1 ==i" },
  { _int32,Pel_VM::PE_TYPE_CONVERSION,	"",	"1 \"Hello\" ==i" },
  { _int32,Pel_VM::PE_TYPE_CONVERSION,	"",	"\"Hello\"  1 ==i" },
  { _int32,Pel_VM::PE_TYPE_CONVERSION,	"",	"1.0 1 ==i" },
  { _int32,Pel_VM::PE_TYPE_CONVERSION,	"",	"1 1.0 ==i" },
  { _int32,Pel_VM::PE_SUCCESS,		"0",	"1 2 ==i" },
  { _int32,Pel_VM::PE_SUCCESS,		"1",	"1 1 ==i" },
  { _int32,Pel_VM::PE_SUCCESS,		"1",	"0xffffffffffffffffU -1 ==i" },
  // >i (integer greater-than)
  { _int32,Pel_VM::PE_STACK_UNDERFLOW,  "",	">i" },
  { _int32,Pel_VM::PE_STACK_UNDERFLOW,  "",	"1 >i" },
  { _int32,Pel_VM::PE_TYPE_CONVERSION,	"",	"1 \"Hello\" >i" },
  { _int32,Pel_VM::PE_TYPE_CONVERSION,	"",	"\"Hello\"  1 >i" },
  { _int32,Pel_VM::PE_TYPE_CONVERSION,	"",	"1.0 1 >i" },
  { _int32,Pel_VM::PE_TYPE_CONVERSION,	"",	"1 1.0 >i" },
  { _int32,Pel_VM::PE_SUCCESS,		"0",	"1 2 >i" },
  { _int32,Pel_VM::PE_SUCCESS,		"1",	"2 1 >i" },
  { _int32,Pel_VM::PE_SUCCESS,		"0",	"1 1 >i" },
  { _int32,Pel_VM::PE_SUCCESS,		"1",	"-1 -2 >i" },
  { _int32,Pel_VM::PE_SUCCESS,		"0",	"-2 -1 >i" },
  { _int32,Pel_VM::PE_SUCCESS,		"0",	"0xffffffffffffffffU 2 >i" },
  // >=i (integer greater-than-or-equal)
  { _int32,Pel_VM::PE_STACK_UNDERFLOW,  "",	">=i" },
  { _int32,Pel_VM::PE_STACK_UNDERFLOW,  "",	"1 >=i" },
  { _int32,Pel_VM::PE_TYPE_CONVERSION,	"",	"1 \"Hello\" >=i" },
  { _int32,Pel_VM::PE_TYPE_CONVERSION,	"",	"\"Hello\"  1 >=i" },
  { _int32,Pel_VM::PE_TYPE_CONVERSION,	"",	"1.0 1 >=i" },
  { _int32,Pel_VM::PE_TYPE_CONVERSION,	"",	"1 1.0 >=i" },
  { _int32,Pel_VM::PE_SUCCESS,		"0",	"1 2 >=i" },
  { _int32,Pel_VM::PE_SUCCESS,		"1",	"2 1 >=i" },
  { _int32,Pel_VM::PE_SUCCESS,		"1",	"1 1 >=i" },
  { _int32,Pel_VM::PE_SUCCESS,		"1",	"-1 -2 >=i" },
  { _int32,Pel_VM::PE_SUCCESS,		"0",	"-2 -1 >=i" },
  { _int32,Pel_VM::PE_SUCCESS,		"0",	"0xffffffffffffffffU 2 >=i" },
  // <i (integer less-than)
  { _int32,Pel_VM::PE_STACK_UNDERFLOW,  "",	"<i" },
  { _int32,Pel_VM::PE_STACK_UNDERFLOW,  "",	"1 <i" },
  { _int32,Pel_VM::PE_TYPE_CONVERSION,	"",	"1 \"Hello\" <i" },
  { _int32,Pel_VM::PE_TYPE_CONVERSION,	"",	"\"Hello\"  1 <i" },
  { _int32,Pel_VM::PE_TYPE_CONVERSION,	"",	"1.0 1 <i" },
  { _int32,Pel_VM::PE_TYPE_CONVERSION,	"",	"1 1.0 <i" },
  { _int32,Pel_VM::PE_SUCCESS,		"1",	"1 2 <i" },
  { _int32,Pel_VM::PE_SUCCESS,		"0",	"2 1 <i" },
  { _int32,Pel_VM::PE_SUCCESS,		"0",	"1 1 <i" },
  { _int32,Pel_VM::PE_SUCCESS,		"0",	"-1 -2 <i" },
  { _int32,Pel_VM::PE_SUCCESS,		"1",	"-2 -1 <i" },
  { _int32,Pel_VM::PE_SUCCESS,		"1",	"0xffffffffffffffffU 2 <i" },
  // <=i (integer less-than-or-equal)
  { _int32,Pel_VM::PE_STACK_UNDERFLOW,  "",	"<=i" },
  { _int32,Pel_VM::PE_STACK_UNDERFLOW,  "",	"1 <=i" },
  { _int32,Pel_VM::PE_TYPE_CONVERSION,	"",	"1 \"Hello\" <=i" },
  { _int32,Pel_VM::PE_TYPE_CONVERSION,	"",	"\"Hello\"  1 <=i" },
  { _int32,Pel_VM::PE_TYPE_CONVERSION,	"",	"1.0 1 <=i" },
  { _int32,Pel_VM::PE_TYPE_CONVERSION,	"",	"1 1.0 <=i" },
  { _int32,Pel_VM::PE_SUCCESS,		"1",	"1 2 <=i" },
  { _int32,Pel_VM::PE_SUCCESS,		"0",	"2 1 <=i" },
  { _int32,Pel_VM::PE_SUCCESS,		"1",	"1 1 <=i" },
  { _int32,Pel_VM::PE_SUCCESS,		"0",	"-1 -2 <=i" },
  { _int32,Pel_VM::PE_SUCCESS,		"1",	"-2 -1 <=i" },
  { _int32,Pel_VM::PE_SUCCESS,		"1",	"0xffffffffffffffffU 2 <=i" },
  // negf (floating-point unary negation)
  { _double,Pel_VM::PE_STACK_UNDERFLOW, "",	"negf" },
  { _double,Pel_VM::PE_TYPE_CONVERSION, "",	"\"A\" negf" },
  { _double,Pel_VM::PE_TYPE_CONVERSION,	"",	"1 negf" },
  { _double,Pel_VM::PE_TYPE_CONVERSION,	"",	"null negf" },
  { _double,Pel_VM::PE_SUCCESS,		"-1",	"1.0 negf" },
  { _double,Pel_VM::PE_SUCCESS,		"1",	"-1.0 negf" },
  { _double,Pel_VM::PE_SUCCESS,		"-2000.5","2000.5 negf" },
  { _double,Pel_VM::PE_SUCCESS,		"2000.5", "-2000.5 negf" },
  { _double,Pel_VM::PE_SUCCESS,		"0",	"0.0 negf" },
  // +f (floating-point addition)
  { _double,Pel_VM::PE_STACK_UNDERFLOW,  "",	"+f" },
  { _double,Pel_VM::PE_STACK_UNDERFLOW,  "",	"1 +f" },
  { _double,Pel_VM::PE_TYPE_CONVERSION,	"",	"1 \"Hello\" +f" },
  { _double,Pel_VM::PE_TYPE_CONVERSION,	"",	"\"Hello\"  1 +f" },
  { _double,Pel_VM::PE_TYPE_CONVERSION,	"",	"1.0 1 +f" },
  { _double,Pel_VM::PE_TYPE_CONVERSION,	"",	"1 1.0 +f" },
  { _double,Pel_VM::PE_SUCCESS,		"3.0",	"1.0 2.0 +f" },
  { _double,Pel_VM::PE_SUCCESS,		"0",	"1.5 -1.5 +f" },
  { _double,Pel_VM::PE_SUCCESS,		"10000.00005",	"10000.0 0.00005 +f" },
  // -f (floating-point subtraction)
  { _double,Pel_VM::PE_STACK_UNDERFLOW,  "",	"-f" },
  { _double,Pel_VM::PE_STACK_UNDERFLOW,  "",	"1 -f" },
  { _double,Pel_VM::PE_TYPE_CONVERSION,	"",	"1 \"Hello\" -f" },
  { _double,Pel_VM::PE_TYPE_CONVERSION,	"",	"\"Hello\"  1 -f" },
  { _double,Pel_VM::PE_TYPE_CONVERSION,	"",	"1.0 1 -f" },
  { _double,Pel_VM::PE_TYPE_CONVERSION,	"",	"1 1.0 -f" },
  { _double,Pel_VM::PE_SUCCESS,		"-1.0",	"1.0 2.0 -f" },
  { _double,Pel_VM::PE_SUCCESS,		"3",	"1.5 -1.5 -f" },
  { _double,Pel_VM::PE_SUCCESS,		"9999.99995",	"10000.0 0.00005 -f" },
  // *f (floating-point multiplication)
  { _double,Pel_VM::PE_STACK_UNDERFLOW,  "",	"*f" },
  { _double,Pel_VM::PE_STACK_UNDERFLOW,  "",	"1 *f" },
  { _double,Pel_VM::PE_TYPE_CONVERSION,	"",	"1 \"Hello\" *f" },
  { _double,Pel_VM::PE_TYPE_CONVERSION,	"",	"\"Hello\"  1 *f" },
  { _double,Pel_VM::PE_TYPE_CONVERSION,	"",	"1.0 1 *f" },
  { _double,Pel_VM::PE_TYPE_CONVERSION,	"",	"1 1.0 *f" },
  { _double,Pel_VM::PE_SUCCESS,		"2",	"1.0 2.0 *f" },
  { _double,Pel_VM::PE_SUCCESS,		"1",	"-1.0 -1.0 *f" },
  { _double,Pel_VM::PE_SUCCESS,		"6.5",	"13.0 0.5 *f" },
  { _double,Pel_VM::PE_SUCCESS,		"0",	"-1.4553 0.0 *f" },
  // /f (floating-point division)
  { _double,Pel_VM::PE_STACK_UNDERFLOW,  "",	"/f" },
  { _double,Pel_VM::PE_STACK_UNDERFLOW,  "",	"1 /f" },
  { _double,Pel_VM::PE_TYPE_CONVERSION,	"",	"1 \"Hello\" /f" },
  { _double,Pel_VM::PE_TYPE_CONVERSION,	"",	"\"Hello\"  1 /f" },
  { _double,Pel_VM::PE_TYPE_CONVERSION,	"",	"1.0 1 /f" },
  { _double,Pel_VM::PE_TYPE_CONVERSION,	"",	"1 1.0 /f" },
  { _double,Pel_VM::PE_SUCCESS,		"inf",	"2.0 0.0 /f" },
  { _double,Pel_VM::PE_SUCCESS,		"4",	"8.0 2.0 /f" },
  { _double,Pel_VM::PE_SUCCESS,		"5.5",	"11.0 2.0 /f" },
  { _double,Pel_VM::PE_SUCCESS,		"0",	"0.0 30.0 /f" },
  // ==f (floating-point equal)
  { _int32,Pel_VM::PE_STACK_UNDERFLOW,  "",	"==f" },
  { _int32,Pel_VM::PE_STACK_UNDERFLOW,  "",	"1 ==f" },
  { _int32,Pel_VM::PE_TYPE_CONVERSION,	"",	"1 \"Hello\" ==f" },
  { _int32,Pel_VM::PE_TYPE_CONVERSION,	"",	"\"Hello\"  1 ==f" },
  { _int32,Pel_VM::PE_TYPE_CONVERSION,	"",	"1.0 1 ==f" },
  { _int32,Pel_VM::PE_TYPE_CONVERSION,	"",	"1 1.0 ==f" },
  { _int32,Pel_VM::PE_SUCCESS,		"0",	"1.0 2.0 ==f" },
  { _int32,Pel_VM::PE_SUCCESS,		"1",	"1.0 1.0 ==f" },
  // >f (floating-point greater-than)
  { _int32,Pel_VM::PE_STACK_UNDERFLOW,  "",	">f" },
  { _int32,Pel_VM::PE_STACK_UNDERFLOW,  "",	"1 >f" },
  { _int32,Pel_VM::PE_TYPE_CONVERSION,	"",	"1 \"Hello\" >f" },
  { _int32,Pel_VM::PE_TYPE_CONVERSION,	"",	"\"Hello\"  1 >f" },
  { _int32,Pel_VM::PE_TYPE_CONVERSION,	"",	"1.0 1 >f" },
  { _int32,Pel_VM::PE_TYPE_CONVERSION,	"",	"1 1.0 >f" },
  { _int32,Pel_VM::PE_SUCCESS,		"0",	"1.0 2.0 >f" },
  { _int32,Pel_VM::PE_SUCCESS,		"1",	"2.0 1.1 >f" },
  { _int32,Pel_VM::PE_SUCCESS,		"0",	"1.2 1.2 >f" },
  { _int32,Pel_VM::PE_SUCCESS,		"1",	"-1.34 -2.45 >f" },
  { _int32,Pel_VM::PE_SUCCESS,		"0",	"-2.78 -1.003  >f" },
  // >=f (floating-point greater-than-or-equal)
  { _int32,Pel_VM::PE_STACK_UNDERFLOW,  "",	">=f" },
  { _int32,Pel_VM::PE_STACK_UNDERFLOW,  "",	"1 >=f" },
  { _int32,Pel_VM::PE_TYPE_CONVERSION,	"",	"1 \"Hello\" >=f" },
  { _int32,Pel_VM::PE_TYPE_CONVERSION,	"",	"\"Hello\"  1 >=f" },
  { _int32,Pel_VM::PE_TYPE_CONVERSION,	"",	"1.0 1 >=f" },
  { _int32,Pel_VM::PE_TYPE_CONVERSION,	"",	"1 1.0 >=f" },
  { _int32,Pel_VM::PE_SUCCESS,		"0",	"1.0 2.0 >=f" },
  { _int32,Pel_VM::PE_SUCCESS,		"1",	"2.0 1.1 >=f" },
  { _int32,Pel_VM::PE_SUCCESS,		"1",	"1.2 1.2 >=f" },
  { _int32,Pel_VM::PE_SUCCESS,		"1",	"-1.34 -2.45 >=f" },
  { _int32,Pel_VM::PE_SUCCESS,		"0",	"-2.78 -1.003  >=f" },
  // <f (floating-point less-than)
  { _int32,Pel_VM::PE_STACK_UNDERFLOW,  "",	"<f" },
  { _int32,Pel_VM::PE_STACK_UNDERFLOW,  "",	"1 <f" },
  { _int32,Pel_VM::PE_TYPE_CONVERSION,	"",	"1 \"Hello\" <f" },
  { _int32,Pel_VM::PE_TYPE_CONVERSION,	"",	"\"Hello\"  1 <f" },
  { _int32,Pel_VM::PE_TYPE_CONVERSION,	"",	"1.0 1 <f" },
  { _int32,Pel_VM::PE_TYPE_CONVERSION,	"",	"1 1.0 <f" },
  { _int32,Pel_VM::PE_SUCCESS,		"1",	"1.0 2.0 <f" },
  { _int32,Pel_VM::PE_SUCCESS,		"0",	"2.0 1.1 <f" },
  { _int32,Pel_VM::PE_SUCCESS,		"0",	"1.2 1.2 <f" },
  { _int32,Pel_VM::PE_SUCCESS,		"0",	"-1.34 -2.45 <f" },
  { _int32,Pel_VM::PE_SUCCESS,		"1",	"-2.78 -1.003  <f" },
  // <=f (floating-point less-than-or-equal)
  { _int32,Pel_VM::PE_STACK_UNDERFLOW,  "",	"<=f" },
  { _int32,Pel_VM::PE_STACK_UNDERFLOW,  "",	"1 <=f" },
  { _int32,Pel_VM::PE_TYPE_CONVERSION,	"",	"1 \"Hello\" <=f" },
  { _int32,Pel_VM::PE_TYPE_CONVERSION,	"",	"\"Hello\"  1 <=f" },
  { _int32,Pel_VM::PE_TYPE_CONVERSION,	"",	"1.0 1 <=f" },
  { _int32,Pel_VM::PE_TYPE_CONVERSION,	"",	"1 1.0 <=f" },
  { _int32,Pel_VM::PE_SUCCESS,		"1",	"1.0 2.0 <=f" },
  { _int32,Pel_VM::PE_SUCCESS,		"0",	"2.0 1.1 <=f" },
  { _int32,Pel_VM::PE_SUCCESS,		"1",	"1.2 1.2 <=f" },
  { _int32,Pel_VM::PE_SUCCESS,		"0",	"-1.34 -2.45 <=f" },
  { _int32,Pel_VM::PE_SUCCESS,		"1",	"-2.78 -1.003  <=f" },
  // abs
  { _int64,Pel_VM::PE_STACK_UNDERFLOW, "",	"abs" },
  { _int64,Pel_VM::PE_TYPE_CONVERSION, "",	"\"A\" abs" },
  { _int64,Pel_VM::PE_TYPE_CONVERSION,	"",	"1.0 abs" },
  { _int64,Pel_VM::PE_TYPE_CONVERSION,	"",	"null abs" },
  { _int64,Pel_VM::PE_SUCCESS,		"1",	"1 abs" },
  { _int64,Pel_VM::PE_SUCCESS,		"1",	"-1 abs" },
  { _int64,Pel_VM::PE_SUCCESS,		"2000",	"2000 abs" },
  { _int64,Pel_VM::PE_SUCCESS,		"2000",	"-2000 abs" },
  { _int64,Pel_VM::PE_SUCCESS,		"0",	"0 abs" },
  // floor
  { _double,Pel_VM::PE_STACK_UNDERFLOW, "",	"floor" },
  { _double,Pel_VM::PE_TYPE_CONVERSION, "",	"\"A\" floor" },
  { _double,Pel_VM::PE_TYPE_CONVERSION,	"",	"1 floor" },
  { _double,Pel_VM::PE_TYPE_CONVERSION,	"",	"null floor" },
  { _double,Pel_VM::PE_SUCCESS,		"1",	"1.0 floor" },
  { _double,Pel_VM::PE_SUCCESS,		"-1",	"-1.0 floor" },
  { _double,Pel_VM::PE_SUCCESS,		"1",	" 1.5 floor" },
  { _double,Pel_VM::PE_SUCCESS,		"-2",	"-1.5 floor" },
  { _double,Pel_VM::PE_SUCCESS,		"0",	"0.0 floor" },
  // ceil
  { _double,Pel_VM::PE_STACK_UNDERFLOW, "",	"ceil" },
  { _double,Pel_VM::PE_TYPE_CONVERSION, "",	"\"A\" ceil" },
  { _double,Pel_VM::PE_TYPE_CONVERSION,	"",	"1 ceil" },
  { _double,Pel_VM::PE_TYPE_CONVERSION,	"",	"null ceil" },
  { _double,Pel_VM::PE_SUCCESS,		"1",	"1.0 ceil" },
  { _double,Pel_VM::PE_SUCCESS,		"-1",	"-1.0 ceil" },
  { _double,Pel_VM::PE_SUCCESS,		"2",	" 1.5 ceil" },
  { _double,Pel_VM::PE_SUCCESS,		"-1",	"-1.5 ceil" },
  { _double,Pel_VM::PE_SUCCESS,		"0",	"0.0 ceil" },
  // ->i32
  { _int32,Pel_VM::PE_STACK_UNDERFLOW, "",	"->i32" },
  { _int32,Pel_VM::PE_SUCCESS,		"1",	"1 ->i32" },
  { _int32,Pel_VM::PE_SUCCESS,		"-1",	"-1 ->i32" },
  { _int32,Pel_VM::PE_SUCCESS,		"1316134911","9999999999999 ->i32"},
  { _int32,Pel_VM::PE_SUCCESS,		"1316134911","9999999999999 ->i32"},
  { _int32,Pel_VM::PE_SUCCESS,		"1",	"1.0 ->i32" },
  { _int32,Pel_VM::PE_SUCCESS,		"1",	"\"1\" ->i32" },
  { _int32,Pel_VM::PE_SUCCESS,		"0",	"1 not ->i32" },
  { _int32,Pel_VM::PE_TYPE_CONVERSION,	"",	"null ->i32" },
  // ->u32
  { _uint32,Pel_VM::PE_STACK_UNDERFLOW, "",	"->u32" },
  { _uint32,Pel_VM::PE_SUCCESS,		"1",	"1 ->u32" },
  { _uint32,Pel_VM::PE_SUCCESS,		"0xffffffffUL","-1 ->u32" },
  { _uint32,Pel_VM::PE_SUCCESS,		"1316134911","9999999999999 ->u32"},
  { _uint32,Pel_VM::PE_SUCCESS,		"1316134911","9999999999999 ->u32"},
  { _uint32,Pel_VM::PE_SUCCESS,		"1",	"1.0 ->u32" },
  { _uint32,Pel_VM::PE_SUCCESS,		"1",	"\"1\" ->u32" },
  { _uint32,Pel_VM::PE_SUCCESS,		"0",	"1 not ->u32" },
  { _uint32,Pel_VM::PE_TYPE_CONVERSION,	"",	"null ->u32" },
  // ->i64
  { _int64,Pel_VM::PE_STACK_UNDERFLOW, "",	"->i64" },
  { _int64,Pel_VM::PE_SUCCESS,		"1",	"1 ->i64" },
  { _int64,Pel_VM::PE_SUCCESS,		"-1",	"-1 ->i64" },
  { _int64,Pel_VM::PE_SUCCESS,		"9999999999999","9999999999999 ->i64"},
  { _int64,Pel_VM::PE_SUCCESS,		"9999999999999","9999999999999 ->i64"},
  { _int64,Pel_VM::PE_SUCCESS,		"1",	"1.0 ->i64" },
  { _int64,Pel_VM::PE_SUCCESS,		"1",	"\"1\" ->i64" },
  { _int64,Pel_VM::PE_SUCCESS,		"0",	"1 not ->i64" },
  { _int64,Pel_VM::PE_TYPE_CONVERSION,	"",	"null ->i64" },
  // ->u64
  { _uint64,Pel_VM::PE_STACK_UNDERFLOW, "",	"->u64" },
  { _uint64,Pel_VM::PE_SUCCESS,		"1",	"1 ->u64" },
  { _uint64,Pel_VM::PE_SUCCESS,		"0xffffffffffffffffULL","-1 ->u64" },
  { _uint64,Pel_VM::PE_SUCCESS,		"9999999999999","9999999999999 ->u64"},
  { _uint64,Pel_VM::PE_SUCCESS,		"9999999999999","9999999999999 ->u64"},
  { _uint64,Pel_VM::PE_SUCCESS,		"1",	"1.0 ->u64" },
  { _uint64,Pel_VM::PE_SUCCESS,		"1",	"\"1\" ->u64" },
  { _uint64,Pel_VM::PE_SUCCESS,		"0",	"1 not ->u64" },
  { _uint64,Pel_VM::PE_TYPE_CONVERSION,	"",	"null ->u64" },
  // ->str
  { _string,Pel_VM::PE_STACK_UNDERFLOW, "",	"->str" },
  { _string,Pel_VM::PE_SUCCESS,		"1",	"1 ->str" },
  { _string,Pel_VM::PE_SUCCESS,		"-1",	"-1 ->str" },
  { _string,Pel_VM::PE_SUCCESS,		"9999999999999","9999999999999 ->str"},
  { _string,Pel_VM::PE_SUCCESS,		"9999999999999","9999999999999 ->str"},
  { _string,Pel_VM::PE_SUCCESS,		"1",	"1.0 ->str" },
  { _string,Pel_VM::PE_SUCCESS,		"1",	"\"1\" ->str" },
 { _string,Pel_VM::PE_SUCCESS,		"0",	"1 not ->str" },
  { _string,Pel_VM::PE_SUCCESS,		"null",	"null ->str" },
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


#if 0
// Not used
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
#endif

void vm_test(Pel_VM &vm, TupleRef tpl, int i) {
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
    return;
  }
  
  ValueRef top = vm.result_val();
  if ( top->typeCode() != t->t ) {
    std::cerr << "** Bad result type for '" << t->src << "'; '" 
	      << top->typeName()
	      << "' instead of expected '" 
	      << t->t << "'\n";
    return;
  }
  
  int eq;
  switch (t->t) {
  case _nullv:
    eq = 1; break;
  case _int32: 
    eq = (strtol(t->val,NULL,0)==Val_Int32::cast(top)); break;
  case _uint32:  
    eq = (strtoul(t->val,NULL,0)==Val_UInt32::cast(top)); break;
  case _int64: 
    eq = (strtoll(t->val,NULL,0)==Val_Int64::cast(top)); break;
  case _uint64:  
    eq = (strtoull(t->val,NULL,0)==Val_UInt64::cast(top)); break;
  case _double:
    eq = (strtod(t->val,NULL)==Val_Double::cast(top)); break;
  case _string:
    eq = (Val_Str::cast(top) == t->val ); break;
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

int main(int argc, char **argv)
{
  std::cout << "PEL\n";
  uint i;

  //
  // Test the compiler
  //
  for(i = 0; i < num_ctests; i++) {
    const CompilerTest *t = &ctests[i];
    std::cout << "Compiling: " << t->src << "\n";
    Pel_Program *prog = Pel_Lexer::compile( t->src);
    if (prog->ops.size() != (uint) t->num_opcodes) {
      std::cerr << "** Bad # opcodes for '" << t->src << "'; " << prog->ops.size() << " instead of expected " << t->num_opcodes << "\n";
    }
    if (prog->const_pool.size() != (uint) t->num_consts) {
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
    vm_test(vm, tpl, i);
    std::cout.flush();
    std::cerr.flush();
  }
  return 0;
}

/*
 * End of file 
 */
