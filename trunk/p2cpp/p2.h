/*
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 */

#ifndef __P2_H__
#define __P2_H__

#include "plumber.h"
#include "element.h"
#include "elementSpec.h"
#include "tupleListener.h"
#include "tupleInjector.h"
#include "udp.h"

class P2 { 
public:
  /** A handle to return to a subscriber of an event. */
  class CallbackHandle {
    friend class P2;

  public:
    CallbackHandle(ElementSpecPtr listener);

  private:
    ElementSpecPtr _listener;
  };


  /** A handle to a P2 dataflow. It contains the required state for
      installing OverLog programs, editing them at runtime, subscribing
      to events and injecting tuples into them. */
  class DataflowHandle {
    friend class P2;

  public:
    DataflowHandle(PlumberPtr plumber,
                   UdpPtr udp,
                   DataflowPtr dataflow);


    PlumberPtr
    plumber();

  private:
    /** My Plumber */
    PlumberPtr _plumber;


    /** My UDP element */
    UdpPtr _udp;


    /** My dataflow pointer */
    DataflowPtr _dataflow;
  };


  /** Run P2. This call will block the main thread, and all further
      interactions with P2 must be done through the listen/inject
      interface. */
  static void
  run();


  /** Install an OverLog program in the P2 system. Currently,
      installation is only allowed before the system runs and for only a
      single program. Eventually, incremental installation at runtime
      will be enabled. */ 
  static int
  install(string programName,
          string overLogProgram);


  /** Uninstall an OverLog program from the P2 system. Not currently
      supported. */
  static int
  uninstall(string programName);


  /** Register the callback to be called when a tuple with tupleName
      arrives. tupleName cannot be the name of a table (i.e., not a
      materialized relation). In the current preliminary implementation,
      subscriptions must be completed after a program has been compiled
      (via a createDataflow() call) but before the system starts (via a
      run() invocation).  */
  static CallbackHandle
  subscribe(DataflowHandle dfHandle,
            string tupleName,
            TupleListener::TupleCallback callback);


  /** Unsubscribe from a tuple name. Not currently supported. */
  static void
  unsubscribe(DataflowHandle dfHandle,
              CallbackHandle handle);


  /** Insert a tuple into the system. The location specifier is expected
      to be the first field after the tuple name, but otherwise the
      tuple schema is left unspecified.  The callback parameter is
      optional. After an injection, if the dataflow is unable to take
      further tuples, it returns 0 and holds on to the supplied
      callback; when the dataflow can again take more injected tuples it
      invokes the callback.  After an injection, if the dataflow can
      take more tuples, it returns 1.  An application that does not wish
      to track the flow control state of the dataflow can invoke
      injectTuple indiscriminately with a no-op callback (e.g., the
      provided P2::nullCallback()), in which case tuples injected into a
      stalled dataflow are just dropped. */
  static int
  injectTuple(DataflowHandle dfHandle,
              TuplePtr tp,
              b_cbv callback);


  /** This is a no-op callback, for use with injectTuple, when the
      caller does not which to receive flow control callbacks. */
  static void
  nullCallback();


  /** Load up an OverLog program from a file. */
  static std::string
  readOverLogProgram(std::string fileName);



  /** Preprocess and load an OverLog program from a file. Along with the
      OverLog file name, a derived filename base where the
      post-processed file can be stored is also required (typically, as
      derivedFailename.cpp), as well as the macros to be applied as a
      vector of NAME=VALUE assignments. preprocessReadOverLogProgram and
      readOverLogProgram are mutually exclusive; only one of the two
      need be invoked. */
  static std::string
  preprocessReadOverLogProgram(std::string overLogFilename,
                               std::string derivedFilename,
                               std::vector< std::string > definitions);
  

  /** Create (but do not start) a dataflow by the given name on the
      given address for the given program. The booleans
      outputCanonicalForm, outputStages, and outputDot determine where
      certain intermediate debugging information during compilation will
      be displayed. The method returns a pointer to the (single) plumber
      object and the UDP element of the dataflow. */
  static DataflowHandle
  createDataflow(string dataflowName,
                 string myAddress,
                 int port,    // extracted from myAddress for
                 // convenience
                 string derivativeFile,
                 std::string program,
                 bool outputCanonicalForm,
                 bool outputStages,
                 bool outputDot);






private:

  /** No instances of this class can be created */
  P2();


  /** Load all loadable modules before they can be addressed by the
      compilation stages */
  static void
  loadAllModules();
};


#endif /* __P2_H_ */
