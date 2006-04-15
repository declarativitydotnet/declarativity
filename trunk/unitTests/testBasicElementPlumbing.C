/* 
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 */
#include "boost/test/unit_test.hpp"

#include "tuple.h"
#include "plumber.h"
#include "val_null.h"
#include "val_str.h"
#include "val_int32.h"
#include "val_uint32.h"
#include "val_int64.h"
#include "val_uint64.h"
#include "val_double.h"
#include "val_opaque.h"

#include "print.h"
#include "timedPushSource.h"
#include "timedPullSink.h"
#include "slot.h"

class testBasicElementPlumbing
{
private:
  TuplePtr
  create_tuple(int i) {
    TuplePtr t = Tuple::mk();
    t->append(Val_Null::mk());
    t->append(Val_Int32::mk(i));
    t->append(Val_UInt64::mk(i));
    t->append(Val_Int32::mk(i));
    ostringstream myStringBuf;
    myStringBuf << "This is string '" << i << "'";
    string myString = myStringBuf.str();
    t->append(Val_Str::mk(myString));
    t->freeze();
    return t;
  }

  
public:
  testBasicElementPlumbing()
  {
  }




  ////////////////////////////////////////////////////////////
  // CHECK HOOKUP ELEMENTS
  ////////////////////////////////////////////////////////////
  
  /** Hookup referring to non-existent element. */
  void testCheckHookupElements_NonExistentToElement()
  {
    PlumberPtr plumber(new Plumber());
    Plumber::DataflowPtr conf(new Plumber::Dataflow("testCheckHookupElements_NonExistentToElement"));
    ElementSpecPtr sourceS =
      conf->addElement(ElementPtr(new TimedPushSource("source", 0)));
    ElementSpecPtr sinkS(new ElementSpec(ElementPtr(new TimedPullSink("sink", 0))));
    conf->hookUp(sourceS, 0, sinkS, 0);
  
    BOOST_CHECK_MESSAGE(plumber->install(conf) != 0,
                        "Failed to catch hookup reference to unknown to element");
  }

  /** Non existent from element */
  void testCheckHookupElements_NonExistentFromElement()
  {
    PlumberPtr plumber(new Plumber());
    Plumber::DataflowPtr conf(new Plumber::Dataflow("testCheckHookupElements_NonExistentFromElement"));
    ElementSpecPtr sourceS(new ElementSpec(ElementPtr(new TimedPushSource("source", 0))));
    ElementSpecPtr sinkS = conf->addElement(ElementPtr(new TimedPullSink("sink", 0)));
    conf->hookUp(sourceS, 0, sinkS, 0);
  
    BOOST_CHECK_MESSAGE(plumber->install(conf) != 0,
                        "Failed to catch hookup reference to unknown from element");
  }


  /** From port is negative */
  void testCheckHookupElements_NegativeFromPort()
  {
    PlumberPtr plumber(new Plumber());
    Plumber::DataflowPtr conf(new Plumber::Dataflow("testCheckHookupElements_NegativeFromPort"));
    ElementSpecPtr sourceS = conf->addElement(ElementPtr(new TimedPushSource("source", 0)));
    ElementSpecPtr sinkS = conf->addElement(ElementPtr(new TimedPullSink("sink", 0)));
    conf->hookUp(sourceS, -1, sinkS, 0);
  
    BOOST_CHECK_MESSAGE(plumber->install(conf) != 0,
                        "Failed to catch negative from port");
  }


  /** To port is negative */
  void testCheckHookupElements_NegativeToPort()
  {
    PlumberPtr plumber(new Plumber());
    Plumber::DataflowPtr conf(new Plumber::Dataflow("testCheckHookupElements_NegativeToPort"));
    ElementSpecPtr sourceS = conf->addElement(ElementPtr(new TimedPushSource("source", 0)));
    ElementSpecPtr sinkS = conf->addElement(ElementPtr(new TimedPullSink("sink", 0)));
    conf->hookUp(sourceS, 0, sinkS, -1);
  
    BOOST_CHECK_MESSAGE(plumber->install(conf) != 0,
                        "Failed to catch negative to port");
  }








  ////////////////////////////////////////////////////////////
  // CHECK HOOKUP RANGE
  ////////////////////////////////////////////////////////////

