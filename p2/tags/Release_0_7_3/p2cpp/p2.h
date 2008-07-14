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
  /** Any combination of the following transport layer options
   *  is supported by this interface. To turn on a transport layer
   *  feature simply pass the desired TransportConf value to the
   *  P2 class constructor. 
   *  For example: P2("localhost", 10000, P2::RELIABLE | P2::RCC);
   *  This enables reliable delivery and rate based congestion
   *  control in the started P2 instance. 
   */
  enum TransportConf {NONE=0, RELIABLE=1, ORDERED=2, CC=4, RCC=5};

  /** A handle to return to a subscriber of an event */
  class CallbackHandle {
  private:
    friend class P2;
    CallbackHandle(string name, uint port) 
      : _valid(true), _name(name), _port(port) {};

    bool   _valid;
    string _name;
    uint   _port;
  };
  typedef boost::shared_ptr<CallbackHandle> CallbackHandlePtr;

  /**
   * Creates a stub P2 installation that receives and
   * sends tuple on hostname:port */
  P2(string hostname,
     string port,
     uint transport_conf,
     LoggerI::Level level);

  /**
   * Run the event loop. This call will block the main
   * thread, and all further interactions with P2 must be
   * done through the callback routines.
   */
  void run();

  /**
   * Install a program in the P2 system. The type
   * argument can be either "overlog" or "dataflow", indicating
   * an overlog program or P2DL script in the program argument.
   */ 
  int install(string type, string program);

  /**
   * Register the callback to be called when a tuple
   * with tupleName arrives. If no rule strand exists
   * for the tupleName then the tupleName is assumed 
   * to be an event, disallowing any further materialized
   * statements on the tupleName. Further rule strands
   * can be installed only if they treat tupleName as an
   * event tuple. 
   * RETURN: The port number is returned to the caller of this
   *         return. In order to cancel the callback, this port
   *         number must be given to the unsubscribe method. */
  CallbackHandlePtr subscribe(string tupleName, cb_tp callback);

  /**
   * Cancel the callback associated with handle. */
  void unsubscribe(CallbackHandlePtr handle);

  /**
   * Insert a tuple into the system */
  int tuple(TuplePtr tp);

private:
  string stub(string, string);
  void compileOverlog(string, std::ostringstream&);

  PlumberPtr _plumber;
  string     _id;
  uint       _transport_conf;

  TupleSourceInterface* _tupleSourceInterface;

  boost::python::api::object _parser;
};


#endif /* __P2_H_ */
