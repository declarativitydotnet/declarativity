// -*- c-basic-offset: 2; related-file-name: "pel_vm.C" -*-
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
 * DESCRIPTION: PEL (P2 Expression Language) virtual machine
 *
 */

#ifndef __PEL_VM_H__
#define __PEL_VM_H__

#include <deque>
#include <vector>

#include "val_list.h"
#include "tuple.h"
#include "pel_program.h"
#include "ID.h"
#include "loop.h"

class Pel_VM {

public: 
  enum Error {
    PE_SUCCESS=0,
    PE_BAD_CONSTANT,
    PE_BAD_FIELD,
    PE_OPER_UNSUP,
    PE_STACK_UNDERFLOW,
    PE_TYPE_CONVERSION,
    PE_BAD_OPCODE,
    PE_DIVIDE_BY_ZERO,
    PE_BAD_STRING_OP,
    PE_INVALID_ERRNO,
    PE_STOP,
    PE_UNKNOWN // Must be the last error
  };

#include "pel_opcode_decls.gen.h"

private:
  // Execution state
  std::deque<ValuePtr> _st;
  const Pel_Program	*prg;
  Error		 error;
  uint		 pc;
  TuplePtr	 result;
  TuplePtr	 operand;

  static const char* err_msgs[];

  ValuePtr pop();
  uint64_t pop_unsigned();
  int64_t pop_signed();
  std::string pop_string();
  double pop_double();
  boost::posix_time::ptime pop_time();
  boost::posix_time::time_duration pop_time_duration();
  IDPtr pop_ID();


  // Deque to stack conversion facilities
  inline void stackPop() {  _st.pop_back(); }
  inline void stackPush(ValuePtr v) { _st.push_back(v); }
  inline ValuePtr stackTop() { return _st.back(); }
  inline ValuePtr stackPeek(unsigned p) { return _st[_st.size() - 1 - p]; }


public:
  Pel_VM();
  Pel_VM(std::deque< ValuePtr >);

  // 
  // Execution paths
  // 

  // Reset the VM (clear the stack)
  void reset();

  // Stop execution without error
  void stop();

  /** Dump the stack */
  void dumpStack(string);

  // Execute the program on the tuple. 
  // Return 0 if success, -1 if an error. 
  Error execute(const Pel_Program &prog, const TuplePtr data);
  
  // Single step an instruction
  Error execute_instruction( u_int32_t inst, TuplePtr data);

  // Return the result (top of the stack)
  ValuePtr result_val();
  
  // Return the current result tuple.
  TuplePtr result_tuple();

  // Reset the result tuple to nothingness
  void reset_result_tuple();

  // Convert the error into a string.
  static const char *strerror(Error e);
};

#endif /* __PEL_VM_H_ */