  /** Incorrect from port. */
  void testCheckHookupRange_IncorrectFromPort()
  {
    PlumberPtr plumber(new Plumber());
    Plumber::DataflowPtr conf(new Plumber::Dataflow("testCheckHookupRange_IncorrectFromPort"));
    ElementSpecPtr sourceS =
      conf->addElement(ElementPtr(new TimedPushSource("source", 0)));
    ElementSpecPtr sinkS =
      conf->addElement(ElementPtr(new TimedPullSink("sink", 0)));
    conf->hookUp(sourceS, 1, sinkS, 0);
  
    BOOST_CHECK_MESSAGE(plumber->install(conf) != 0,
                        "Failed to catch incorrect from port");
  }


  /** Incorrect to port. */
  void testCheckHookupRange_IncorrectToPort()
  {
    PlumberPtr plumber(new Plumber());
    Plumber::DataflowPtr conf(new Plumber::Dataflow("testCheckHookupRange_IncorrectToPort"));
    ElementSpecPtr sourceS =
      conf->addElement(ElementPtr(new TimedPushSource("source", 0)));
    ElementSpecPtr sinkS =
      conf->addElement(ElementPtr(new TimedPullSink("sink", 0)));
    conf->hookUp(sourceS, 0, sinkS, 1);
  
    BOOST_CHECK_MESSAGE(plumber->install(conf) != 0,
                        "Failed to catch incorrect to port");
  }


  /** Incorrect ports (both). */
  void testCheckHookupRange_IncorrectPorts()
  {
    PlumberPtr plumber(new Plumber());
    Plumber::DataflowPtr conf(new Plumber::Dataflow("testCheckHookupRange_IncorrectPorts"));
    ElementSpecPtr sourceS =
      conf->addElement(ElementPtr(new TimedPushSource("source", 0)));
    ElementSpecPtr sinkS =
      conf->addElement(ElementPtr(new TimedPullSink("sink", 0)));
    conf->hookUp(sourceS, 1, sinkS, 1);
  
    BOOST_CHECK_MESSAGE(plumber->install(conf) != 0,
                        "Failed to catch incorrect from/to ports");
  }


  /** Portless hookup. */
  void testCheckHookupRange_Portless()
  {
    PlumberPtr plumber(new Plumber());
    Plumber::DataflowPtr conf(new Plumber::Dataflow("testCheckHookupRange_Portless"));
    ElementSpecPtr sourceS =
      conf->addElement(ElementPtr(new TimedPushSource("source", 0)));
    ElementSpecPtr sinkS =
      conf->addElement(ElementPtr(new TimedPullSink("sink", 0)));
    conf->hookUp(sinkS, 1, sourceS, 1);
  
    BOOST_CHECK_MESSAGE(plumber->install(conf) != 0,
                        "Failed to catch portless hookup");
  }


























  ////////////////////////////////////////////////////////////
  // PUSH and PULL semantics
  ////////////////////////////////////////////////////////////



  /** Pull to push. */
  void testCheckPushPull_PullToPush()
  {
    PlumberPtr plumber(new Plumber());
    Plumber::DataflowPtr conf(new Plumber::Dataflow("testCheckPushPull_PullToPush"));
    ElementSpecPtr sourceS = conf->addElement(ElementPtr(new TimedPushSource("source", 0)));
    ElementSpecPtr sinkS = conf->addElement(ElementPtr(new TimedPullSink("sink", 0)));
    ElementSpecPtr slot1S = conf->addElement(ElementPtr(new Slot("slot1")));
    ElementSpecPtr slot2S = conf->addElement(ElementPtr(new Slot("slot2")));
    conf->hookUp(sourceS, 0, slot1S, 0);
    conf->hookUp(slot1S, 0, slot2S, 0);
    conf->hookUp(slot2S, 0, sinkS, 0);
  
    BOOST_CHECK_MESSAGE(plumber->install(conf) != 0,
                        "Failed to catch pull output hooked up with push input");
  }


