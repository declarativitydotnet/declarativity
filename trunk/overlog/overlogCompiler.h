/*
 * 
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 * DESCRIPTION: 
 */

#ifndef __OVERLOGCOMPILER_H__
#define __OVERLOGCOMPILER_H__

#include <sstream>
#include "element.h"
#include "plumber.h"

class OverlogCompiler : public Element { 
public:
  
  OverlogCompiler(string, PlumberPtr, string);

  const char *class_name() const { return "OverlogCompiler";}
  const char *processing() const { return "h/h"; }
  const char *flow_code() const	 { return "-/-"; }

  int push(int port, TuplePtr, b_cbv cb);

private:
  void compile(string, string, std::ostringstream&);
  string readScript( string fileName );

  PlumberPtr _plumber;
  string     _id;
};


#endif /* __OVERLOGCOMPILER_H__ */
