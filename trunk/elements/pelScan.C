// -*- c-basic-offset: 2; related-file-name: "pelScan.h" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 */

#include "pelScan.h"
#include "pel_lexer.h"

PelScan::PelScan(str name,
                 TableRef table,
                 unsigned fieldNo,
                 str startup,
                 str scan,
                 str cleanup)
  : Element(name, 1, 1),
    _table(table),
    _iterator(table->scanAll(fieldNo)),
    _pushCallback(cbv_null),
    _pullCallback(cbv_null),
    _indexFieldNo(fieldNo),
    _startup(Pel_Lexer::compile(startup)),
    _scan(Pel_Lexer::compile(scan)),
    _cleanup(Pel_Lexer::compile(cleanup)),
    _vm()
{
  warn << "PELScan\n";
}

PelScan::~PelScan()
{
  if (_startup != 0) {
    delete _startup;
  }
  if (_scan != 0) {
    delete _scan;
  }
  if (_cleanup != 0) {
    delete _cleanup;
  }
}

int
PelScan::push(int port, TupleRef t, cbv cb)
{
  // Is this the right port?
  assert(port == 0);

  // Do I have a scan pending?
  if (_scanTuple == NULL) {
    // No pending scan.  Take it in
    assert(_pushCallback == cbv_null);

    // Establish the scan and run the startup script
    _scanTuple = t;
    _vm.reset();
    Pel_VM::Error e = _vm.execute(*_startup, _scanTuple);
    if (e != Pel_VM::PE_SUCCESS) {
      // Reject the push and warn
      log(LoggerI::ERROR, 0, strbuf("push: startup script: ") <<
          Pel_VM::strerror(e));
      return 1;
    }

    // Groovy.  Signal puller that we're ready to give results
    log(LoggerI::INFO, 0, strbuf("push: accepted scan for ")
        << _scanTuple->toString());
    
    // Unblock the puller if one is waiting
    if (_pullCallback != cbv_null) {
      log(LoggerI::INFO, 0, "push: wakeup puller");
      _pullCallback();
      _pullCallback = cbv_null;
    }
    
    // Fetch the iterator
    _iterator = _table->scanAll(_indexFieldNo);
    
    // And stop the pusher since we have to wait until the iterator is
    // flushed one way or another
    _pushCallback = cb;
    return 0;
  } else {
    // We already have a lookup pending
    assert(_pushCallback != cbv_null);
    log(LoggerI::WARN, 0, "push: lookup overrun");
    return 0;
  }
}


TuplePtr PelScan::pull(int port, cbv cb) 
{
  // Is this the right port?
  assert(port == 0);
  
  // Do I have a pending scan?
  if (_scanTuple == NULL) {
    // Nope, no pending scan.  Underrun
  
    assert(_scanTuple == NULL);
    
    if (_pullCallback == cbv_null) {
      // Accept the callback
      log(LoggerI::INFO, 0, "pull: raincheck");
      _pullCallback = cb;
    } else {
      // I already have a pull callback
      log(LoggerI::INFO, 0, "pull: callback underrun");
    }
    return 0;
  }

  // OK, proceed with existing scan.
  while (true) {
    // Does the iterator still have elements?
    if (!_iterator->done()) {
      // Run the scan script on this tuple
      Pel_VM::Error e = _vm.execute(*_scan, _iterator->next());
      if (e != Pel_VM::PE_SUCCESS) {
        log(LoggerI::ERROR, 0, strbuf("pull: scan script:") << Pel_VM::strerror(e));
        return 0;
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
      if (_pushCallback != cbv_null) {
        log(LoggerI::INFO, 0, "pull: wakeup pusher");
        _pushCallback();
        _pushCallback = cbv_null;
      }

      // The cleanup script
      Pel_VM::Error e = _vm.execute(*_cleanup, _scanTuple);
      if (e != Pel_VM::PE_SUCCESS) {
        log(LoggerI::ERROR, 0, strbuf("pull: cleanup script:") << Pel_VM::strerror(e));
        _scanTuple = NULL;
        return 0;
      }
      _scanTuple = NULL;

      
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

str PelScan::END_OF_SCAN = "PelScan:END_OF_SCAN";
