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

class Pel_Program;

class Pel_VM {

private:
  std::stack<TupleFieldRef> st;
  static const char* err_msgs[];
  TuplePtr result;

  int pop_int();
  bool top_is_int();

#include "pel_opcode_decls.gen.h"

public:

  enum Error {
    E_SUCCESS=0,
    E_TYPECONV,
    E_INVALIDERRNO,
    E_UNKNOWN // Must be the last error
  };

  Pel_VM();

  // 
  // Execution paths
  // 

  // Reset the VM (clear the stack)
  void reset();

  // Execute the program on the tuple. 
  // Return 0 if success, -1 if an error. 
  Error execute( Pel_Program &prog, TupleRef data);
  
  // Single step an instruction
  Error execute_instruction( u_int32_t inst, TupleRef data);

  // Return the result (top of the stack)
  TupleFieldRef result_val() const;
  
  // Return the current result tuple.
  TupleRef result_tuple();

  // Convert the error into a string.
  static const char *strerror(Error e);
};

#endif /* __PEL_VM_H_ */
