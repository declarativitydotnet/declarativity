/*
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 * DESCRIPTION: Element which simply prints any tuple that passes
 * through it.
 */

#ifndef __P2_H__
#define __P2_H__

#include <Python.h>
#include <boost/python.hpp>

#include "plumber.h"
#include "element.h"
#include "elementSpec.h"
#include "tupleListener.h"
#include "tupleSourceInterface.h"

class P2 { 
public:

  P2(string hostname, string port);

  void run();

  int install(string type, string program);

  int callback(string tupleName, cb_tp callback);

  int tuple(TuplePtr tp);

private:
  string stub(string, string);
  void compileOverlog(string, std::ostringstream&);

  PlumberPtr _plumber;
  string     _id;

  TupleSourceInterface* _tupleSourceInterface;

  boost::python::api::object _parser;
};


#endif /* __P2_H_ */
