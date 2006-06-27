/* 
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 */

#include "boost/test/unit_test.hpp"

#include "pel_lexer.h"
#include "pel_program.h"
#include "pel_vm.h"
#include <sstream>

#include "val_null.h"
#include "val_str.h"
#include "val_int32.h"
#include "val_uint32.h"
#include "val_int64.h"
#include "val_uint64.h"
#include "val_double.h"
#include "val_opaque.h"
#include "val_list.h"

#include "testPel.h"

class testPel
{
public:
  testPel()
  {
  }



  ////////////////////////////////////////////////////////////
  // Compiler Tests
  ////////////////////////////////////////////////////////////
  
private:
  struct CompilerTest {
    /** Source code */
    char *src; 

    /** Expected output of disassembler */
    char *disassembly;

    /** Number of constants in the pool afterwards */
    int num_consts;

    /** Generated instructions */
    int	num_opcodes;
  };

  static const CompilerTest ctests[];
  
  static const uint32_t num_ctests;

public:
  
  void
  compilerTests()
  {
    for(uint32_t i = 0;
        i < num_ctests;
        i++) {
      const CompilerTest *t = &ctests[i];

      boost::shared_ptr<Pel_Program> prog = Pel_Lexer::compile(t->src);

      // Check opcodes left
      std::ostringstream message1;
      message1 << "Compiler test "
               << i
               << ". Bad # opcodes for '"
               << t->src
               << "'; "
               << prog->ops.size()
               << " instead of expected "
               << t->num_opcodes;
      BOOST_CHECK_MESSAGE(prog->ops.size() == (uint) t->num_opcodes,
                          message1.str().c_str());

      
      // Check constants in poll
      std::ostringstream message2;
      message2 << "Compiler test "
               << i
               << ". Bad # consts for '"
               << t->src
               << "'; "
               << prog->const_pool.size()
               << " instead of expected "
               << t->num_consts;
      BOOST_CHECK_MESSAGE(prog->const_pool.size() == (uint) t->num_consts,
                          message2.str().c_str());

      
      // Decompile and compare
      std::ostringstream message3;
      std::string dec = Pel_Lexer::decompile(*prog);
      message3 << "Compiler test "
               << i
               << ". Bad disassembly for '"
               << t->src
               << "'; '"
               << dec
               << "' instead of expected '"
               << t->disassembly;
      BOOST_CHECK_MESSAGE(dec == t->disassembly,
                          message3.str().c_str());
    }  
  }










  ////////////////////////////////////////////////////////////
  // Virtual machine tests
  ////////////////////////////////////////////////////////////

private:
  struct ValTest {
    /* Line number of test */
    int line;
    
    /* Type of expected result  */
    Value::TypeCode t;
    
    /* Error (0 = success) */
    Pel_VM::Error err;
    
    /* String representation of value */
    char *val;		
    
    /* Source code */
    char *src;		
  };
  
  const static uint32_t num_vtests;

  const static ValTest vtests[];

