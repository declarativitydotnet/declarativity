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
 * DESCRIPTION: PEL (P2 Expression Language) virtual machine
 *
 */

#ifndef __PEL_VM_H__
#define __PEL_VM_H__

#include <stack>
#include <vector>
#include <async.h>

#include "tuple.h"
#include "pel_program.h"

class Pel_VM {

public: 
  enum Error {
    PE_SUCCESS=0,
    PE_BAD_CONSTANT,
    PE_BAD_FIELD,
    PE_STACK_UNDERFLOW,
    PE_TYPE_CONVERSION,
    PE_BAD_OPCODE,
    PE_DIVIDE_BY_ZERO,
    PE_INVALID_ERRNO,
    PE_UNKNOWN // Must be the last error
  };

private:
  // Execution state
  std::stack<TupleFieldRef> st;
  const Pel_Program	*prg;
  Error		 error;
  uint		 pc;
  TuplePtr	 result;
  TuplePtr	 operand;

  static const char* err_msgs[];

#include "pel_opcode_decls.gen.h"

  uint64_t pop_unsigned();
  int64_t pop_signed();
  str pop_string();
  double pop_double();

public:
  Pel_VM();

  // 
  // Execution paths
  // 

  // Reset the VM (clear the stack)
  void reset();

  // Execute the program on the tuple. 
  // Return 0 if success, -1 if an error. 
  Error execute(const Pel_Program &prog, const TupleRef data);
  
  // Single step an instruction
  Error execute_instruction( u_int32_t inst, TupleRef data);

  // Return the result (top of the stack)
  TupleFieldRef result_val();
  
  // Return the current result tuple.
  TupleRef result_tuple();

  // Convert the error into a string.
  static const char *strerror(Error e);
};

#endif /* __PEL_VM_H_ */
