// -*- c-basic-offset: 2; related-file-name: "scan.h" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 */

#include "scan.h"

Scan::Scan(str name,
	   Table::UniqueScanIterator iterator,
	   bool continuous)
  : Element(name, 0, 1),
    _iterator(iterator),
    _pullCB(0)
{
  _firstTime = true;
  _continuous = continuous; 

  if (continuous) {
    _iterator->addListener(boost::bind(&Scan::listener, this, _1));
  }
}

void
Scan::listener(TupleRef t)
{
  log(LoggerI::INFO, 0, str(strbuf() << "Listener " << t->toString()));
  scanBuffer.push_back(t);
  if (_pullCB) {
    _pullCB();
    _pullCB = 0;
  }
}

TuplePtr Scan::pull(int port, b_cbv cb) 
{
  
  // Is this the right port?
  assert(port == 0);

  if (_firstTime) {
    // buffer up all the entries     
    while (_iterator->done() == false) {
      scanBuffer.push_back(_iterator->next());
    }
    _firstTime = false;
  }

  if (scanBuffer.size() == 0) { 
    _pullCB = cb;
    return 0; 
  }
  TuplePtr retTuple = scanBuffer.front();
  scanBuffer.pop_front();
  log(LoggerI::INFO, 0, str(strbuf() << "Pull returns " << retTuple->toString()));
  //warn << "Pull returns " << retTuple->toString() << "\n";
  return retTuple;
}

// add update callbacks