  void
  vmTest(Pel_VM &vm, TuplePtr tpl, int i) {
    const ValTest *t = &vtests[i];
    
    boost::shared_ptr<Pel_Program> prog = Pel_Lexer::compile( t->src);
    vm.reset();

    Pel_VM::Error e = vm.execute(*prog, tpl);

    std::string testID;
    {
      std::ostringstream ID;
      ID << "VM test "
         << i
         << " line "
         << t->line;
      testID = ID.str();
    }

    // Check matching return conditions
    BOOST_CHECK_MESSAGE(e == t->err,
                        testID
                        << ". Return condition mismatch on '"
                        << t->src
                        << "'; '"
                        << Pel_VM::strerror(e)
                        << "'("
                        << e
                        << ") instead of expected '"
                        << Pel_VM::strerror(t->err));
    
    if (t->err != Pel_VM::PE_SUCCESS || e != Pel_VM::PE_SUCCESS) {
      return;
    }


    // If the test was to succeed and it did, check result type 
    ValuePtr top = vm.result_val();
   {
      std::ostringstream message;
      message << testID
              << ". Mismatched result type for '"
              << t->src
              << "'; '"
              << top->typeName()
              << "("
              << top->typeCode()
              << ")' instead of expected '"
              << t->t
              << "'";
      BOOST_CHECK_MESSAGE(top->typeCode() == t->t,
                          message.str().c_str());
      //return;
    }


    // If the test succeeded and return the right type, check the
    // returned value
    {
      // Turn the expected return value from string form to whatever
      // type I'm expecting and compare
      // XXX This is really ugly. Replace with ValuePtr.
      int eq;
      switch (t->t) {
      case Value::NULLV:
        eq = 1;
        break;
      case Value::INT32: 
        eq = (strtol(t->val,NULL,0)==Val_Int32::cast(top));
        break;
      case Value::UINT32:  
        eq = (strtoul(t->val,NULL,0)==Val_UInt32::cast(top));
        break;
      case Value::INT64: 
        eq = (strtoll(t->val,NULL,0)==Val_Int64::cast(top));
        break;
      case Value::UINT64:  
        eq = (strtoull(t->val,NULL,0)==Val_UInt64::cast(top));
        break;
      case Value::DOUBLE:
        eq = (strtod(t->val,NULL)==Val_Double::cast(top));
        break;
      case Value::STR:
        eq = (Val_Str::cast(top) == t->val);
        break;
      case Value::LIST:
        eq = (Val_Str::cast(top).compare(t->val) == 0) ? 1 : 0;
      default:
        // Ignore remaining values XXX
        break;
      }
      {
        std::ostringstream message;
        message << testID
                << ". Mismatching stack top for '"
                << t->src
                << "'; expected '"
                << t->val
                << "' but received '"
                << top->toTypeString()
                << "'";
        BOOST_CHECK_MESSAGE(eq, 
                            message.str().c_str());
        return;
      }
    }
  }
  
public:
  
  void
  vmTests()
  {
    Pel_VM vm;
    TuplePtr tpl = Tuple::mk();
    for(uint32_t i = 0;
        i < num_vtests;
        i++) {
      vmTest(vm, tpl, i);
    }
  }
};


////////////////////////////////////////////////////////////
// Compiler test definitions
////////////////////////////////////////////////////////////

const testPel::CompilerTest
testPel::ctests[] = {
  { "  ", "", 0, 0 },
  { "1 2 3", "1 2 3 ", 3, 3},
  { "1 1.2 \"String\" swap dup ", "1 1.2 \"String\" swap dup ", 3, 5},
  { "NULL\t\n pop $2 $4  ->u32", "NULL pop $2 $4 ->u32 ", 1, 5},
  { "1 ifpoptuple", "1 ifpoptuple ", 1, 2},
  { "1 2 /* This is a comment */ pop pop", "1 2 pop pop ", 2, 4}
};
  
const uint32_t
testPel::num_ctests = sizeof(testPel::ctests) /
                  sizeof(testPel::CompilerTest);



////////////////////////////////////////////////////////////
// VM test definitions
////////////////////////////////////////////////////////////

#define TST(_type,_err,_val,_src) {__LINE__,Value::_type,Pel_VM::PE_##_err,_val,_src}

