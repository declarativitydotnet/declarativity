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
#include "loop.h"

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
#define _tuple Value::TUPLE

#define TST(_type,_err,_val,_src) {__LINE__,Value::_type,Pel_VM::PE_##_err,_val,_src}

  

struct ValTest {
  int	line;		// Line number of test
  Value::TypeCode t;	// Type of expected result
  Pel_VM::Error err;	// Error (0 = success)
  char *val;		// String representation of value
  char *src;		// Source code
};

static const ValTest vtests[] = {
  // Literal values
  TST(UINT64, SUCCESS, "0xffffffffffffffff","0xffffffffffffffffU" ),
  // drop
  TST(NULLV, STACK_UNDERFLOW, "",	"drop" ),
  TST(INT64, SUCCESS, "1",	"1 2 drop"),
  TST(DOUBLE, SUCCESS, "1.0",  "1.0 \"Hello\" drop"),
  // swap
  TST(INT64, STACK_UNDERFLOW, "",	"1 swap"),
  TST(INT64, SUCCESS, "1",	"1 2 swap"),
  TST(INT64, SUCCESS, "2",	"1 2 swap swap"),
  // dup
  TST(INT64, STACK_UNDERFLOW, "",	"1 dup drop drop drop"),
  TST(NULLV, SUCCESS, "",	"1 dup drop drop"),
  TST(INT64, SUCCESS, "2",	"2 1 dup drop drop"),
  // ifelse
  TST(INT64, STACK_UNDERFLOW, "",	"ifelse"),
  TST(INT64, STACK_UNDERFLOW, "",	"1 ifelse"),
  TST(INT64, STACK_UNDERFLOW, "",	"1 2 ifelse"),
  TST(INT64, SUCCESS, "1",    "3.0 1 2 ifelse"),
  TST(INT64, SUCCESS, "2",    "0.0 1 2 ifelse"),
  TST(INT64, SUCCESS, "1",    "\"3.0\" 1 2 ifelse"),
  TST(INT64, SUCCESS, "2",	"1 2 3 ifelse"),
  TST(INT64, SUCCESS, "3",	"0 2 3 ifelse"),
  // ifpop
  TST(INT64, STACK_UNDERFLOW, "",	"ifpop"),
  TST(INT64, STACK_UNDERFLOW, "",	"1 ifpop"),
  TST(INT64, SUCCESS, "6",	"6 1 2 ifpop"),
  // ifpoptuple
  TST(INT64, STACK_UNDERFLOW, "",	"ifpoptuple"),
  // pop XXX more tests here on the resulting tuple!!
  TST(NULLV, STACK_UNDERFLOW, "",	"pop" ),
  TST(INT64, SUCCESS, "1",	"1 2 pop"),
  TST(NULLV, SUCCESS, "",     "1.0 \"Hello\" pop pop"),
  TST(NULLV, SUCCESS, "",     "1.0 pop \"Hello\" pop"),
  // not
  TST(INT32, STACK_UNDERFLOW, "",	"not" ),
  TST(INT32, SUCCESS, "0",     "1 not"),
  TST(INT32, SUCCESS, "1",     "0 not"),
  TST(INT32, SUCCESS, "0",     "300 not"),
  TST(INT32, SUCCESS, "0",     "300.0 not"),
  TST(INT32, SUCCESS, "1",     "\"Hello\" not"),
  TST(INT32, SUCCESS, "1",     "null not"),
  // and
  TST(INT32, STACK_UNDERFLOW, "",	"and" ),
  TST(INT32, STACK_UNDERFLOW, "",	"1 and" ),
  TST(INT32, SUCCESS, "0",     "0 0 and"),
  TST(INT32, SUCCESS, "0",     "1 0 and"),
  TST(INT32, SUCCESS, "0",     "0 1 and"),
  TST(INT32, SUCCESS, "1",     "1 1 and"),
  TST(INT32, SUCCESS, "1",     "1 300 and"),
  TST(INT32, SUCCESS, "1",     "300.0 1 and"),
  TST(INT32, SUCCESS, "0",     "\"Hello\" 1 and"),
  TST(INT32, SUCCESS, "0",     "null 1 and"),
  TST(INT32, SUCCESS, "0",     "0 300.0 and"),
  TST(INT32, SUCCESS, "0",     "1 \"Hello\" and"),
  TST(INT32, SUCCESS, "0",     "1 null and"),
  // or
  TST(INT32, STACK_UNDERFLOW, "",	"or" ),
  TST(INT32, STACK_UNDERFLOW, "",	"1 or" ),
  TST(INT32, SUCCESS, "0",     "0 0 or"),
  TST(INT32, SUCCESS, "1",     "1 0 or"),
  TST(INT32, SUCCESS, "1",     "0 1 or"),
  TST(INT32, SUCCESS, "1",     "1 1 or"),
  TST(INT32, SUCCESS, "1",     "1 300 or"),
  TST(INT32, SUCCESS, "1",     "300.0 1 or"),
  TST(INT32, SUCCESS, "1",     "\"Hello\" 1 or"),
  TST(INT32, SUCCESS, "1",     "null 1 or"),
  TST(INT32, SUCCESS, "1",     "0 300.0 or"),
  TST(INT32, SUCCESS, "1",     "1 \"Hello\" or"),
  TST(INT32, SUCCESS, "1",     "1 null or"),
  // >> (logical shift right)
  TST(INT32, STACK_UNDERFLOW, "",	">>" ),
  TST(INT32, STACK_UNDERFLOW, "",	"1 >>" ),
  TST(INT64, SUCCESS, "0",	"1 1 >>" ),
  TST(INT64, SUCCESS, "1",	"2 1 >>" ),
  TST(INT64, SUCCESS, "4",	"16 2 >>" ),
  TST(UINT64, SUCCESS, "0x7fffffffffffffff","0xffffffffffffffffU 1 >>" ),
  TST(UINT64, SUCCESS, "0x3fffffffffffffffU","-1 2U >>" ),

  // >> (arithmetic shift right)
  TST(INT64, STACK_UNDERFLOW, "",	">>" ),
  TST(INT64, STACK_UNDERFLOW, "",	"1 >>" ),
  TST(INT64, SUCCESS, "0",	"1 1 >>" ),
  TST(INT64, SUCCESS, "1",	"2 1 >>" ),
  TST(INT64, SUCCESS, "4",	"16 2 >>" ),
  TST(INT64, SUCCESS, "-1",	"0xffffffffffffffffU ->i64 1 >>" ),
  TST(INT64, SUCCESS, "-1",	"-1 2 >>" ),
  // << (logical shift left)
  TST(INT64, STACK_UNDERFLOW, "",	"<<" ),
  TST(INT64, STACK_UNDERFLOW, "",	"1 <<" ),
  TST(INT64, SUCCESS, "2",	"1 1 <<" ),
  TST(INT64, SUCCESS, "4",	"2 1 <<" ),
  TST(INT64, SUCCESS, "64",	"16 2 <<" ),
  TST(ID,    SUCCESS, "4",	"1 ->id 2 <<" ),
  TST(UINT64, SUCCESS, "0xfffffffffffffffeU",	"0xffffffffffffffffU 1 <<" ),
  TST(UINT64, SUCCESS, "0xfffffffffffffffcU",	"-1 ->u64 2 <<" ),
  // & (bitwise AND)
  TST(UINT64, STACK_UNDERFLOW, "",	"&" ),
  TST(UINT64, STACK_UNDERFLOW, "",	"1U &" ),
  TST(UINT64, SUCCESS, "1",	"1U 1 &" ),
  TST(UINT64, SUCCESS, "0",	"0U 3 &" ),
  TST(UINT64, SUCCESS, "0",	"3U 0 &" ),
  TST(UINT64, SUCCESS, "3",	"3U 3 &" ),
  TST(UINT64, SUCCESS, "3",	"7U 3 &" ), 
  TST(UINT64, SUCCESS, "0","0xf0f0fU 0xf0f0f0 &" ),
  TST(UINT64, SUCCESS, "0",	"0xffffffff00000000U 0xffffffff &" ),
  TST(UINT64, SUCCESS, "0x1010101010101010",	"0xffffffffffffffffU 0x1010101010101010 &" ),
  // | (bitwise inclusive OR)
  TST(UINT64, STACK_UNDERFLOW, "",	"|" ),
  TST(UINT64, STACK_UNDERFLOW, "",	"1U |" ),
  TST(UINT64, SUCCESS, "1",	"1U 1 |" ),
  TST(UINT64, SUCCESS, "3",	"0U 3 |" ),
  TST(UINT64, SUCCESS, "3",	"3U 0 |" ),
  TST(UINT64, SUCCESS, "3",	"3U 3 |" ),
  TST(UINT64, SUCCESS, "7",	"7U 3 |" ), 
  TST(UINT64, SUCCESS, "0xffffff","0xf0f0fU 0xf0f0f0 |" ),
  TST(UINT64, SUCCESS, "0xffffffffffffffff",	"0xffffffff00000000U 0xffffffff |" ),
  TST(UINT64, SUCCESS, "0xffffffffffffffff",	"0xffffffffffffffffU 0x1010101010101010 |" ),
  // ^ (bitwise exclusive OR)
  TST(UINT64, STACK_UNDERFLOW, "",	"^" ),
  TST(UINT64, STACK_UNDERFLOW, "",	"1U ^" ),
  TST(UINT64, SUCCESS, "0",	"1U 1 ^" ),
  TST(UINT64, SUCCESS, "3",	"0U 3 ^" ),
  TST(UINT64, SUCCESS, "3",	"3U 0 ^" ),
  TST(UINT64, SUCCESS, "0",	"3U 3 ^" ),
  TST(UINT64, SUCCESS, "4",	"7U 3 ^" ), 
  TST(UINT64, SUCCESS, "0xffffff","0xf0f0fU 0xf0f0f0 ^" ),
  TST(UINT64, SUCCESS, "0xffffffffffffffff",	"0xffffffff00000000U 0xffffffff ^" ),
  TST(UINT64, SUCCESS, "0xefefefefefefefef",	"0xffffffffffffffffU 0x1010101010101010 ^" ),
  // ~ (bitwise NOT)
  TST(UINT64, STACK_UNDERFLOW, "",	"~" ),
  TST(UINT64, SUCCESS, "0xfffffffffffffffe",	"1U ~" ),
  TST(UINT64, SUCCESS, "0xffffffffffffffff",	"\"Hello\" ->u64 ~" ),
  TST(UINT64, SUCCESS, "0xfffffffffffffffe",	"1 1U ~" ),
  TST(UINT64, SUCCESS, "0xffff0f0f0f0f0f0f","0xf0f0f0f0f0f0U ~" ),
  TST(UINT64, SUCCESS, "0", "0xffffffffffffffffU ~" ),
  TST(UINT64, SUCCESS, "0xefefefefefefefef",	"0x1010101010101010U ~" ),
  // % (integer modulus)
  TST(INT64, STACK_UNDERFLOW, "",	"%" ),
  TST(INT64, STACK_UNDERFLOW, "",	"1 %" ),
  TST(INT64, SUCCESS, "0",	"1 1 %" ),
  TST(INT64, SUCCESS, "0",	"\"Hello\" ->i64 1 %" ),
  TST(INT64, SUCCESS, "0",	"1 1 %" ),
  TST(INT64, DIVIDE_BY_ZERO, "",	"1 0 %" ),
  TST(INT64, SUCCESS, "0",	"0 2 %"),
  TST(INT64, SUCCESS, "0",	"0 1 %"),
  TST(INT64, SUCCESS, "0",	"0 500 %"),
  TST(INT64, SUCCESS, "499",	"499 500 %"),
  TST(INT64, SUCCESS, "1",	"13 2 %"),
  TST(INT64, SUCCESS, "2",	"602 5 %"),
  // <s (string less-than)
  TST(INT32, STACK_UNDERFLOW, "",	"<s" ),
  TST(INT32, STACK_UNDERFLOW, "",	"1 <s" ),
  TST(INT32, SUCCESS, "1",	"1.0 \"Hello\" <s" ),
  TST(INT32, SUCCESS, "1",	"1 \"Hello\" <s" ),
  TST(INT32, SUCCESS, "0",	"\"Hello\" 1.0 <s" ),
  TST(INT32, SUCCESS, "0",	"\"Hello\" 1 <s" ),
  TST(INT32, SUCCESS, "1",	"\"A\" \"B\" <s" ),
  TST(INT32, SUCCESS, "0",	"\"B\" \"A\" <s" ),
  TST(INT32, SUCCESS, "0",	"\"A\" \"A\" <s" ),
  TST(INT32, SUCCESS, "1",	"\"A\" \"a\" <s" ),
  TST(INT32, SUCCESS, "0",	"\"a\" \"A\" <s" ),
  TST(INT32, SUCCESS, "0",	"\"AAB\" \"AAA\" <s" ),
  TST(INT32, SUCCESS, "1",	"\"AAA\" \"AAB\" <s" ),
  // < (string less-than)
  TST(INT32, STACK_UNDERFLOW, "",	"<" ),
  TST(INT32, STACK_UNDERFLOW, "",	"1 <" ),
  TST(INT32, SUCCESS, "1",	"1.0 \"Hello\" <" ),
  TST(INT32, SUCCESS, "1",	"1 \"Hello\" <" ),
  TST(INT32, SUCCESS, "0",	"\"Hello\" 1.0 <" ),
  TST(INT32, SUCCESS, "0",	"\"Hello\" 1 <" ),
  TST(INT32, SUCCESS, "1",	"\"A\" \"B\" <" ),
  TST(INT32, SUCCESS, "0",	"\"B\" \"A\" <" ),
  TST(INT32, SUCCESS, "0",	"\"A\" \"A\" <" ),
  TST(INT32, SUCCESS, "1",	"\"A\" \"a\" <" ),
  TST(INT32, SUCCESS, "0",	"\"a\" \"A\" <" ),
  TST(INT32, SUCCESS, "0",	"\"AAB\" \"AAA\" <" ),
  TST(INT32, SUCCESS, "1",	"\"AAA\" \"AAB\" <" ),
  // <=s (string less-than-or-equal)
  TST(INT32, STACK_UNDERFLOW, "",	"<=s" ),
  TST(INT32, STACK_UNDERFLOW, "",	"1 <=s" ),
  TST(INT32, SUCCESS, "1",	"1.0 \"Hello\" <=s" ),
  TST(INT32, SUCCESS, "1",	"1 \"Hello\" <=s" ),
  TST(INT32, SUCCESS, "0",	"\"Hello\" 1.0 <=s" ),
  TST(INT32, SUCCESS, "0",	"\"Hello\" 1 <=s" ),
  TST(INT32, SUCCESS, "1",	"\"A\" \"B\" <=s" ),
  TST(INT32, SUCCESS, "0",	"\"B\" \"A\" <=s" ),
  TST(INT32, SUCCESS, "1",	"\"A\" \"A\" <=s" ),
  TST(INT32, SUCCESS, "1",	"\"A\" \"a\" <=s" ),
  TST(INT32, SUCCESS, "0",	"\"a\" \"A\" <=s" ),
  TST(INT32, SUCCESS, "0",	"\"AAB\" \"AAA\" <=s" ),
  TST(INT32, SUCCESS, "1",	"\"AAA\" \"AAB\" <=s" ),
  // <= (string less-than-or-equal)
  TST(INT32, STACK_UNDERFLOW, "",	"<=" ),
  TST(INT32, STACK_UNDERFLOW, "",	"1 <=" ),
  TST(INT32, SUCCESS, "1",	"1.0 \"Hello\" <=" ),
  TST(INT32, SUCCESS, "1",	"1 \"Hello\" <=" ),
  TST(INT32, SUCCESS, "0",	"\"Hello\" 1.0 <=" ),
  TST(INT32, SUCCESS, "0",	"\"Hello\" 1 <=" ),
  TST(INT32, SUCCESS, "1",	"\"A\" \"B\" <=" ),
  TST(INT32, SUCCESS, "0",	"\"B\" \"A\" <=" ),
  TST(INT32, SUCCESS, "1",	"\"A\" \"A\" <=" ),
  TST(INT32, SUCCESS, "1",	"\"A\" \"a\" <=" ),
  TST(INT32, SUCCESS, "0",	"\"a\" \"A\" <=" ),
  TST(INT32, SUCCESS, "0",	"\"AAB\" \"AAA\" <=" ),
  TST(INT32, SUCCESS, "1",	"\"AAA\" \"AAB\" <=" ),
  // >s (string greater-than)
  TST(INT32, STACK_UNDERFLOW, "",	">s" ),
  TST(INT32, STACK_UNDERFLOW, "",	"1 >s" ),
  TST(INT32, SUCCESS, "0",	"1.0 \"Hello\" >s" ),
  TST(INT32, SUCCESS, "0",	"1 \"Hello\" >s" ),
  TST(INT32, SUCCESS, "1",	"\"Hello\" 1.0 >s" ),
  TST(INT32, SUCCESS, "1",	"\"Hello\" 1 >s" ),
  TST(INT32, SUCCESS, "0",	"\"A\" \"B\" >s" ),
  TST(INT32, SUCCESS, "1",	"\"B\" \"A\" >s" ),
  TST(INT32, SUCCESS, "0",	"\"A\" \"A\" >s" ),
  TST(INT32, SUCCESS, "0",	"\"A\" \"a\" >s" ),
  TST(INT32, SUCCESS, "1",	"\"a\" \"A\" >s" ),
  TST(INT32, SUCCESS, "1",	"\"AAB\" \"AAA\" >s" ),
  TST(INT32, SUCCESS, "0",	"\"AAA\" \"AAB\" >s" ),
  // > (string greater-than)
  TST(INT32, STACK_UNDERFLOW, "",	">" ),
  TST(INT32, STACK_UNDERFLOW, "",	"1 >" ),
  TST(INT32, SUCCESS, "0",	"1.0 \"Hello\" >" ),
  TST(INT32, SUCCESS, "0",	"1 \"Hello\" >" ),
  TST(INT32, SUCCESS, "1",	"\"Hello\" 1.0 >" ),
  TST(INT32, SUCCESS, "1",	"\"Hello\" 1 >" ),
  TST(INT32, SUCCESS, "0",	"\"A\" \"B\" >" ),
  TST(INT32, SUCCESS, "1",	"\"B\" \"A\" >" ),
  TST(INT32, SUCCESS, "0",	"\"A\" \"A\" >" ),
  TST(INT32, SUCCESS, "0",	"\"A\" \"a\" >" ),
  TST(INT32, SUCCESS, "1",	"\"a\" \"A\" >" ),
  TST(INT32, SUCCESS, "1",	"\"AAB\" \"AAA\" >" ),
  TST(INT32, SUCCESS, "0",	"\"AAA\" \"AAB\" >" ),
  // >=s (string greater-than-or-equal)
  TST(INT32, STACK_UNDERFLOW, "",	">=s" ),
  TST(INT32, STACK_UNDERFLOW, "",	"1 >=s" ),
  TST(INT32, SUCCESS, "0",	"1.0 \"Hello\" >=s" ),
  TST(INT32, SUCCESS, "0",	"1 \"Hello\" >=s" ),
  TST(INT32, SUCCESS, "1",	"\"Hello\" 1.0 >=s" ),
  TST(INT32, SUCCESS, "1",	"\"Hello\" 1 >=s" ),
  TST(INT32, SUCCESS, "0",	"\"A\" \"B\" >=s" ),
  TST(INT32, SUCCESS, "1",	"\"B\" \"A\" >=s" ),
  TST(INT32, SUCCESS, "1",	"\"A\" \"A\" >=s" ),
  TST(INT32, SUCCESS, "0",	"\"A\" \"a\" >=s" ),
  TST(INT32, SUCCESS, "1",	"\"a\" \"A\" >=s" ),
  TST(INT32, SUCCESS, "1",	"\"AAB\" \"AAA\" >=s" ),
  TST(INT32, SUCCESS, "0",	"\"AAA\" \"AAB\" >=s" ),
  // >= (string greater-than-or-equal)
  TST(INT32, STACK_UNDERFLOW, "",	">=" ),
  TST(INT32, STACK_UNDERFLOW, "",	"1 >=" ),
  TST(INT32, SUCCESS, "0",	"1.0 \"Hello\" >=" ),
  TST(INT32, SUCCESS, "0",	"1 \"Hello\" >=" ),
  TST(INT32, SUCCESS, "1",	"\"Hello\" 1.0 >=" ),
  TST(INT32, SUCCESS, "1",	"\"Hello\" 1 >=" ),
  TST(INT32, SUCCESS, "0",	"\"A\" \"B\" >=" ),
  TST(INT32, SUCCESS, "1",	"\"B\" \"A\" >=" ),
  TST(INT32, SUCCESS, "1",	"\"A\" \"A\" >=" ),
  TST(INT32, SUCCESS, "0",	"\"A\" \"a\" >=" ),
  TST(INT32, SUCCESS, "1",	"\"a\" \"A\" >=" ),
  TST(INT32, SUCCESS, "1",	"\"AAB\" \"AAA\" >=" ),
  TST(INT32, SUCCESS, "0",	"\"AAA\" \"AAB\" >=" ),
  // ==s (string equality)
  TST(INT32, STACK_UNDERFLOW, "",	"==s" ),
  TST(INT32, STACK_UNDERFLOW, "",	"1 ==s" ),
  TST(INT32, SUCCESS, "0",	"1.0 \"Hello\" ==s" ),
  TST(INT32, SUCCESS, "0",	"1 \"Hello\" ==s" ),
  TST(INT32, SUCCESS, "0",	"\"Hello\" 1.0 ==s" ),
  TST(INT32, SUCCESS, "0",	"\"Hello\" 1 ==s" ),
  TST(INT32, SUCCESS, "0",	"\"A\" \"B\" ==s" ),
  TST(INT32, SUCCESS, "0",	"\"B\" \"A\" ==s" ),
  TST(INT32, SUCCESS, "1",	"\"A\" \"A\" ==s" ),
  TST(INT32, SUCCESS, "0",	"\"A\" \"a\" ==s" ),
  TST(INT32, SUCCESS, "0",	"\"a\" \"A\" ==s" ),
  TST(INT32, SUCCESS, "1",	"\"AAA\" \"AAA\" ==s" ),
  TST(INT32, SUCCESS, "0",	"\"AAB\" \"AAA\" ==s" ),
  TST(INT32, SUCCESS, "0",	"\"AAA\" \"AAB\" ==s" ),
  // == (string equality)
  TST(INT32, STACK_UNDERFLOW, "",	"==" ),
  TST(INT32, STACK_UNDERFLOW, "",	"1 ==" ),
  TST(INT32, SUCCESS, "0",	"1.0 \"Hello\" ==" ),
  TST(INT32, SUCCESS, "0",	"1 \"Hello\" ==" ),
  TST(INT32, SUCCESS, "0",	"\"Hello\" 1.0 ==" ),
  TST(INT32, SUCCESS, "0",	"\"Hello\" 1 ==" ),
  TST(INT32, SUCCESS, "0",	"\"A\" \"B\" ==" ),
  TST(INT32, SUCCESS, "0",	"\"B\" \"A\" ==" ),
  TST(INT32, SUCCESS, "1",	"\"A\" \"A\" ==" ),
  TST(INT32, SUCCESS, "0",	"\"A\" \"a\" ==" ),
  TST(INT32, SUCCESS, "0",	"\"a\" \"A\" ==" ),
  TST(INT32, SUCCESS, "1",	"\"AAA\" \"AAA\" ==" ),
  TST(INT32, SUCCESS, "0",	"\"AAB\" \"AAA\" ==" ),
  TST(INT32, SUCCESS, "0",	"\"AAA\" \"AAB\" ==" ),
  // strcat (string concatenation)
  TST(STR, STACK_UNDERFLOW, "",	"strcat" ),
  TST(STR, STACK_UNDERFLOW, "",	"1 strcat" ),
  TST(STR, SUCCESS, "0x1p+0Hello",	"1.0 \"Hello\" strcat" ),
  TST(STR, SUCCESS, "1Hello",	"1 \"Hello\" strcat" ),
  TST(STR, SUCCESS, "Hello0x1p+0",	"\"Hello\" 1.0 strcat" ),
  TST(STR, SUCCESS, "Hello1",	"\"Hello\" 1 strcat" ),
  TST(STR, SUCCESS, "AB",	"\"A\" \"B\" strcat" ),
  TST(STR, SUCCESS, "B",	"\"B\" \"\" strcat" ),
  TST(STR, SUCCESS, "B",	"\"\" \"B\" strcat" ),
  TST(STR, SUCCESS, "",	"\"\" \"\" strcat" ),
  TST(STR, SUCCESS, "Aa",	"\"A\" \"a\" strcat" ),
  TST(STR, SUCCESS, "aA",	"\"a\" \"A\" strcat" ),
  TST(STR, SUCCESS, "AAAAAA", "\"AAA\" \"AAA\" strcat" ),
  // strlen (string length)
  TST(UINT32, STACK_UNDERFLOW, "",	"strlen" ),
  TST(UINT32, SUCCESS, "6",	"1.0 strlen" ),
  TST(UINT32, SUCCESS, "1",	"1 strlen" ),
  TST(UINT32, SUCCESS, "1",	"\"A\" strlen" ),
  TST(UINT32, SUCCESS, "1",	"\"1\" strlen" ),
  TST(UINT32, SUCCESS, "0",	"\"\" strlen" ),
  TST(UINT32, SUCCESS, "1",	"\"\\0\" strlen" ),
  TST(UINT32, SUCCESS, "5",	"\"Hello\" strlen" ),
  TST(UINT32, SUCCESS, "20","\"String\\nwith\\rcontrols\" strlen" ),
  // upper (string to upper case)
  TST(STR, STACK_UNDERFLOW, "",	"upper" ),
  TST(STR, SUCCESS, "0X1P+0",	"1.0 upper" ),
  TST(STR, SUCCESS, "1",	"1 upper" ),
  TST(STR, SUCCESS, "A",	"\"A\" upper" ),
  TST(STR, SUCCESS, "A",	"\"a\" upper" ),
  TST(STR, SUCCESS, "",	"\"\" upper" ),
  TST(STR, SUCCESS, "HELLO", "\"Hello\" upper" ),
  TST(STR, SUCCESS, "STRING\nWITH\rCONTROLS","\"String\\nwith\\rcontrols\" upper" ),
  // lower (string to lower case)
  TST(STR, STACK_UNDERFLOW, "",	"lower" ),
  TST(STR, SUCCESS, "0x1p+0",	"1.0 lower" ),
  TST(STR, SUCCESS, "1",	"1 lower" ),
  TST(STR, SUCCESS, "a",	"\"A\" lower" ),
  TST(STR, SUCCESS, "a",	"\"a\" lower" ),
  TST(STR, SUCCESS, "",	"\"\" lower" ),
  TST(STR, SUCCESS, "hello", "\"Hello\" lower" ),
  TST(STR, SUCCESS, "string\nwith\rcontrols","\"String\\nwith\\rcontrols\" lower" ),
  // substr (extract substring)
  TST(STR, STACK_UNDERFLOW, "",	"substr" ),
  TST(STR, STACK_UNDERFLOW, "",	"\"A\" substr" ),
  TST(STR, STACK_UNDERFLOW, "",	"\"A\" 1 substr" ),
  TST(STR, SUCCESS, "",	"1 1 1 substr" ),
  TST(STR, SUCCESS, "x",	"1.0 1 1 substr" ),
  TST(STR, SUCCESS, "b",	"\"abcdefg\" 1.0 1 substr" ),
  TST(STR, SUCCESS, "b",	"\"abcdefg\" 1 1.0 substr" ),
  TST(STR, SUCCESS, "a",	"\"abcdefg\" \"a\" 1 substr" ),
  TST(STR, SUCCESS, "", 	"\"abcdefg\" 1 \"a\" substr" ),
  TST(STR, SUCCESS, "", 	"\"abcdefg\" 1 null substr" ),
  TST(STR, SUCCESS, "a",	"\"abcdefg\" null 1 substr" ),
  TST(STR, SUCCESS, "U",	"null 1 1 substr" ),
  TST(STR, SUCCESS, "abc",	"\"abcdefg\" 0 3 substr" ),
  TST(STR, SUCCESS, "bcde","\"abcdefg\" 1 4 substr" ),
  TST(STR, SUCCESS, "abcdefg","\"abcdefg\" 0 7 substr" ),
  TST(STR, SUCCESS, "fg",	"\"abcdefg\" 5 8 substr" ),
  TST(STR, SUCCESS, "",	"\"abcdefg\" 0 0 substr" ),
  TST(STR, SUCCESS, "",	"\"abcdefg\" 4 0 substr" ),
  TST(STR, SUCCESS, "",	"\"abcdefg\" 10 0 substr" ),
  TST(STR, SUCCESS, "",	"\"abcdefg\" 10 3 substr" ),
  // match (Perl regular expression matching: not much testing here
  // yet :-( )
  TST(INT32, STACK_UNDERFLOW, "",	"match" ),
  TST(INT32, STACK_UNDERFLOW, "",	"\"A\" match" ),
  TST(INT32, SUCCESS, "0",	"\"A\" 1 match" ),
  TST(INT32, SUCCESS, "1",	"1 1 match" ),
  TST(INT32, SUCCESS, "0",	"1.0 1 match" ),
  TST(INT32, SUCCESS, "0",	"\"abcdefg\" 1.0 match" ),
  TST(INT32, SUCCESS, "0",	"\"abcdefg\" 1 match" ),
  TST(INT32, SUCCESS, "0",	"\"abcdefg\" null match" ),
  TST(INT32, SUCCESS, "0",	"null \"A\" match" ),
  TST(INT32, SUCCESS, "1",	"\"abcd\" \"abcd\" match" ),
  TST(INT32, SUCCESS, "1",	"\"abcd\" \"ab.*\" match" ),
  TST(INT32, SUCCESS, "1",	"\"abcd\" \".*cd\" match" ),
  TST(INT32, SUCCESS, "0",	"\"abcd\" \"ab\" match" ),
  TST(INT32, SUCCESS, "0",	"\"abcd\" \"cd\" match" ),
  // hash (hashing a string to 32 bits)
  TST(UINT32, STACK_UNDERFLOW, "",	"hash" ),
  TST(UINT32, SUCCESS, "0x568C3A1F","1 hash" ),
  TST(UINT32, SUCCESS, "0x384BCCBB","1.0 hash" ),
  TST(UINT32, SUCCESS, "0xF705F9BF","null hash" ),
  TST(UINT32, SUCCESS, "0x0BA024EB","\"A\" hash" ),
  TST(UINT32, SUCCESS, "0x7C79376A","\"\" hash" ),
  TST(UINT32, SUCCESS, "0x4BB7E6E7","\"Hello, world!\" hash" ),
  // negi (integer unary negation)
  TST(INT64, STACK_UNDERFLOW, "",	"negi" ),
  TST(INT64, SUCCESS, "0",	"\"A\" negi" ),
  TST(INT64, SUCCESS, "-1",	"1.0 negi" ),
  TST(INT64, SUCCESS, "0",	"null negi" ),
  TST(INT64, SUCCESS, "-1",	"1 negi" ),
  TST(INT64, SUCCESS, "1",	"-1 negi" ),
  TST(INT64, SUCCESS, "-2000","2000 negi" ),
  TST(INT64, SUCCESS, "2000", "-2000 negi" ),
  TST(INT64, SUCCESS, "0",	"0 negi" ),
  // neg (unary negation)
  TST(INT64, STACK_UNDERFLOW, "", "neg" ),
  TST(INT64, OPER_UNSUP, "0", "\"A\" neg" ),
  TST(DOUBLE, SUCCESS, "-1.0", "1.0 neg" ),
  TST(INT64, OPER_UNSUP, "0",	"null neg" ),
  TST(INT64, SUCCESS, "-1",	"1 neg" ),
  TST(INT64, SUCCESS, "1",	"-1 neg" ),
  TST(INT64, SUCCESS, "-2000","2000 neg" ),
  TST(INT64, SUCCESS, "2000", "-2000 neg" ),
  TST(INT64, SUCCESS, "0",	"0 neg" ),
  // +i (integer addition)
  TST(INT64, STACK_UNDERFLOW, "",	"+i" ),
  TST(INT64, STACK_UNDERFLOW, "",	"1 +i" ),
  TST(INT64, SUCCESS, "1",	"1 \"Hello\" +i" ),
  TST(INT64, SUCCESS, "1",	"\"Hello\"  1 +i" ),
  TST(INT64, SUCCESS, "2",	"1.0 1 +i" ),
  TST(INT64, SUCCESS, "2",	"1 1.0 +i" ),
  TST(INT64, SUCCESS, "3",	"1 2 +i" ),
  TST(INT64, SUCCESS, "0",	"1 -1 +i" ),
  TST(INT64, SUCCESS, "0x100000000","0xffffffff 1 +i" ),
  TST(INT64, SUCCESS, "-2",	"0xffffffffffffffffU -1 +i" ),
  // + (integer addition)
  TST(INT64, STACK_UNDERFLOW, "", "+" ),
  TST(INT64, STACK_UNDERFLOW, "", "1 +" ),
  TST(STR, SUCCESS, "1Hello", "1 \"Hello\" +" ),
  TST(STR, SUCCESS, "Hello1", "\"Hello\"  1 +" ),
  TST(DOUBLE, SUCCESS, "2", "1.0 1 +" ),
  TST(DOUBLE, SUCCESS, "2", "1 1.0 +" ),
  TST(INT64, SUCCESS, "3", "1 2 +" ),
  TST(INT64, SUCCESS, "0", "1 -1 +" ),
  TST(INT64, SUCCESS, "0x100000000","0xffffffff 1 +" ),
  TST(INT64, SUCCESS, "-2", "0xffffffffffffffffU ->i64 -1 +" ),
  // -i (integer subtraction)
  TST(INT64, STACK_UNDERFLOW, "",	"-i" ),
  TST(INT64, STACK_UNDERFLOW, "",	"1 -i" ),
  TST(INT64, SUCCESS, "1",	"1 \"Hello\" -i" ),
  TST(INT64, SUCCESS, "-1",	"\"Hello\"  1 -i" ),
  TST(INT64, SUCCESS, "0",	"1.0 1 -i" ),
  TST(INT64, SUCCESS, "0",	"1 1.0 -i" ),
  TST(INT64, SUCCESS, "-1",	"1 2 -i" ),
  TST(INT64, SUCCESS, "2",	"1 -1 -i" ),
  TST(INT64, SUCCESS, "0",	"0xffffffffffffffffU -1 -i" ),
  // - (integer subtraction)
  TST(INT64, STACK_UNDERFLOW, "",	"-" ),
  TST(INT64, STACK_UNDERFLOW, "",	"1 -" ),
  TST(INT64, OPER_UNSUP, "1",	"1 \"Hello\" -" ),
  TST(INT64, OPER_UNSUP, "-1",	"\"Hello\"  1 -" ),
  TST(DOUBLE, SUCCESS, "0",	"1.0 1 -" ),
  TST(DOUBLE, SUCCESS, "0",	"1 1.0 -" ),
  TST(INT64, SUCCESS, "-1",	"1 2 -" ),
  TST(INT64, SUCCESS, "2",	"1 -1 -" ),
  TST(INT64, SUCCESS, "-2",	"-1 1 -" ),
  TST(INT64, SUCCESS, "0",	"0xffffffffffffffffU ->i64 -1 -" ),
  // *i (integer multiplication)
  TST(INT64, STACK_UNDERFLOW, "",	"*i" ),
  TST(INT64, STACK_UNDERFLOW, "",	"1 *i" ),
  TST(INT64, SUCCESS, "0",	"1 \"Hello\" *i" ),
  TST(INT64, SUCCESS, "0",	"\"Hello\"  1 *i" ),
  TST(INT64, SUCCESS, "1",	"1.0 1 *i" ),
  TST(INT64, SUCCESS, "1",	"1 1.0 *i" ),
  TST(INT64, SUCCESS, "2",	"1 2 *i" ),
  TST(INT64, SUCCESS, "1",	"-1 -1 *i" ),
  TST(INT64, SUCCESS, "-2",	"0xffffffffffffffffU 2 *i" ),
  // * (integer multiplication)
  TST(INT64, STACK_UNDERFLOW, "",	"*" ),
  TST(INT64, STACK_UNDERFLOW, "",	"1 *" ),
  TST(INT64, OPER_UNSUP, "0",	"1 \"Hello\" *" ),
  TST(INT64, OPER_UNSUP, "0",	"\"Hello\"  1 *" ),
  TST(DOUBLE, SUCCESS, "1",	"1.0 1 *" ),
  TST(DOUBLE, SUCCESS, "1",	"1 1.0 *" ),
  TST(INT64, SUCCESS, "2",	"1 2 *" ),
  TST(INT64, SUCCESS, "1",	"-1 -1 *" ),
  TST(INT64, SUCCESS, "-2",	"0xffffffffffffffffU ->i64 2 *" ),
  // /i (integer division)
  TST(INT64, STACK_UNDERFLOW, "",	"/i" ),
  TST(INT64, STACK_UNDERFLOW, "",	"1 /i" ),
  TST(INT64, DIVIDE_BY_ZERO, "",	"1 \"Hello\" /i" ),
  TST(INT64, SUCCESS, "0",	"\"Hello\"  1 /i" ),
  TST(INT64, SUCCESS, "1",	"1.0 1 /i" ),
  TST(INT64, SUCCESS, "1",	"1 1.0 /i" ),
  TST(INT64, DIVIDE_BY_ZERO, "",	"2 0 /i" ),
  TST(INT64, SUCCESS, "4",	"8 2 /i" ),
  TST(INT64, SUCCESS, "5",	"11 2 /i" ),
  TST(INT64, SUCCESS, "0",	"0 30 /i" ),
  TST(INT64, SUCCESS, "0",	"0xffffffffffffffffU 3 /i" ),
  // / (integer division)
  TST(INT64, STACK_UNDERFLOW, "",	"/" ),
  TST(INT64, STACK_UNDERFLOW, "",	"1 /" ),
  TST(INT64, OPER_UNSUP, "",	"1 \"Hello\" /" ),
  TST(INT64, OPER_UNSUP, "0",	"\"Hello\"  1 /" ),
  TST(DOUBLE, SUCCESS, "1",	"1.0 1 /" ),
  TST(DOUBLE, SUCCESS, "1",	"1 1.0 /" ),
  TST(INT64, DIVIDE_BY_ZERO, "",	"2 0 /" ),
  TST(INT64, SUCCESS, "4",	"8 2 /" ),
  TST(INT64, SUCCESS, "5",	"11 2 /" ),
  TST(INT64, SUCCESS, "0",	"0 30 /" ),
  TST(INT64, SUCCESS, "0",	"0xffffffffffffffffU ->i64 3 /" ),
  // ==i (integer equal)
  TST(INT32, STACK_UNDERFLOW, "",	"==i" ),
  TST(INT32, STACK_UNDERFLOW, "",	"1 ==i" ),
  TST(INT32, SUCCESS, "0",	"1 \"Hello\" ==i" ),
  TST(INT32, SUCCESS, "0",	"\"Hello\"  1 ==i" ),
  TST(INT32, SUCCESS, "1",	"1.0 1 ==i" ),
  TST(INT32, SUCCESS, "1",	"1 1.0 ==i" ),
  TST(INT32, SUCCESS, "0",	"1 2 ==i" ),
  TST(INT32, SUCCESS, "1",	"1 1 ==i" ),
  TST(INT32, SUCCESS, "1",	"0xffffffffffffffffU -1 ==i" ),
  // == (integer equal)
  TST(INT32, STACK_UNDERFLOW, "",	"==" ),
  TST(INT32, STACK_UNDERFLOW, "",	"1 ==" ),
  TST(INT32, SUCCESS, "0",	"1 \"Hello\" ==" ),
  TST(INT32, SUCCESS, "0",	"\"Hello\"  1 ==" ),
  TST(INT32, SUCCESS, "1",	"1.0 1 ==" ),
  TST(INT32, SUCCESS, "1",	"1 1.0 ==" ),
  TST(INT32, SUCCESS, "0",	"1 2 ==" ),
  TST(INT32, SUCCESS, "1",	"1 1 ==" ),
  TST(INT32, SUCCESS, "1",	"0xffffffffffffffffU -1 ==" ),
  // >i (integer greater-than)
  TST(INT32, STACK_UNDERFLOW, "",	">i" ),
  TST(INT32, STACK_UNDERFLOW, "",	"1 >i" ),
  TST(INT32, SUCCESS, "1",	"1 \"Hello\" >i" ),
  TST(INT32, SUCCESS, "0",	"\"Hello\"  1 >i" ),
  TST(INT32, SUCCESS, "0",	"1.0 1 >i" ),
  TST(INT32, SUCCESS, "0",	"1 1.0 >i" ),
  TST(INT32, SUCCESS, "0",	"1 2 >i" ),
  TST(INT32, SUCCESS, "1",	"2 1 >i" ),
  TST(INT32, SUCCESS, "0",	"1 1 >i" ),
  TST(INT32, SUCCESS, "1",	"-1 -2 >i" ),
  TST(INT32, SUCCESS, "0",	"-2 -1 >i" ),
  TST(INT32, SUCCESS, "0",	"0xffffffffffffffffU 2 >i" ),
  // > (integer greater-than)
  TST(INT32, STACK_UNDERFLOW, "",	">" ),
  TST(INT32, STACK_UNDERFLOW, "",	"1 >" ),
  TST(INT32, SUCCESS, "0",	"1 \"Hello\" >" ),
  TST(INT32, SUCCESS, "1",	"\"Hello\"  1 >" ),
  TST(INT32, SUCCESS, "0",	"1.0 1 >" ),
  TST(INT32, SUCCESS, "0",	"1 1.0 >" ),
  TST(INT32, SUCCESS, "0",	"1 2 >" ),
  TST(INT32, SUCCESS, "1",	"2 1 >" ),
  TST(INT32, SUCCESS, "0",	"1 1 >" ),
  TST(INT32, SUCCESS, "1",	"-1 -2 >" ),
  TST(INT32, SUCCESS, "0",	"-2 -1 >" ),
  TST(INT32, SUCCESS, "0",	"0xffffffffffffffffU ->i64 2 >" ),
  // >=i (integer greater-than-or-equal)
  TST(INT32, STACK_UNDERFLOW, "",	">=i" ),
  TST(INT32, STACK_UNDERFLOW, "",	"1 >=i" ),
  TST(INT32, SUCCESS, "1",	"1 \"Hello\" >=i" ),
  TST(INT32, SUCCESS, "0",	"\"Hello\"  1 >=i" ),
  TST(INT32, SUCCESS, "1",	"1.0 1 >=i" ),
  TST(INT32, SUCCESS, "1",	"1 1.0 >=i" ),
  TST(INT32, SUCCESS, "0",	"1 2 >=i" ),
  TST(INT32, SUCCESS, "1",	"2 1 >=i" ),
  TST(INT32, SUCCESS, "1",	"1 1 >=i" ),
  TST(INT32, SUCCESS, "1",	"-1 -2 >=i" ),
  TST(INT32, SUCCESS, "0",	"-2 -1 >=i" ),
  TST(INT32, SUCCESS, "0",	"0xffffffffffffffffU 2 >=i" ),
  // >= (integer greater-than-or-equal)
  TST(INT32, STACK_UNDERFLOW, "",	">=" ),
  TST(INT32, STACK_UNDERFLOW, "",	"1 >=" ),
  TST(INT32, SUCCESS, "0",	"1 \"Hello\" >=" ),
  TST(INT32, SUCCESS, "1",	"\"Hello\"  1 >=" ),
  TST(INT32, SUCCESS, "1",	"1.0 1 >=" ),
  TST(INT32, SUCCESS, "1",	"1 1.0 >=" ),
  TST(INT32, SUCCESS, "0",	"1 2 >=" ),
  TST(INT32, SUCCESS, "1",	"2 1 >=" ),
  TST(INT32, SUCCESS, "1",	"1 1 >=" ),
  TST(INT32, SUCCESS, "1",	"-1 -2 >=" ),
  TST(INT32, SUCCESS, "0",	"-2 -1 >=" ),
  TST(INT32, SUCCESS, "0",	"0xffffffffffffffffU ->i64 2 >=" ),
  // <i (integer less-than)
  TST(INT32, STACK_UNDERFLOW, "",	"<i" ),
  TST(INT32, STACK_UNDERFLOW, "",	"1 <i" ),
  TST(INT32, SUCCESS, "0",	"1 \"Hello\" <i" ),
  TST(INT32, SUCCESS, "1",	"\"Hello\"  1 <i" ),
  TST(INT32, SUCCESS, "0",	"1.0 1 <i" ),
  TST(INT32, SUCCESS, "0",	"1 1.0 <i" ),
  TST(INT32, SUCCESS, "1",	"1 2 <i" ),
  TST(INT32, SUCCESS, "0",	"2 1 <i" ),
  TST(INT32, SUCCESS, "0",	"1 1 <i" ),
  TST(INT32, SUCCESS, "0",	"-1 -2 <i" ),
  TST(INT32, SUCCESS, "1",	"-2 -1 <i" ),
  TST(INT32, SUCCESS, "1",	"0xffffffffffffffffU 2 <i" ),
  // < (integer less-than)
  TST(INT32, STACK_UNDERFLOW, "",	"<" ),
  TST(INT32, STACK_UNDERFLOW, "",	"1 <" ),
  TST(INT32, SUCCESS, "1",	"1 \"Hello\" <" ),
  TST(INT32, SUCCESS, "0",	"\"Hello\"  1 <" ),
  TST(INT32, SUCCESS, "0",	"1.0 1 <" ),
  TST(INT32, SUCCESS, "0",	"1 1.0 <" ),
  TST(INT32, SUCCESS, "1",	"1 2 <" ),
  TST(INT32, SUCCESS, "0",	"2 1 <" ),
  TST(INT32, SUCCESS, "0",	"1 1 <" ),
  TST(INT32, SUCCESS, "0",	"-1 -2 <" ),
  TST(INT32, SUCCESS, "1",	"-2 -1 <" ),
  TST(INT32, SUCCESS, "1",	"0xffffffffffffffffU ->i64 2 <" ),
  // <=i (integer less-than-or-equal)
  TST(INT32, STACK_UNDERFLOW, "",	"<=i" ),
  TST(INT32, STACK_UNDERFLOW, "",	"1 <=i" ),
  TST(INT32, SUCCESS, "0",	"1 \"Hello\" <=i" ),
  TST(INT32, SUCCESS, "1",	"\"Hello\"  1 <=i" ),
  TST(INT32, SUCCESS, "1",	"1.0 1 <=i" ),
  TST(INT32, SUCCESS, "1",	"1 1.0 <=i" ),
  TST(INT32, SUCCESS, "1",	"1 2 <=i" ),
  TST(INT32, SUCCESS, "0",	"2 1 <=i" ),
  TST(INT32, SUCCESS, "1",	"1 1 <=i" ),
  TST(INT32, SUCCESS, "0",	"-1 -2 <=i" ),
  TST(INT32, SUCCESS, "1",	"-2 -1 <=i" ),
  TST(INT32, SUCCESS, "1",	"0xffffffffffffffffU 2 <=i" ),
  // <= (integer less-than-or-equal)
  TST(INT32, STACK_UNDERFLOW, "",	"<=" ),
  TST(INT32, STACK_UNDERFLOW, "",	"1 <=" ),
  TST(INT32, SUCCESS, "1",	"1 \"Hello\" <=" ),
  TST(INT32, SUCCESS, "0",	"\"Hello\"  1 <=" ),
  TST(INT32, SUCCESS, "1",	"1.0 1 <=" ),
  TST(INT32, SUCCESS, "1",	"1 1.0 <=" ),
  TST(INT32, SUCCESS, "1",	"1 2 <=" ),
  TST(INT32, SUCCESS, "0",	"2 1 <=" ),
  TST(INT32, SUCCESS, "1",	"1 1 <=" ),
  TST(INT32, SUCCESS, "0",	"-1 -2 <=" ),
  TST(INT32, SUCCESS, "1",	"-2 -1 <=" ),
  TST(INT32, SUCCESS, "1",	"0xffffffffffffffffU ->i64 2 <=" ),
  // negf (floating-point unary negation)
  TST(DOUBLE, STACK_UNDERFLOW, "",	"negf" ),
  TST(DOUBLE, SUCCESS, "0.0",	"\"A\" negf" ),
  TST(DOUBLE, SUCCESS, "-1.0",	"1 negf" ),
  TST(DOUBLE, SUCCESS, "0.0",	"null negf" ),
  TST(DOUBLE, SUCCESS, "-1",	"1.0 negf" ),
  TST(DOUBLE, SUCCESS, "1",	"-1.0 negf" ),
  TST(DOUBLE, SUCCESS, "-2000.5","2000.5 negf" ),
  TST(DOUBLE, SUCCESS, "2000.5", "-2000.5 negf" ),
  TST(DOUBLE, SUCCESS, "0",	"0.0 negf" ),
  // +f (floating-point addition)
  TST(DOUBLE, STACK_UNDERFLOW, "",	"+f" ),
  TST(DOUBLE, STACK_UNDERFLOW, "",	"1 +f" ),
  TST(DOUBLE, SUCCESS, "1.0",	"1 \"Hello\" +f" ),
  TST(DOUBLE, SUCCESS, "1.0",	"\"Hello\"  1 +f" ),
  TST(DOUBLE, SUCCESS, "2.0",	"1.0 1 +f" ),
  TST(DOUBLE, SUCCESS, "2.0",	"1 1.0 +f" ),
  TST(DOUBLE, SUCCESS, "3.0",	"1.0 2.0 +f" ),
  TST(DOUBLE, SUCCESS, "0",	"1.5 -1.5 +f" ),
  TST(DOUBLE, SUCCESS, "10000.00005",	"10000.0 0.00005 +f" ),
  // -f (floating-point subtraction)
  TST(DOUBLE, STACK_UNDERFLOW, "",	"-f" ),
  TST(DOUBLE, STACK_UNDERFLOW, "",	"1 -f" ),
  TST(DOUBLE, SUCCESS, "1.0",	"1 \"Hello\" -f" ),
  TST(DOUBLE, SUCCESS, "-1.0",	"\"Hello\"  1 -f" ),
  TST(DOUBLE, SUCCESS, "0.0",	"1.0 1 -f" ),
  TST(DOUBLE, SUCCESS, "0.0",	"1 1.0 -f" ),
  TST(DOUBLE, SUCCESS, "-1.0",	"1.0 2.0 -f" ),
  TST(DOUBLE, SUCCESS, "3",	"1.5 -1.5 -f" ),
  TST(DOUBLE, SUCCESS, "9999.99995",	"10000.0 0.00005 -f" ),
  // *f (floating-point multiplication)
  TST(DOUBLE, STACK_UNDERFLOW, "",	"*f" ),
  TST(DOUBLE, STACK_UNDERFLOW, "",	"1 *f" ),
  TST(DOUBLE, SUCCESS, "0.0",	"1 \"Hello\" *f" ),
  TST(DOUBLE, SUCCESS, "0.0",	"\"Hello\"  1 *f" ),
  TST(DOUBLE, SUCCESS, "1.0",	"1.0 1 *f" ),
  TST(DOUBLE, SUCCESS, "1.0",	"1 1.0 *f" ),
  TST(DOUBLE, SUCCESS, "2",	"1.0 2.0 *f" ),
  TST(DOUBLE, SUCCESS, "1",	"-1.0 -1.0 *f" ),
  TST(DOUBLE, SUCCESS, "6.5",	"13.0 0.5 *f" ),
  TST(DOUBLE, SUCCESS, "0",	"-1.4553 0.0 *f" ),
  // /f (floating-point division)
  TST(DOUBLE, STACK_UNDERFLOW, "",	"/f" ),
  TST(DOUBLE, STACK_UNDERFLOW, "",	"1 /f" ),
  TST(DOUBLE, SUCCESS, "inf",	"1 \"Hello\" /f" ),
  TST(DOUBLE, SUCCESS, "0",	"\"Hello\"  1 /f" ),
  TST(DOUBLE, SUCCESS, "1",	"1.0 1 /f" ),
  TST(DOUBLE, SUCCESS, "1",	"1 1.0 /f" ),
  TST(DOUBLE, SUCCESS, "inf",	"2.0 0.0 /f" ),
  TST(DOUBLE, SUCCESS, "4",	"8.0 2.0 /f" ),
  TST(DOUBLE, SUCCESS, "5.5",	"11.0 2.0 /f" ),
  TST(DOUBLE, SUCCESS, "0",	"0.0 30.0 /f" ),
  // ==f (floating-point equal)
  TST(INT32, STACK_UNDERFLOW, "",	"==f" ),
  TST(INT32, STACK_UNDERFLOW, "",	"1 ==f" ),
  TST(INT32, SUCCESS, "0",	"1 \"Hello\" ==f" ),
  TST(INT32, SUCCESS, "0",	"\"Hello\"  1 ==f" ),
  TST(INT32, SUCCESS, "1",	"1.0 1 ==f" ),
  TST(INT32, SUCCESS, "1",	"1 1.0 ==f" ),
  TST(INT32, SUCCESS, "0",	"1.0 2.0 ==f" ),
  TST(INT32, SUCCESS, "1",	"1.0 1.0 ==f" ),
  // >f (floating-point greater-than)
  TST(INT32, STACK_UNDERFLOW, "",	">f" ),
  TST(INT32, STACK_UNDERFLOW, "",	"1 >f" ),
  TST(INT32, SUCCESS, "1",	"1 \"Hello\" >f" ),
  TST(INT32, SUCCESS, "0",	"\"Hello\"  1 >f" ),
  TST(INT32, SUCCESS, "0",	"1.0 1 >f" ),
  TST(INT32, SUCCESS, "0",	"1 1.0 >f" ),
  TST(INT32, SUCCESS, "0",	"1.0 2.0 >f" ),
  TST(INT32, SUCCESS, "1",	"2.0 1.1 >f" ),
  TST(INT32, SUCCESS, "0",	"1.2 1.2 >f" ),
  TST(INT32, SUCCESS, "1",	"-1.34 -2.45 >f" ),
  TST(INT32, SUCCESS, "0",	"-2.78 -1.003  >f" ),
  // >=f (floating-point greater-than-or-equal)
  TST(INT32, STACK_UNDERFLOW, "",	">=f" ),
  TST(INT32, STACK_UNDERFLOW, "",	"1 >=f" ),
  TST(INT32, SUCCESS, "1",	"1 \"Hello\" >=f" ),
  TST(INT32, SUCCESS, "0",	"\"Hello\"  1 >=f" ),
  TST(INT32, SUCCESS, "1",	"1.0 1 >=f" ),
  TST(INT32, SUCCESS, "1",	"1 1.0 >=f" ),
  TST(INT32, SUCCESS, "0",	"1.0 2.0 >=f" ),
  TST(INT32, SUCCESS, "1",	"2.0 1.1 >=f" ),
  TST(INT32, SUCCESS, "1",	"1.2 1.2 >=f" ),
  TST(INT32, SUCCESS, "1",	"-1.34 -2.45 >=f" ),
  TST(INT32, SUCCESS, "0",	"-2.78 -1.003  >=f" ),
  // <f (floating-point less-than)
  TST(INT32, STACK_UNDERFLOW, "",	"<f" ),
  TST(INT32, STACK_UNDERFLOW, "",	"1 <f" ),
  TST(INT32, SUCCESS, "0",	"1 \"Hello\" <f" ),
  TST(INT32, SUCCESS, "1",	"\"Hello\"  1 <f" ),
  TST(INT32, SUCCESS, "0",	"1.0 1 <f" ),
  TST(INT32, SUCCESS, "0",	"1 1.0 <f" ),
  TST(INT32, SUCCESS, "1",	"1.0 2.0 <f" ),
  TST(INT32, SUCCESS, "0",	"2.0 1.1 <f" ),
  TST(INT32, SUCCESS, "0",	"1.2 1.2 <f" ),
  TST(INT32, SUCCESS, "0",	"-1.34 -2.45 <f" ),
  TST(INT32, SUCCESS, "1",	"-2.78 -1.003  <f" ),
  // <=f (floating-point less-than-or-equal)
  TST(INT32, STACK_UNDERFLOW, "",	"<=f" ),
  TST(INT32, STACK_UNDERFLOW, "",	"1 <=f" ),
  TST(INT32, SUCCESS, "0",	"1 \"Hello\" <=f" ),
  TST(INT32, SUCCESS, "1",	"\"Hello\"  1 <=f" ),
  TST(INT32, SUCCESS, "1",	"1.0 1 <=f" ),
  TST(INT32, SUCCESS, "1",	"1 1.0 <=f" ),
  TST(INT32, SUCCESS, "1",	"1.0 2.0 <=f" ),
  TST(INT32, SUCCESS, "0",	"2.0 1.1 <=f" ),
  TST(INT32, SUCCESS, "1",	"1.2 1.2 <=f" ),
  TST(INT32, SUCCESS, "0",	"-1.34 -2.45 <=f" ),
  TST(INT32, SUCCESS, "1",	"-2.78 -1.003  <=f" ),
  // neg (floating-point unary negation)
  TST(DOUBLE, STACK_UNDERFLOW, "",	"neg" ),
  TST(DOUBLE, SUCCESS, "-1.0",	"1.0 neg" ),
  TST(DOUBLE, SUCCESS, "1.0",	"-1.0 neg" ),
  TST(DOUBLE, SUCCESS, "-2000.5","2000.5 neg" ),
  TST(DOUBLE, SUCCESS, "2000.5", "-2000.5 neg" ),
  TST(DOUBLE, SUCCESS, "0",	"0.0 neg" ),
  // + (floating-point addition)
  TST(STR, SUCCESS, "0x1p+0Hello",	"1.0 \"Hello\" +" ),
  TST(STR, SUCCESS, "Hello0x1p+0",	"\"Hello\"  1.0 +" ),
  TST(DOUBLE, SUCCESS, "2.0",	"1.0 1 +" ),
  TST(DOUBLE, SUCCESS, "2.0",	"1 1.0 +" ),
  TST(DOUBLE, SUCCESS, "3.0",	"1.0 2.0 +" ),
  TST(DOUBLE, SUCCESS, "0",	"1.5 -1.5 +" ),
  TST(DOUBLE, SUCCESS, "10000.00005",	"10000.0 0.00005 +" ),
  // - (floating-point subtraction)
  TST(DOUBLE, OPER_UNSUP, "1.0",	"1 \"Hello\" -" ),
  TST(DOUBLE, OPER_UNSUP, "-1.0",	"\"Hello\"  1 -" ),
  TST(DOUBLE, SUCCESS, "0.0",	"1.0 1 -" ),
  TST(DOUBLE, SUCCESS, "0.0",	"1 1.0 -" ),
  TST(DOUBLE, SUCCESS, "-1.0",	"1.0 2.0 -" ),
  TST(DOUBLE, SUCCESS, "3",	"1.5 -1.5 -" ),
  TST(DOUBLE, SUCCESS, "9999.99995",	"10000.0 0.00005 -" ),
  // * (floating-point multiplication)
  TST(DOUBLE, OPER_UNSUP, "0.0",	"1 \"Hello\" *" ),
  TST(DOUBLE, OPER_UNSUP, "0.0",	"\"Hello\"  1 *" ),
  TST(DOUBLE, SUCCESS, "1.0",	"1.0 1 *" ),
  TST(DOUBLE, SUCCESS, "1.0",	"1 1.0 *" ),
  TST(DOUBLE, SUCCESS, "2",	"1.0 2.0 *" ),
  TST(DOUBLE, SUCCESS, "1",	"-1.0 -1.0 *" ),
  TST(DOUBLE, SUCCESS, "6.5",	"13.0 0.5 *" ),
  TST(DOUBLE, SUCCESS, "0",	"-1.4553 0.0 *" ),
  // / (floating-point division)
  TST(DOUBLE, OPER_UNSUP, "inf",	"1 \"Hello\" /" ),
  TST(DOUBLE, OPER_UNSUP, "0",	"\"Hello\"  1 /" ),
  TST(DOUBLE, SUCCESS, "1",	"1.0 1 /" ),
  TST(DOUBLE, SUCCESS, "1",	"1 1.0 /" ),
  TST(DOUBLE, DIVIDE_BY_ZERO, "inf",	"2.0 0.0 /" ),
  TST(DOUBLE, SUCCESS, "4",	"8.0 2.0 /" ),
  TST(DOUBLE, SUCCESS, "5.5",	"11.0 2.0 /" ),
  TST(DOUBLE, SUCCESS, "0",	"0.0 30.0 /" ),
  // == (floating-point equal)
  TST(INT32, SUCCESS, "0",	"1 \"Hello\" ==" ),
  TST(INT32, SUCCESS, "0",	"\"Hello\"  1 ==" ),
  TST(INT32, SUCCESS, "1",	"1.0 1 ==" ),
  TST(INT32, SUCCESS, "1",	"1 1.0 ==" ),
  TST(INT32, SUCCESS, "0",	"1.0 2.0 ==" ),
  TST(INT32, SUCCESS, "1",	"1.0 1.0 ==" ),
  // > (floating-point greater-than)
  TST(INT32, SUCCESS, "0",	"1.0 \"Hello\" >" ),
  TST(INT32, SUCCESS, "1",	"\"Hello\"  1.0 >" ),
  TST(INT32, SUCCESS, "0",	"1.0 1 >" ),
  TST(INT32, SUCCESS, "0",	"1 1.0 >" ),
  TST(INT32, SUCCESS, "0",	"1.0 2.0 >" ),
  TST(INT32, SUCCESS, "1",	"2.0 1.1 >" ),
  TST(INT32, SUCCESS, "0",	"1.2 1.2 >" ),
  TST(INT32, SUCCESS, "1",	"-1.34 -2.45 >" ),
  TST(INT32, SUCCESS, "0",	"-2.78 -1.003  >" ),
  // >= (floating-point greater-than-or-equal)
  TST(INT32, SUCCESS, "0",	"1.0 \"Hello\" >=" ),
  TST(INT32, SUCCESS, "1",	"\"Hello\"  1.0 >=" ),
  TST(INT32, SUCCESS, "1",	"1.0 1 >=" ),
  TST(INT32, SUCCESS, "1",	"1 1.0 >=" ),
  TST(INT32, SUCCESS, "0",	"1.0 2.0 >=" ),
  TST(INT32, SUCCESS, "1",	"2.0 1.1 >=" ),
  TST(INT32, SUCCESS, "1",	"1.2 1.2 >=" ),
  TST(INT32, SUCCESS, "1",	"-1.34 -2.45 >=" ),
  TST(INT32, SUCCESS, "0",	"-2.78 -1.003  >=" ),
  // < (floating-point less-than)
  TST(INT32, SUCCESS, "1",	"1.0 \"Hello\" <" ),
  TST(INT32, SUCCESS, "0",	"\"Hello\"  1.0 <" ),
  TST(INT32, SUCCESS, "0",	"1.0 1 <" ),
  TST(INT32, SUCCESS, "0",	"1 1.0 <" ),
  TST(INT32, SUCCESS, "1",	"1.0 2.0 <" ),
  TST(INT32, SUCCESS, "0",	"2.0 1.1 <" ),
  TST(INT32, SUCCESS, "0",	"1.2 1.2 <" ),
  TST(INT32, SUCCESS, "0",	"-1.34 -2.45 <" ),
  TST(INT32, SUCCESS, "1",	"-2.78 -1.003  <" ),
  // <= (floating-point less-than-or-equal)
  TST(INT32, SUCCESS, "1",	"1.0 \"Hello\" <=" ),
  TST(INT32, SUCCESS, "0",	"\"Hello\"  1.0 <=" ),
  TST(INT32, SUCCESS, "1",	"1.0 1 <=" ),
  TST(INT32, SUCCESS, "1",	"1 1.0 <=" ),
  TST(INT32, SUCCESS, "1",	"1.0 2.0 <=" ),
  TST(INT32, SUCCESS, "0",	"2.0 1.1 <=" ),
  TST(INT32, SUCCESS, "1",	"1.2 1.2 <=" ),
  TST(INT32, SUCCESS, "0",	"-1.34 -2.45 <=" ),
  TST(INT32, SUCCESS, "1",	"-2.78 -1.003  <=" ),
  // abs
  TST(INT64, STACK_UNDERFLOW, "",	"abs" ),
  TST(INT64, SUCCESS, "0",	"\"A\" abs" ),
  TST(INT64, SUCCESS, "1",	"1.0 abs" ),
  TST(INT64, SUCCESS, "0",	"null abs" ),
  TST(INT64, SUCCESS, "1",	"1 abs" ),
  TST(INT64, SUCCESS, "1",	"-1 abs" ),
  TST(INT64, SUCCESS, "2000",	"2000 abs" ),
  TST(INT64, SUCCESS, "2000",	"-2000 abs" ),
  TST(INT64, SUCCESS, "0",	"0 abs" ),
  // floor
  TST(DOUBLE, STACK_UNDERFLOW, "",	"floor" ),
  TST(DOUBLE, SUCCESS, "0",	"\"A\" floor" ),
  TST(DOUBLE, SUCCESS, "1",	"1 floor" ),
  TST(DOUBLE, SUCCESS, "0",	"null floor" ),
  TST(DOUBLE, SUCCESS, "1",	"1.0 floor" ),
  TST(DOUBLE, SUCCESS, "-1",	"-1.0 floor" ),
  TST(DOUBLE, SUCCESS, "1",	" 1.5 floor" ),
  TST(DOUBLE, SUCCESS, "-2",	"-1.5 floor" ),
  TST(DOUBLE, SUCCESS, "0",	"0.0 floor" ),
  // ceil
  TST(DOUBLE, STACK_UNDERFLOW, "",	"ceil" ),
  TST(DOUBLE, SUCCESS, "0",	"\"A\" ceil" ),
  TST(DOUBLE, SUCCESS, "1",	"1 ceil" ),
  TST(DOUBLE, SUCCESS, "0",	"null ceil" ),
  TST(DOUBLE, SUCCESS, "1",	"1.0 ceil" ),
  TST(DOUBLE, SUCCESS, "-1",	"-1.0 ceil" ),
  TST(DOUBLE, SUCCESS, "2",	" 1.5 ceil" ),
  TST(DOUBLE, SUCCESS, "-1",	"-1.5 ceil" ),
  TST(DOUBLE, SUCCESS, "0",	"0.0 ceil" ),
  // ->i32
  TST(INT32, STACK_UNDERFLOW, "",	"->i32" ),
  TST(INT32, SUCCESS, "1",	"1 ->i32" ),
  TST(INT32, SUCCESS, "-1",	"-1 ->i32" ),
  TST(INT32, SUCCESS, "1316134911","9999999999999 ->i32"),
  TST(INT32, SUCCESS, "1316134911","9999999999999 ->i32"),
  TST(INT32, SUCCESS, "1",	"1.0 ->i32" ),
  TST(INT32, SUCCESS, "1",	"\"1\" ->i32" ),
  TST(INT32, SUCCESS, "0",	"1 not ->i32" ),
  TST(INT32, SUCCESS, "",	"null ->i32" ),
  // ->u32
  TST(UINT32, STACK_UNDERFLOW, "",	"->u32" ),
  TST(UINT32, SUCCESS, "1",	"1 ->u32" ),
  TST(UINT32, SUCCESS, "0xffffffffUL","-1 ->u32" ),
  TST(UINT32, SUCCESS, "1316134911","9999999999999 ->u32"),
  TST(UINT32, SUCCESS, "1316134911","9999999999999 ->u32"),
  TST(UINT32, SUCCESS, "1",	"1.0 ->u32" ),
  TST(UINT32, SUCCESS, "1",	"\"1\" ->u32" ),
  TST(UINT32, SUCCESS, "0",	"1 not ->u32" ),
  TST(UINT32, SUCCESS, "",	"null ->u32" ),
  // ->i64
  TST(INT64, STACK_UNDERFLOW, "",	"->i64" ),
  TST(INT64, SUCCESS, "1",	"1 ->i64" ),
  TST(INT64, SUCCESS, "-1",	"-1 ->i64" ),
  TST(INT64, SUCCESS, "9999999999999","9999999999999 ->i64"),
  TST(INT64, SUCCESS, "9999999999999","9999999999999 ->i64"),
  TST(INT64, SUCCESS, "1",	"1.0 ->i64" ),
  TST(INT64, SUCCESS, "1",	"\"1\" ->i64" ),
  TST(INT64, SUCCESS, "0",	"1 not ->i64" ),
  TST(INT64, SUCCESS, "",	"null ->i64" ),
  // ->u64
  TST(UINT64, STACK_UNDERFLOW, "",	"->u64" ),
  TST(UINT64, SUCCESS, "1",	"1 ->u64" ),
  TST(UINT64, SUCCESS, "0xffffffffffffffffULL","-1 ->u64" ),
  TST(UINT64, SUCCESS, "9999999999999","9999999999999 ->u64"),
  TST(UINT64, SUCCESS, "9999999999999","9999999999999 ->u64"),
  TST(UINT64, SUCCESS, "1",	"1.0 ->u64" ),
  TST(UINT64, SUCCESS, "1",	"\"1\" ->u64" ),
  TST(UINT64, SUCCESS, "0",	"1 not ->u64" ),
  TST(UINT64, SUCCESS, "",	"null ->u64" ),
  // ->str
  TST(STR, STACK_UNDERFLOW, "",	"->str" ),
  TST(STR, SUCCESS, "1",	"1 ->str" ),
  TST(STR, SUCCESS, "-1",	"-1 ->str" ),
  TST(STR, SUCCESS, "9999999999999","9999999999999 ->str"),
  TST(STR, SUCCESS, "9999999999999","9999999999999 ->str"),
  TST(STR, SUCCESS, "0x1p+0",	"1.0 ->str" ),
  TST(STR, SUCCESS, "1",	"\"1\" ->str" ),
  TST(STR, SUCCESS, "0",	"1 not ->str" ),
  TST(STR, SUCCESS, "NULL",	"null ->str" ),
  // ->dbl
  TST(DOUBLE, STACK_UNDERFLOW, "",	"->dbl" ),
  TST(DOUBLE, SUCCESS, "1",	"1 ->dbl" ),
  TST(DOUBLE, SUCCESS, "-1",	"-1 ->dbl" ),
  TST(DOUBLE, SUCCESS, "9999999999999","9999999999999 ->dbl"),
  TST(DOUBLE, SUCCESS, "9999999999999","9999999999999 ->dbl"),
  TST(DOUBLE, SUCCESS, "1",	"1.0 ->dbl" ),
  TST(DOUBLE, SUCCESS, "1",	"\"1\" ->dbl" ),
  TST(DOUBLE, SUCCESS, "0",	"1 not ->dbl" ),
  TST(DOUBLE, SUCCESS, "0",	"null ->dbl" ),
  // ->t
  TST(TUPLE, STACK_UNDERFLOW, "",	"->t"),
  TST(TUPLE, SUCCESS, "1",		"0 ->t"),
  // append
  TST(TUPLE, STACK_UNDERFLOW, "",	"append"),
  TST(TUPLE, STACK_UNDERFLOW, "",	"6 append"),
  TST(TUPLE, SUCCESS, "",		"0 ->t 7 append")

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
  { "NULL\t\n pop $2 $4  ->u32", "NULL pop $2 $4 ->u32 ", 1, 5},
  { "1 ifpoptuple", "1 ifpoptuple ", 1, 2},
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

#define FLAG_ERROR std::cerr << __FILE__ ":" << t->line << ": "

void vm_test(Pel_VM &vm, TuplePtr tpl, int i) {
  const ValTest *t = &vtests[i];
  std::cout << "Running: " << t->src << "\n";
  
  boost::shared_ptr<Pel_Program> prog = Pel_Lexer::compile( t->src);
  vm.reset();
  Pel_VM::Error e = vm.execute(*prog, tpl);
  if ( e != t->err ) {
    const char *x = Pel_VM::strerror(t->err);
    const char *r = Pel_VM::strerror(e);
    FLAG_ERROR << "error '" << t->src << "'; '" 
	      << r << "'(" << e << ") instead of expected '" << x << "'\n";
  }
  if ( t->err != Pel_VM::PE_SUCCESS || e != Pel_VM::PE_SUCCESS) {
    return;
  }
  
  ValuePtr top = vm.result_val();
  if ( top->typeCode() != t->t ) {
    FLAG_ERROR <<"error: Bad result type for '" << t->src << "'; '" 
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
  case _tuple:
    // Don't check
    break;
  default:
    std::cerr << "** Unknown type " << t->t << "\n";
    eq = 1;
  }
  if (!eq) { 
    FLAG_ERROR << "Bad result value for '" 
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
    boost::shared_ptr<Pel_Program> prog = Pel_Lexer::compile( t->src);
    if (prog->ops.size() != (uint) t->num_opcodes) {
      std::cerr << "** Bad # opcodes for '" << t->src << "'; " << prog->ops.size() << " instead of expected " << t->num_opcodes << "\n";
    }
    if (prog->const_pool.size() != (uint) t->num_consts) {
      std::cerr << "** Bad # consts for '" << t->src << "'; " << prog->const_pool.size() << " instead of expected " << t->num_consts << "\n";
    }
    string dec = Pel_Lexer::decompile(*prog);
    if (dec != t->disassembly) {
      std::cerr << "** Bad disassembly for '" << t->src << "'; '" << dec << "' instead of expected '" << t->disassembly << "'\n";
    }
  }

  // 
  // Test the VM
  //
  Pel_VM vm;
  TuplePtr tpl = Tuple::mk();
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
