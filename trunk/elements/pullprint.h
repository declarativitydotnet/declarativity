// -*- c-basic-offset: 2; related-file-name: "element.C" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 * DESCRIPTION: Element which simply prints any tuple pulled by it
 */

#ifndef __PULLPRINT_H__
#define __PULLPRINT_H__

#include "element.h"

class PullPrint : public Element { 
public:
  
  PullPrint();

  const char *class_name() const		{ return "PullPrint";}
  const char *processing() const		{ return PULL; }

  void run();
  void wakeup();
  
private:
  cbv _wakeup_cb;
  cbv _run_cb;

};


#endif /* __PULLPRINT_H_ */
