// -*- c-basic-offset: 2; related-file-name: "pelTransform.h" -*-
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
 */

#include "pelTransform.h"
#include "pel_lexer.h"

PelTransform::PelTransform(string name, string pelCode)
  : Element(name, 1, 1)
{
  _pelCode = pelCode;
  _program = Pel_Lexer::compile(pelCode.c_str());
}

TuplePtr
PelTransform::simple_action(TuplePtr p)
{
  _vm.reset();
  Pel_VM::Error e = _vm.execute(*_program, p);

  if (e != Pel_VM::PE_SUCCESS) {
    // The transform failed.  Return nothing
    ELEM_ERROR("Pel VM execution on "
               << p->toString()
               << " failed.");
    return TuplePtr();
  } else {
    // The transform succeeded.

    // Was there a result tuple?
    TuplePtr result = _vm.result_tuple();
    if (result == 0) {
      // No result tuple or empty result tuple.  Return nothing
      ELEM_WORDY("Pel VM execution on "
                 << p->toString()
                 << " succeeded but yielded nothing");
      return TuplePtr();
    } else {
      // This tuple better have at least a single field
      ELEM_WORDY("Pel VM execution on "
                 << p->toString()
                 << " succeeded and yielded "
                 << result->toString());
      return result;
    }
  }
}


string
PelTransform::pelCode()
{
  return _pelCode;
}