  /** Pull to Push. */
  void testCheckPushPull_PullToPushHop()
  {
    PlumberPtr plumber(new Plumber());
    Plumber::DataflowPtr conf(new Plumber::Dataflow("testCheckPushPull_PullToPushHop"));
    ElementSpecPtr sourceS = conf->addElement(ElementPtr(new TimedPushSource("source", 0)));
    ElementSpecPtr sinkS = conf->addElement(ElementPtr(new TimedPullSink("sink", 0)));
    ElementSpecPtr slot1S = conf->addElement(ElementPtr(new Slot("slot1")));
    ElementSpecPtr slot2S = conf->addElement(ElementPtr(new Slot("slot2")));
    ElementSpecPtr printS = conf->addElement(ElementPtr(new Print("Printer")));

    conf->hookUp(sourceS, 0, slot1S, 0);
    conf->hookUp(slot1S, 0, printS, 0);
    conf->hookUp(printS, 0, slot2S, 0);
    conf->hookUp(slot2S, 0, sinkS, 0);
  
    BOOST_CHECK_MESSAGE(plumber->install(conf) != 0,
                        "Failed to catch incorrect pull-push hookup via a/a element");
  }


  /** Push to pull, multi hop. */
  void testCheckPushPull_PullToPushMultiHop()
  {
    PlumberPtr plumber(new Plumber());
    Plumber::DataflowPtr conf(new Plumber::Dataflow("testCheckPushPull_PullToPushMultiHop"));
    ElementSpecPtr sourceS = conf->addElement(ElementPtr(new TimedPushSource("source", 0)));
    ElementSpecPtr sinkS = conf->addElement(ElementPtr(new TimedPullSink("sink", 0)));
    ElementSpecPtr slot1S = conf->addElement(ElementPtr(new Slot("slot1")));
    ElementSpecPtr slot2S = conf->addElement(ElementPtr(new Slot("slot2")));
    ElementSpecPtr printS = conf->addElement(ElementPtr(new Print("Printer")));
    ElementSpecPtr print2S = conf->addElement(ElementPtr(new Print("Printer Two")));

    conf->hookUp(sourceS, 0, slot1S, 0);
    conf->hookUp(slot1S, 0, printS, 0);
    conf->hookUp(printS, 0, print2S, 0);
    conf->hookUp(print2S, 0, slot2S, 0);
    conf->hookUp(slot2S, 0, sinkS, 0);
  
    BOOST_CHECK_MESSAGE(plumber->install(conf) != 0,
                        "Failed to catch incorrect pull-push hookup via multiple a/a elements");
  }


  /** Pull to pull multi hop (correct). */
  void testCheckPushPull_PullToPullMultiHop()
  {
    PlumberPtr plumber(new Plumber());
    Plumber::DataflowPtr conf(new Plumber::Dataflow("testCheckPushPull_PullToPullMultihop"));
    ElementSpecPtr sourceS = conf->addElement(ElementPtr(new TimedPushSource("source", 0)));
    ElementSpecPtr sinkS = conf->addElement(ElementPtr(new TimedPullSink("sink", 0)));
    ElementSpecPtr slot1S = conf->addElement(ElementPtr(new Slot("slot")));
    ElementSpecPtr printS = conf->addElement(ElementPtr(new Print("Printer")));
    ElementSpecPtr print2S = conf->addElement(ElementPtr(new Print("Printer Two")));

    conf->hookUp(sourceS, 0, slot1S, 0);
    conf->hookUp(slot1S, 0, printS, 0);
    conf->hookUp(printS, 0, print2S, 0);
    conf->hookUp(print2S, 0, sinkS, 0);
  
    BOOST_CHECK_MESSAGE(plumber->install(conf) == 0,
                        "Caught incorrectly a pull-pull hookup via multiple a/a elements");
  }
















  ////////////////////////////////////////////////////////////
  // Check Duplicates
  ////////////////////////////////////////////////////////////

