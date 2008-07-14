/*
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

#ifndef __COMPILE_TERMINAL_H__
#define __COMPILE_TERMINAL_H__

#include "element.h"
#include "elementRegistry.h"

class CompileTerminal : public Element { 
public:
  
  CompileTerminal(string name);

  CompileTerminal(TuplePtr args);

  ~CompileTerminal();

  const char *class_name() const { return "CompileTerminal";}
  const char *processing() const { return "/h"; }
  const char *flow_code() const	 { return "/-"; }

  DECLARE_PUBLIC_ELEMENT_INITS

private:
  int initialize();
  void terminal();
  void programUpdate(TuplePtr program);

  DECLARE_PRIVATE_ELEMENT_INITS
};


#endif /* __COMPILE_TERMINAL_H_ */
