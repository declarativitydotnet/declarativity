// -*- c-basic-offset: 2; related-file-name: "element.C" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 * DESCRIPTION: Generate a fixed number of tuples as quickly as
 *    possible and push them.
 */

#ifndef __SIMPLEPUSHGENERATOR_H__
#define __SIMPLEPUSHGENERATOR_H__

#include "element.h"

class SimplePushGenerator : public Element { 
public:
  
  SimplePushGenerator();
  
  int push(int port, TupleRef t, cbv cb);
  const char *class_name() const	{ return "SimplePushGenerator";}
  const char *processing() const	{ return PUSH; }
  
};


#endif /* __SIMPLEPUSHGENERATOR_H_ */