const testPel::ValTest
testPel::vtests[] = {
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
  TST(INT64, SUCCESS, "-1","-1 2 >>" ), // Recall this
                                                         // is with sign
                                                         // extension,
                                                         // so empty
                                                         // bits extend
                                                         // the last bit
  TST(UINT64, SUCCESS, "0x3fffffffffffffffU","-1 ->u64 2 >>" ), // Recall this
                                                         // is with sign
                                                         // extension,
                                                         // so empty
                                                         // bits extend
                                                         // the last bit

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
  TST(INT32, SUCCESS, "0",	"1.0 \"Hello\" <" ),
  TST(INT32, SUCCESS, "0",	"1 \"Hello\" <" ),
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
  TST(INT32, SUCCESS, "0",	"1.0 \"Hello\" <=" ),
  TST(INT32, SUCCESS, "0",	"1 \"Hello\" <=" ),
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
  TST(INT32, SUCCESS, "1",	"1.0 \"Hello\" >" ),
  TST(INT32, SUCCESS, "1",	"1 \"Hello\" >" ),
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
  TST(INT32, SUCCESS, "1",	"1.0 \"Hello\" >=" ),
  TST(INT32, SUCCESS, "1",	"1 \"Hello\" >=" ),
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
  // Since 1.0 seems to print as 1, I'm going to add some tests that 
  // will exercise PEL with the existing checking system. --ACR
  TST(STR, SUCCESS, "1Hello",	"1.0 \"Hello\" strcat" ),
  TST(STR, SUCCESS, "4.23Hello", "4.23 \"Hello\" strcat"),
  TST(STR, SUCCESS, "Hello1",	"\"Hello\" 1.0 strcat" ),
  TST(STR, SUCCESS, "Hello4.23", "\"Hello\" 4.23 strcat" ),
  TST(STR, SUCCESS, "1Hello",	"1 \"Hello\" strcat" ),
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
  TST(UINT32, SUCCESS, "1",	"1.0 strlen" ),
  TST(UINT32, SUCCESS, "4.23", "4.23 strlen" ), 
  TST(UINT32, SUCCESS, "1",	"1 strlen" ),
  TST(UINT32, SUCCESS, "1",	"\"A\" strlen" ),
  TST(UINT32, SUCCESS, "1",	"\"1\" strlen" ),
  TST(UINT32, SUCCESS, "0",	"\"\" strlen" ),
  TST(UINT32, SUCCESS, "1",	"\"\\0\" strlen" ),
  TST(UINT32, SUCCESS, "5",	"\"Hello\" strlen" ),
  TST(UINT32, SUCCESS, "20","\"String\\nwith\\rcontrols\" strlen" ),
  // upper (string to upper case)
  TST(STR, STACK_UNDERFLOW, "",	"upper" ),
  TST(STR, SUCCESS, "1",	"1.0 upper" ), 
  TST(STR, SUCCESS, "4.3", "4.3 upper" ),
  TST(STR, SUCCESS, "4.3", "4.3 lower" ),
  TST(STR, SUCCESS, "1",	"1 upper" ),
  TST(STR, SUCCESS, "A",	"\"A\" upper" ),
  TST(STR, SUCCESS, "A",	"\"a\" upper" ),
  TST(STR, SUCCESS, "",	"\"\" upper" ),
  TST(STR, SUCCESS, "HELLO", "\"Hello\" upper" ),
  TST(STR, SUCCESS, "STRING\nWITH\rCONTROLS","\"String\\nwith\\rcontrols\" upper" ),
  // lower (string to lower case)
  TST(STR, STACK_UNDERFLOW, "",	"lower" ),
  TST(STR, SUCCESS, "1",	"1.0 lower" ),
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
  TST(STR, SUCCESS, "1",	"1 0 1 substr" ),
  TST(STR, SUCCESS, "1",	"1.0 0 1 substr" ),
  TST(STR, SUCCESS, "b",	"\"abcdefg\" 1.0 1 substr" ),
  TST(STR, SUCCESS, "b",	"\"abcdefg\" 1 1.0 substr" ),
  TST(STR, SUCCESS, "a",	"\"abcdefg\" \"a\" 1 substr" ),
  TST(STR, BAD_STRING_OP, "", 	"\"abcdefg\" 1 \"a\" substr" ), // because 0 len is bad
  TST(STR, BAD_STRING_OP, "", 	"\"abcdefg\" 1 null substr" ), // because 0 len is bad
  TST(STR, SUCCESS, "a",	"\"abcdefg\" null 1 substr" ),
  TST(STR, SUCCESS, "U",	"null 1 1 substr" ),
  TST(STR, SUCCESS, "abc",	"\"abcdefg\" 0 3 substr" ),
  TST(STR, SUCCESS, "bcde","\"abcdefg\" 1 4 substr" ),
  TST(STR, SUCCESS, "abcdefg","\"abcdefg\" 0 7 substr" ),
  TST(STR, SUCCESS, "fg",	"\"abcdefg\" 5 2 substr" ),
  TST(STR, BAD_STRING_OP, "",	"\"abcdefg\" 0 0 substr" ),
  TST(STR, BAD_STRING_OP, "",	"\"abcdefg\" 4 0 substr" ),
  TST(STR, BAD_STRING_OP, "",	"\"abcdefg\" 10 0 substr" ),
  TST(STR, BAD_STRING_OP, "",	"\"abcdefg\" 10 3 substr" ),
  // match (Perl regular expression matching: not much testing here
  // yet :-( )
  TST(INT32, STACK_UNDERFLOW, "",	"match" ),
  TST(INT32, STACK_UNDERFLOW, "",	"\"A\" match" ),
  TST(INT32, SUCCESS, "0",	"\"A\" 1 match" ),
  TST(INT32, SUCCESS, "1",	"1 1 match" ),
  TST(INT32, SUCCESS, "1",	"1.0 1 match" ),
  TST(INT32, SUCCESS, "0", "4.3 4 match" ), 
  TST(INT32, SUCCESS, "0",	"\"abcdefg\" 1.0 match" ),
  TST(INT32, SUCCESS, "0",	"\"abcdefg\" 1 match" ),
  TST(INT32, SUCCESS, "0",	"\"abcdefg\" null match" ),
  TST(INT32, SUCCESS, "0",	"null \"A\" match" ),
  TST(INT32, SUCCESS, "1",	"\"abcd\" \"abcd\" match" ),
  TST(INT32, SUCCESS, "1",	"\"abcd\" \"ab.*\" match" ),
  TST(INT32, SUCCESS, "1",	"\"abcd\" \".*cd\" match" ),
  TST(INT32, SUCCESS, "0",	"\"abcd\" \"ab\" match" ),
  TST(INT32, SUCCESS, "0",	"\"abcd\" \"cd\" match" ),
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
  TST(INT64, OPER_UNSUP, "0", "\"A\" neg" ), // turns A into 0
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
  TST(INT64, SUCCESS, "1Hello", "1 \"Hello\" +" ),
  TST(STR, SUCCESS, "Hello1", "\"Hello\"  1 +" ),
  TST(DOUBLE, SUCCESS, "2", "1.0 1 +" ),
  TST(INT64, SUCCESS, "2", "1 1.0 +" ),
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
  TST(INT64, SUCCESS, "1",	"1 \"Hello\" -" ),
  TST(INT64, OPER_UNSUP, "-1",	"\"Hello\"  1 -" ),
  TST(DOUBLE, SUCCESS, "0",	"1.0 1 -" ),
  TST(INT64, SUCCESS, "0",	"1 1.0 -" ),
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
  TST(INT64, SUCCESS, "0",	"1 \"Hello\" *" ),
  TST(INT64, OPER_UNSUP, "0",	"\"Hello\"  1 *" ),
  TST(DOUBLE, SUCCESS, "1",	"1.0 1 *" ),
  TST(INT64, SUCCESS, "1",	"1 1.0 *" ),
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
  TST(INT64, DIVIDE_BY_ZERO, "",	"1 \"Hello\" /" ),
  TST(INT64, OPER_UNSUP, "0",	"\"Hello\"  1 /" ),
  TST(DOUBLE, SUCCESS, "1",	"1.0 1 /" ),
  TST(INT64, SUCCESS, "1",	"1 1.0 /" ),
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
  TST(INT32, SUCCESS, "1",	"1 \"Hello\" >" ),
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
  TST(INT32, SUCCESS, "1",	"1 \"Hello\" >=" ),
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
  TST(INT32, SUCCESS, "0",	"1 \"Hello\" <" ),
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
  TST(INT32, SUCCESS, "0",	"1 \"Hello\" <=" ),
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
  // neg (unary negation, of the operand's type)
  TST(DOUBLE, STACK_UNDERFLOW, "",	"neg" ),
  TST(DOUBLE, SUCCESS, "-1.0",	"1.0 neg" ),
  TST(DOUBLE, SUCCESS, "1.0",	"-1.0 neg" ),
  TST(DOUBLE, SUCCESS, "-2000.5","2000.5 neg" ),
  TST(DOUBLE, SUCCESS, "2000.5", "-2000.5 neg" ),
  TST(DOUBLE, SUCCESS, "0",	"0.0 neg" ),
  TST(INT64, SUCCESS, "-1",	"1 neg" ),
  TST(INT64, SUCCESS, "1",	"-1 neg" ),
  TST(INT64, SUCCESS, "-2000","2000 neg" ),
  TST(INT64, SUCCESS, "2000", "-2000 neg" ),
  TST(INT64, SUCCESS, "0",	"0 neg" ),
  TST(INT32, SUCCESS, "-1",	"1 ->i32 neg" ),
  TST(INT32, SUCCESS, "1",	"-1 ->i32 neg" ),
  TST(INT32, SUCCESS, "-2000","2000 ->i32 neg" ),
  TST(INT32, SUCCESS, "2000", "-2000 ->i32 neg" ),
  TST(INT32, SUCCESS, "0",	"0 ->i32 neg" ),
  // + (floating-point addition)
  TST(DOUBLE, SUCCESS, "1.0",	"1.0 \"Hello\" +" ),
  TST(DOUBLE, SUCCESS, "4.23", "4.23 \"Hello\" +"),
  TST(STR, SUCCESS, "Hello1",	"\"Hello\"  1.0 +" ),
  TST(STR, SUCCESS, "Hello4.23", "\"Hello\" 4.23 +"),
  TST(DOUBLE, SUCCESS, "2.0",	"1.0 1 +" ),
  TST(INT64, SUCCESS, "2",	"1 1.0 +" ),
  TST(DOUBLE, SUCCESS, "3.0",	"1.0 2.0 +" ),
  TST(DOUBLE, SUCCESS, "0",	"1.5 -1.5 +" ),
  TST(DOUBLE, SUCCESS, "10000.00005",	"10000.0 0.00005 +" ),
  // - (polymorphic subtraction)
  TST(INT64, SUCCESS, "1",	"1 \"Hello\" -" ),
  TST(DOUBLE, OPER_UNSUP, "-1.0",	"\"Hello\"  1 -" ),
  TST(DOUBLE, SUCCESS, "0.0",	"1.0 1 -" ),
  TST(INT64, SUCCESS, "0",	"1 1.0 -" ),
  TST(DOUBLE, SUCCESS, "-1.0",	"1.0 2.0 -" ),
  TST(DOUBLE, SUCCESS, "3.0",	"1.5 -1.5 -" ),
  TST(DOUBLE, SUCCESS, "9999.99995",	"10000.0 0.00005 -" ),
  // * (polymorphic multiplication)
  TST(INT64, SUCCESS, "0",	"1 \"Hello\" *" ),
  TST(STR, OPER_UNSUP, "0.0",	"\"Hello\"  1 *" ),
  TST(DOUBLE, SUCCESS, "1.0",	"1.0 1 *" ),
  TST(INT64, SUCCESS, "1",	"1 1.0 *" ),
  TST(DOUBLE, SUCCESS, "2",	"1.0 2.0 *" ),
  TST(DOUBLE, SUCCESS, "1",	"-1.0 -1.0 *" ),
  TST(DOUBLE, SUCCESS, "6.5",	"13.0 0.5 *" ),
  TST(DOUBLE, SUCCESS, "0",	"-1.4553 0.0 *" ),
  // / (polymorphic division)
  TST(INT64, DIVIDE_BY_ZERO, "inf",	"1 \"Hello\" /" ),
  TST(DOUBLE, OPER_UNSUP, "0",	"\"Hello\"  1 /" ),
  TST(DOUBLE, SUCCESS, "1",	"1.0 1 /" ),
  TST(INT64, SUCCESS, "1",	"1 1.0 /" ),
  TST(DOUBLE, DIVIDE_BY_ZERO, "inf",	"2.0 0.0 /" ),
  TST(DOUBLE, SUCCESS, "4",	"8.0 2.0 /" ),
  TST(DOUBLE, SUCCESS, "5.5",	"11.0 2.0 /" ),
  TST(DOUBLE, SUCCESS, "0",	"0.0 30.0 /" ),
  // == (polymorphic equal)
  TST(INT32, SUCCESS, "0",	"1 \"Hello\" ==" ),
  TST(INT32, SUCCESS, "0",	"\"Hello\"  1 ==" ),
  TST(INT32, SUCCESS, "1",	"1.0 1 ==" ),
  TST(INT32, SUCCESS, "1",	"1 1.0 ==" ),
  TST(INT32, SUCCESS, "0",	"1.0 2.0 ==" ),
  TST(INT32, SUCCESS, "1",	"1.0 1.0 ==" ),
  // > (polymorphic greater-than)
  TST(INT32, SUCCESS, "1",	"1.0 \"Hello\" >" ),
  TST(INT32, SUCCESS, "1",	"\"Hello\"  1.0 >" ),
  TST(INT32, SUCCESS, "0",	"1.0 1 >" ),
  TST(INT32, SUCCESS, "0",	"1 1.0 >" ),
  TST(INT32, SUCCESS, "0",	"1.0 2.0 >" ),
  TST(INT32, SUCCESS, "1",	"2.0 1.1 >" ),
  TST(INT32, SUCCESS, "0",	"1.2 1.2 >" ),
  TST(INT32, SUCCESS, "1",	"-1.34 -2.45 >" ),
  TST(INT32, SUCCESS, "0",	"-2.78 -1.003  >" ),
  // >= (floating-point greater-than-or-equal)
  TST(INT32, SUCCESS, "1",	"1.0 \"Hello\" >=" ),
  TST(INT32, SUCCESS, "1",	"\"Hello\"  1.0 >=" ),
  TST(INT32, SUCCESS, "1",	"1.0 1 >=" ),
  TST(INT32, SUCCESS, "1",	"1 1.0 >=" ),
  TST(INT32, SUCCESS, "0",	"1.0 2.0 >=" ),
  TST(INT32, SUCCESS, "1",	"2.0 1.1 >=" ),
  TST(INT32, SUCCESS, "1",	"1.2 1.2 >=" ),
  TST(INT32, SUCCESS, "1",	"-1.34 -2.45 >=" ),
  TST(INT32, SUCCESS, "0",	"-2.78 -1.003  >=" ),
  // < (floating-point less-than)
  TST(INT32, SUCCESS, "0",	"1.0 \"Hello\" <" ),
  TST(INT32, SUCCESS, "0",	"\"Hello\"  1.0 <" ),
  TST(INT32, SUCCESS, "0",	"1.0 1 <" ),
  TST(INT32, SUCCESS, "0",	"1 1.0 <" ),
  TST(INT32, SUCCESS, "1",	"1.0 2.0 <" ),
  TST(INT32, SUCCESS, "0",	"2.0 1.1 <" ),
  TST(INT32, SUCCESS, "0",	"1.2 1.2 <" ),
  TST(INT32, SUCCESS, "0",	"-1.34 -2.45 <" ),
  TST(INT32, SUCCESS, "1",	"-2.78 -1.003  <" ),
  // <= (floating-point less-than-or-equal)
  TST(INT32, SUCCESS, "0",	"1.0 \"Hello\" <=" ),
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
  TST(STR, SUCCESS, "1",	"1.0 ->str" ),
  TST(STR, SUCCESS, "4.23", "4.23 ->str"),
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
  TST(TUPLE, SUCCESS, "",		"0 ->t 7 append"),

// Insert list tests here.

  TST(LIST, STACK_UNDERFLOW, "", "concat"),
  TST(LIST, STACK_UNDERFLOW, "", "null 7 lappend concat"),
  TST(LIST, SUCCESS, "(6, 7)", "null 7 lappend null 6 lappend concat"),
  TST(LIST, SUCCESS, "(6)", "null 6 lappend null concat"),
  TST(LIST, SUCCESS, "(6)", "null null 6 lappend concat"),
  TST(LIST, STACK_UNDERFLOW, "", "lappend"),
  TST(LIST, STACK_UNDERFLOW, "", "6 lappend"),
  TST(LIST, SUCCESS, "(6)", "null 6 lappend"),
  TST(LIST, SUCCESS, "(6, 5)", "null 6 lappend 5 lappend"),
  TST(LIST, SUCCESS, "(6, 5, 4)", "null 6 lappend 5 lappend 4 lappend"),
  TST(LIST, STACK_UNDERFLOW, "", "member"),
  TST(LIST, STACK_UNDERFLOW, "", "6 member"),
  TST(INT32, SUCCESS, "1", "null 6 lappend 6 member"),
  TST(INT32, SUCCESS, "0", "null 6 lappend 4 member"),
  TST(LIST, STACK_UNDERFLOW, "", "intersect"), 
  TST(LIST, STACK_UNDERFLOW, "", "null 6 lappend intersect"),
  TST(LIST, SUCCESS, "(6)", "null 6 lappend 4 lappend 3 lappend null 5 lappend 6 lappend 7 lappend intersect"),
  TST(LIST, SUCCESS, "()", "null 8 lappend 7 lappend 6 lappend null intersect"),
  TST(LIST, SUCCESS, "()", "null null 8 lappend 7 lappend 6 lappend intersect"),
  TST(LIST, SUCCESS, "(2, 3, 3, 4, 4)", "null 1 lappend 2 lappend 3 lappend 2 lappend 3 lappend 4 lappend 2 lappend 4 lappend 2 lappend 4 lappend null 2 lappend 3 lappend 3 lappend 3 lappend 3 lappend 3 lappend 4 lappend 4 lappend msintersect"),
  TST(LIST, SUCCESS, "()", "null null msintersect"), 
  TST(LIST, SUCCESS, "()", "null null 6 lappend msintersect"),
  TST(LIST, SUCCESS, "(1, 1, 1)", "null 1 lappend 1 lappend 1 lappend 1 lappend null 1 lappend 1 lappend 1 lappend msintersect"),
  TST(LIST, SUCCESS, "(1, 1, 1)", "null 1 lappend 1 lappend 1 lappend null 1 lappend 1 lappend 1 lappend 1 lappend msintersect")
};
#undef TST


const uint32_t testPel::num_vtests =
                  sizeof(testPel::vtests) /
                  sizeof(testPel::ValTest);


testPel_testSuite::testPel_testSuite()
  : boost::unit_test_framework::test_suite("testPel")
{
  boost::shared_ptr<testPel> instance(new testPel());
  
  
  add(BOOST_CLASS_TEST_CASE(&testPel::compilerTests,
                            instance));
  add(BOOST_CLASS_TEST_CASE(&testPel::vmTests,
                            instance));
}
