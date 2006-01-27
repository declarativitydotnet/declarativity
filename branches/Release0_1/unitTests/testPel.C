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

// The test macro
#define TST(_e,_l,_s,_c) { _c;                                          \
    BOOST_CHECK_MESSAGE(fb.length() == _l, "Bad fdbuf length");         \
    BOOST_CHECK_MESSAGE(fb.last_errno() == _e, "Bad errno");            \
    BOOST_CHECK_MESSAGE(fb.str() == _s, "Bad fdbuf contents");          \
}


class testPel
{
private:


public:
  testPel()
  {
  }



private:
  ////////////////////////////////////////////////////////////
  // Compiler Tests
  ////////////////////////////////////////////////////////////
  
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
  
  static const size_t num_ctests;

public:
  
  void
  compilerTests()
  {
    for(size_t i = 0;
        i < num_ctests;
        i++) {
      const CompilerTest *t = &ctests[i];

      boost::shared_ptr<Pel_Program> prog = Pel_Lexer::compile(t->src);

      // Check opcodes left
      std::ostringstream message1;
      message1 << "Bad # opcodes for '"
               << t->src
               << "'; "
               << prog->ops.size()
               << " instead of expected "
               << t->num_opcodes;
      BOOST_CHECK_MESSAGE(prog->ops.size() == (uint) t->num_opcodes,
                          message1.str().c_str());

      
      // Check constants in poll
      std::ostringstream message2;
      message2 << "Bad # consts for '"
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
      message3 << "Bad disassembly for '"
               << t->src
               << "'; '"
               << dec
               << "' instead of expected '"
               << t->disassembly;
      BOOST_CHECK_MESSAGE(dec == t->disassembly,
                          message3.str().c_str());
    }  
  }
};


const testPel::CompilerTest
testPel::ctests[] = {
  { "  ", "", 0, 0 },
  { "1 2 3", "1 2 3 ", 3, 3},
  { "1 1.2 \"String\" swap dup ", "1 1.2 \"String\" swap dup ", 3, 5},
  { "NULL\t\n pop $2 $4  ->u32", "NULL pop $2 $4 ->u32 ", 1, 5},
  { "1 ifpoptuple", "1 ifpoptuple ", 1, 2},
  { "1 2 /* This is a comment */ pop pop", "1 2 pop pop ", 2, 4}
};
  
const size_t
testPel::num_ctests = sizeof(testPel::ctests) /
                  sizeof(testPel::CompilerTest);





class testPel_testSuite
  : public boost::unit_test_framework::test_suite
{
public:
  testPel_testSuite()
    : boost::unit_test_framework::test_suite("testPel")
  {
    boost::shared_ptr<testPel> instance(new testPel());
    
    
    add(BOOST_CLASS_TEST_CASE(&testPel::compilerTests,
                              instance));
  }
};
