// -*- c-basic-offset: 2; related-file-name: "pelTransform.h" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 */

#include "pelTransform.h"
#include "pel_lexer.h"

PelTransform::PelTransform(str pelCode)
  : Element(1, 1)
{
  _program = Pel_Lexer::compile(pelCode);
}

PelTransform::~PelTransform()
{
  if (_program != 0) {
    delete _program;
  }
}

TuplePtr PelTransform::simple_action(TupleRef p)
{
  Pel_VM::Error e = _vm.execute(*_program, p);

  if (e != Pel_VM::PE_SUCCESS) {
    // The transform failed.  Return nothing
    return 0;
  } else {
    // The transform succeeded.

    // Was there a result tuple?
    TuplePtr result = _vm.result_tuple();
    if ((result == 0) || (result->size() == 0)) {
      // No result tuple or empty result tuple.  Return nothing
      return 0;
    } else {
      // This tuple better have at least a single field
      return result;
    }
  }
}
