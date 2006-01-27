// -*- c-basic-offset: 2; related-file-name: "pelScan.h" -*-
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

#include "pelScan.h"
#include "pel_lexer.h"

PelScan::PelScan(string name,
                 TablePtr table,
                 unsigned fieldNo,
                 string startup,
                 string scan,
                 string cleanup)
  : Element(name, 1, 1),
    _table(table),
    _iterator(table->scanAll(fieldNo)),
    _pushCallback(0),
    _pullCallback(0),
    _indexFieldNo(fieldNo),
    _startup(Pel_Lexer::compile(startup.c_str())),
    _scan(Pel_Lexer::compile(scan.c_str())),
    _cleanup(Pel_Lexer::compile(cleanup.c_str())),
    _vm()
{
}

int
PelScan::push(int port, TuplePtr t, b_cbv cb)
{
  // Is this the right port?
  assert(port == 0);

  // Do I have a scan pending?
  if (_scanTuple == NULL) {
    // No pending scan.  Take it in
    assert(!_pushCallback);

    // Establish the scan and run the startup script
    _scanTuple = t;
    _vm.reset();
    Pel_VM::Error e = _vm.execute(*_startup, _scanTuple);
    if (e != Pel_VM::PE_SUCCESS) {
      // Reject the push and warn
      log(LoggerI::ERROR, 0, string("push: startup script: ") +
          Pel_VM::strerror(e));
      _vm.dumpStack(string("startup script"));
    }

    // Groovy.  Signal puller that we're ready to give results
    log(LoggerI::INFO, 0, string("push: accepted scan for ") + _scanTuple->toString());
    
    // Unblock the puller if one is waiting
    if (_pullCallback) {
      log(LoggerI::INFO, 0, "push: wakeup puller");
      _pullCallback();
      _pullCallback = 0;
    }
    
    // Fetch the iterator
    _iterator = _table->scanAll(_indexFieldNo);
    
    // And stop the pusher since we have to wait until the iterator is
    // flushed one way or another
    _pushCallback = cb;
    return 0;
  } else {
    // We already have a lookup pending
    assert(!_pushCallback);
    log(LoggerI::WARN, 0, "push: lookup overrun");
    return 0;
  }
}


TuplePtr PelScan::pull(int port, b_cbv cb) 
{
  // Is this the right port?
  assert(port == 0);
  
  // Do I have a pending scan?
  if (_scanTuple == NULL) {
    // Nope, no pending scan.  Underrun
  
    assert(_scanTuple == NULL);
    
    if (!_pullCallback) {
      // Accept the callback
      log(LoggerI::INFO, 0, "pull: raincheck");
      _pullCallback = cb;
    } else {
      // I already have a pull callback
      log(LoggerI::INFO, 0, "pull: callback underrun");
    }
    return TuplePtr();
  }

  // OK, proceed with existing scan.
  while (true) {
    // Does the iterator still have elements?
    if (!_iterator->done()) {
      // Run the scan script on this tuple
      Pel_VM::Error e = _vm.execute(*_scan, _iterator->next());
      if (e != Pel_VM::PE_SUCCESS) {
        log(LoggerI::ERROR, 0, string("pull: scan script:") + Pel_VM::strerror(e));
        _vm.dumpStack(string("scan script"));
        return TuplePtr();
      }
      
      // Did we get a result?
      TuplePtr result = _vm.result_tuple();
      _vm.reset_result_tuple();
      if (result == NULL) {
        // Keep going
      } else {
        // Woo hoo, we got a result
        return result;
      }
    } else {
      // We've run out of tuples.   Just clean up.

      // Wake up any pusher
      if (_pushCallback) {
        log(LoggerI::INFO, 0, "pull: wakeup pusher");
        _pushCallback();
        _pushCallback = 0;
      }

      // The cleanup script
      Pel_VM::Error e = _vm.execute(*_cleanup, _scanTuple);
      if (e != Pel_VM::PE_SUCCESS) {
        log(LoggerI::ERROR, 0, string("pull: cleanup script:") + Pel_VM::strerror(e));
        _scanTuple.reset();
        _vm.dumpStack(string("cleanup script"));
        return TuplePtr();
      }
      _scanTuple.reset();

      
      // Did we get a result?
      TuplePtr result = _vm.result_tuple();
      _vm.reset_result_tuple();
      if (result == NULL) {
        result = Tuple::mk();
      }
      result->tag(PelScan::END_OF_SCAN, Val_Null::mk());
      return result;
    }
  }
}

string PelScan::END_OF_SCAN = "PelScan:END_OF_SCAN";
