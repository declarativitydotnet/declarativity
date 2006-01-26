// -*- c-basic-offset: 2; related-file-name: "timestampSource.C" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 * DESCRIPTION: Element that produces time stamps whenever pulled.  It
 * never blocks.
 */


#ifndef __TIMESTAMP_SOURCE_H__
#define __TIMESTAMP_SOURCE_H__

#include <element.h>

class TimestampSource : public Element { 
 public:
  
  TimestampSource(string name);

  const char *class_name() const		{ return "TimestampSource"; }
  const char *flow_code() const			{ return "/-"; }
  const char *processing() const		{ return "/l"; }

  virtual TuplePtr pull(int port, b_cbv cb);

 private:
};

#endif /* __TIMESTAMP_SOURCE_H_ */
