// -*- c-basic-offset: 2; related-file-name: "strToSockaddr.C" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 * DESCRIPTION: Element that translates a string field of the form
 * "addr:port" into the corresponding sockaddr.
 */

#ifndef __STRTOSOCKADDR_H__
#define __STRTOSOCKADDR_H__

#include "element.h"
#include "value.h"

class StrToSockaddr : public Element { 
public:
  StrToSockaddr(string, unsigned);

  ~StrToSockaddr();
  
  /** Overridden to perform the projecting. */
  TuplePtr simple_action(TuplePtr p);

  const char *class_name() const		{ return "StrToSockaddr";}
  const char *processing() const		{ return "a/a"; }
  const char *flow_code() const			{ return "x/x"; }


private:
  /** The field number to translate */
  unsigned _fieldNo;
};


#endif /* __STRTOSOCKADDR_H_ */
