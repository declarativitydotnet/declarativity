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

#include "pel_vm.h"

typedef int(Pel_VM::*Op_fn_t)(u_int32_t);

struct JumpTableEnt_t {
  char *opcode;
  Op_fn_t fn;
};

const char *Pel_VM::err_msgs[] = {
  "Success",
  "Type conversion",
  "Invalid Errno",
  "Unknown Error"
};

#include "pel_opcode_defns.gen.h"

//
// Create the VM
//
Pel_VM::Pel_VM() : st() {
  reset();
}

//
// Reset the VM
//
void Pel_VM::reset() {
  while (!st.empty()) {
    st.pop();
  }
}

//
// Return a result
//
TupleFieldRef Pel_VM::result_val() const {
  return st.top();
}

// 
// Make a tuple out of the top elements on the stack and return it
// 
TupleRef Pel_VM::result_tuple() {
  if (!result) {
    result = Tuple::mk();
  }
  return result;
}

//
// Convert an error message into a string
//
const char *Pel_VM::strerror(Pel_VM::Error e) {
  if (e < 0 | e > E_UNKNOWN ) {
    e = E_INVALIDERRNO;
  }
  return err_msgs[e];
}

/***********************************************************************
 *
 * Opcode definitions follow
 *
 */

/*
 * End of file 
 */