  /** Unused port */
  void testDuplicates_UnusedPort()
  {
    PlumberPtr plumber(new Plumber());
    Plumber::DataflowPtr conf(new Plumber::Dataflow("testDuplicates_UnusedPort"));
    ElementSpecPtr sourceS =
      conf->addElement(ElementPtr(new TimedPushSource("source", 0)));
    ElementSpecPtr sinkS =
      conf->addElement(ElementPtr(new TimedPullSink("sink", 0)));
    ElementSpecPtr slot1S = conf->addElement(ElementPtr(new Slot("slot")));
    ElementSpecPtr printS = conf->addElement(ElementPtr(new Print("Printer")));
    ElementSpecPtr print2S = conf->addElement(ElementPtr(new Print("Printer Two")));

    conf->hookUp(sourceS, 0, slot1S, 0);
    conf->hookUp(slot1S, 0, print2S, 0);
    conf->hookUp(print2S, 0, sinkS, 0);
  
    BOOST_CHECK_MESSAGE(plumber->install(conf) < 0,
                        "Incorrectly allowed unused port of an element");
  }


  /** Reused port */
  void testDuplicates_ReusedPort()
  {
    PlumberPtr plumber(new Plumber());
    Plumber::DataflowPtr conf(new Plumber::Dataflow("testDuplicates_ReusedPort"));
    ElementSpecPtr sourceS =
      conf->addElement(ElementPtr(new TimedPushSource("source", 0)));
    ElementSpecPtr sinkS =
      conf->addElement(ElementPtr(new TimedPullSink("sink", 0)));
    ElementSpecPtr slot1S = conf->addElement(ElementPtr(new Slot("slot")));
    ElementSpecPtr printS = conf->addElement(ElementPtr(new Print("Printer")));
    ElementSpecPtr print2S = conf->addElement(ElementPtr(new Print("Printer Two")));

    conf->hookUp(sourceS, 0, slot1S, 0);
    conf->hookUp(slot1S, 0, print2S, 0);
    conf->hookUp(print2S, 0, slot1S, 0);
    conf->hookUp(slot1S, 0, sinkS, 0);
  
    BOOST_CHECK_MESSAGE(plumber->install(conf) != 0,
                        "Incorrectly allowed port reuse");
  }







};







class testBasicElementPlumbing_testSuite
  : public boost::unit_test_framework::test_suite
{
public:
  testBasicElementPlumbing_testSuite()
    : boost::unit_test_framework::test_suite("testBasicElementPlumbing: Marshaling/Unmarshaling")
  {
    boost::shared_ptr<testBasicElementPlumbing> instance(new testBasicElementPlumbing());
    
    
    add(BOOST_CLASS_TEST_CASE(&testBasicElementPlumbing::testCheckHookupElements_NonExistentToElement,
                              instance));

    add(BOOST_CLASS_TEST_CASE(&testBasicElementPlumbing::testCheckHookupElements_NonExistentFromElement,
                              instance));

    add(BOOST_CLASS_TEST_CASE(&testBasicElementPlumbing::testCheckHookupElements_NegativeFromPort,
                              instance));

    add(BOOST_CLASS_TEST_CASE(&testBasicElementPlumbing::testCheckHookupElements_NegativeToPort,
                              instance));

    add(BOOST_CLASS_TEST_CASE(&testBasicElementPlumbing::testCheckHookupRange_IncorrectFromPort,
                              instance));

    add(BOOST_CLASS_TEST_CASE(&testBasicElementPlumbing::testCheckHookupRange_IncorrectToPort,
                              instance));

    add(BOOST_CLASS_TEST_CASE(&testBasicElementPlumbing::testCheckHookupRange_IncorrectPorts,
                              instance));

    add(BOOST_CLASS_TEST_CASE(&testBasicElementPlumbing::testCheckHookupRange_Portless,
                              instance));

    add(BOOST_CLASS_TEST_CASE(&testBasicElementPlumbing::testCheckPushPull_PullToPush,
                              instance));

    add(BOOST_CLASS_TEST_CASE(&testBasicElementPlumbing::testCheckPushPull_PullToPushHop,
                              instance));

    add(BOOST_CLASS_TEST_CASE(&testBasicElementPlumbing::testCheckPushPull_PullToPushMultiHop,
                              instance));

    add(BOOST_CLASS_TEST_CASE(&testBasicElementPlumbing::testCheckPushPull_PullToPullMultiHop,
                              instance));

    add(BOOST_CLASS_TEST_CASE(&testBasicElementPlumbing::testDuplicates_UnusedPort,
                              instance));

    add(BOOST_CLASS_TEST_CASE(&testBasicElementPlumbing::testDuplicates_ReusedPort,
                              instance));
  }
};
