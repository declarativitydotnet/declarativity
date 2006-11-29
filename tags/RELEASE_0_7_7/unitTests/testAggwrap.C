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
#include "aggwrap2.h"

#include "val_str.h"
#include "val_tuple.h"
#include "val_int32.h"
#include "val_uint32.h"
#include "val_uint64.h"
#include "val_id.h"
#include "val_null.h"
#include "ID.h"

#include "testAggwrap.h"
#include <boost/bind.hpp>
#include "vector"

#include "stdlib.h"

class testAggwrap
{
private:




public:
  testAggwrap();


  /** A dummy element that is used to replace the inner graph of
      Aggwraps */
  class InnerElement : public Element {
  private:
    /** My simple action callback */
    boost::function< TuplePtr (TuplePtr)> _function;



  public:

    /** Set up a callback whenever the simple_action is invoked */
    InnerElement(string instanceName,
                 boost::function< TuplePtr (TuplePtr)> function);

    
    /** Gimme a class name */
    const char *class_name() const { return "testAggwrap::InnerElement";};


    /** Override simple_action to use the callback */
    TuplePtr simple_action(TuplePtr p);
  };




  /** A dummy element that is used to receive output from Aggwraps. */
  class ReceiverElement : public Element {
  private:
    /** My simple action callback */
    boost::function< void (TuplePtr)> _function;



  public:

    /** Set up a callback whenever the simple_action is invoked */
    ReceiverElement(string instanceName,
                    boost::function< void (TuplePtr)> function);

    
    /** Gimme a class name */
    const char*
    class_name() const { return "testAggwrap::ReceiverElement";};


    /** Override push to use the callback */
    int
    push(int port, TuplePtr, b_cbv cb);
  };


  /** Simple aggwrap test. Put together an aggwrap that takes tuples of
      the form incoming(@A, B), listens for internal tuples of the form
      joined(@A, B, C), and outputs outgoing(@B, max<C>). */
  void
  simpleTest();


  /** Inner graph function for simpleTest */
  TuplePtr
  simpleTestFeeder(TuplePtr p);


  /** Receiver function for simpleTest. Handles aggregated tuples. */
  void
  simpleTestReceiver(TuplePtr p);


  /** State for simple test */
  int _simpleTestCounter;





  ////////////////////////////////////////////////////////////
  // Script-based tests
  ////////////////////////////////////////////////////////////

  void testScripts();
};




testAggwrap::testAggwrap()
  : _simpleTestCounter(0)
{
}





////////////////////////////////////////////////////////////
// Simple existence test
////////////////////////////////////////////////////////////

testAggwrap::InnerElement::
InnerElement(string instanceName,
             boost::function< TuplePtr (TuplePtr) > function)
  : Element(instanceName, 1, 1),
    _function(function)
{
}


TuplePtr
testAggwrap::InnerElement::
simple_action(TuplePtr p)
{
  return _function(p);
}


testAggwrap::ReceiverElement::
ReceiverElement(string instanceName,
                boost::function< void (TuplePtr) > function)
  : Element(instanceName, 1, 0),
    _function(function)
{
}


int
testAggwrap::ReceiverElement::push(int port, TuplePtr t, b_cbv cb)
{
  BOOST_CHECK_MESSAGE(port == 0,
                      "testAggwrap::ReceiverElement received a push "
                      << "at a port different from port 0");

  // Now handle the result
  _function(t);
  return 1;
}


TuplePtr
testAggwrap::simpleTestFeeder(TuplePtr p)
{
  TuplePtr joined = Tuple::mk();
  joined->append(Val_Str::mk("joined"));
  joined->append((*p)[1]);
  joined->append((*p)[2]);
  joined->append(Val_Int32::mk(_simpleTestCounter++));
  joined->freeze();
  return joined;
}


void
testAggwrap::simpleTestReceiver(TuplePtr p)
{
  // The output should be exactly 0
  BOOST_CHECK_MESSAGE(p->size() == 4, "Expected tuple of size 4 " 
                      << "but got tuple of size "
                      << p->size());
  if (p->size() >= 4) {
    BOOST_CHECK_MESSAGE((*p)[3]->compareTo(Val_Int32::mk(0)) == 0,
                        "Expected aggregate value 0 " 
                        << "but got instead "
                        << (*p)[3]->toString());
  }
}


/** Will build an inner graph that for every push produces the inner
    tuple on the inner input of the Aggwrap */
void
testAggwrap::simpleTest()
{
  Aggwrap2 aggwrap("TestedAggwrap",
                  "MAX",
                   3,           // C from joined(@, B, C)
                   "joined");   
  testAggwrap::InnerElement inner("Inner",
                                  boost::bind(&testAggwrap::simpleTestFeeder,
                                              this,
                                              _1));
  testAggwrap::ReceiverElement receiver("Receiver",
                                        boost::bind(&testAggwrap::
                                                    simpleTestReceiver,
                                                    this,
                                                    _1));
  /** aggwrap[1]->[0]inner */
  aggwrap.connect_output(1, &inner, 0);
  inner.connect_input(0, &aggwrap, 1);


  /** inner[0]->aggwrap[1] */
  inner.connect_output(0, &aggwrap, 1);
  aggwrap.connect_input(1, &inner, 0);
  _simpleTestCounter = 0;

  /** aggwrap[0]->[0]consumer */
  receiver.connect_input(0, &aggwrap, 0);
  aggwrap.connect_output(0, &receiver, 0);

  
  /** Fetch the completion callback */
  b_cbv completionCallback =
    aggwrap.get_comp_cb();

  /** Set up the two group-bys */
  aggwrap.registerGroupbyField(1);
  aggwrap.registerGroupbyField(2);
  
  
  TuplePtr incoming = Tuple::mk();
  incoming->append(Val_Str::mk("incoming"));
  incoming->append(Val_Str::mk("A"));
  incoming->append(Val_Str::mk("B"));
  incoming->freeze();

  /** Send the "event" tuple */
  aggwrap.push(0, incoming, NULL);

  /** Notify aggwrap that "join" is done */
  completionCallback();

  /** Receive result */
}





































////////////////////////////////////////////////////////////
// Script-based test harness
////////////////////////////////////////////////////////////

/** Superclass of script-based tests.
 *
 * The script language accepts the following commands.
 *
 * e<tuple> pushes a tuple into the external input
 *
 * i<tuple> pushes a tuple into the internal input
 *
 * o<tuple> expects a tuple from the external output
 *
 * j<number> calls the give number of join completion callback
 *
 * a<name, aggField, joins, outerGroupByField1, outerGroupByField2, ...,
 * innerGroupByField1, innerGroupByField2, ...> creates an aggwrap with
 * the named aggregate function on the aggField field, the given number
 * of internal joins, and the field list as the outer/inner group-by
 * field sets. This command must precede all e,i,o,j commands.
 *
 * */
class AggwrapTest {
public:
  /** Create and execute a test given the script */
  AggwrapTest(std::string script,
              int line);


  /** The actual script */
  std::string _script;


  /** What line of the source was this test add (used for reporting) */
  int _line;
};


AggwrapTest::AggwrapTest(std::string script,
                         int line)
  : _script(script),
    _line(line)
{
}




/**
 * This object tracks a single interactive test for a single script. It
 * creates an Aggwrap element, plugs into its internal ports a dummy
 * element that produces tuples according to the script, and plugs into
 * its external output port a dummy element that listens for output.  If
 * the script is executed without mismatch, the test is a success.
 * Otherwise, an error is generated and the rest of the test is aborted.
 */
class AggwrapTracker {
public:
  /** Create the new tracker */
  AggwrapTracker(AggwrapTest &);


  /** Destroy a tracker */
  ~AggwrapTracker();


  /** Process an array of tests */
  static void
  process(std::vector< AggwrapTest >);


  /** A dummy element that is used to replace the inner graph of
      Aggwraps.  Its input is a no-op and takes all that's pushed ot it.
      Its output is invoked whenever the appropriate script command is
      encountered. */
  class InnerElement : public Element {
  public:
    InnerElement(string instanceName);


    /** Gimme a class name */
    const char *class_name() const { return "AggwrapTracker::InnerElement";};


    /** Override simple_action to ignore inputs. */
    TuplePtr simple_action(TuplePtr p);
  };


  /** A dummy element that is used to take the output of an aggwrap */
  class ReceiverElement : public Element {
  private:
    /** My tracker */
    AggwrapTracker* _myTracker;


  public:
    ReceiverElement(string instanceName,
                    AggwrapTracker* myTracker);

    /** Gimme a class name */
    const char *class_name() const { return "AggwrapTracker::ReceiverElement";};


    /** Override push to forward to the tracker's listener */
    int
    push(int port, TuplePtr, b_cbv cb);
  };
protected:
  /** My test */
  AggwrapTest& _test;


  /** My aggwrap element */
  Aggwrap2* _aggwrap;


  /** My dummy inner element. It does nothing with its inputs and pushes
      an output whenever the appropriate script command is found. */
  InnerElement* _inner;


  /** My dummy receiver element. It just takes the eventual output of
      the aggregation */
  ReceiverElement* _receiver;


  /** My join callbacks */
  std::vector< b_cbv > _joinCallbacks;
  

  /** The remainder of my script */
  std::string _remainder;


  /** Advance the script by a command. Returns false if the tracking is
      aborted, true otherwise. */
  bool
  fetchCommand();


  /** Listen for external output by aggwrap */
  void
  listener(TuplePtr t);


  /** The current command type */
  char _command;


  /** The current tuple */
  TuplePtr _tuple;


  /** Execute the tracked script */
  void
  test();
};



AggwrapTracker::AggwrapTracker(AggwrapTest & test)
  : _test(test),
    _aggwrap(NULL)
{
}


AggwrapTracker::~AggwrapTracker()
{
  if (_aggwrap) {
    delete _aggwrap;
    _aggwrap = NULL;
    delete _inner;
    _inner = NULL;
    delete _receiver;
    _receiver= NULL;
  }
}

AggwrapTracker::InnerElement::
InnerElement(string instanceName)
  : Element(instanceName, 1, 1)
{
}


TuplePtr
AggwrapTracker::InnerElement::
simple_action(TuplePtr p)
{
  return TuplePtr();
}


AggwrapTracker::ReceiverElement::
ReceiverElement(string instanceName,
                AggwrapTracker* myTracker)
  : Element(instanceName, 1, 0),
    _myTracker(myTracker)
{
}


int
AggwrapTracker::ReceiverElement::push(int port, TuplePtr t, b_cbv cb)
{
  BOOST_CHECK_MESSAGE(port == 0,
                      "AggwrapTracker::ReceiverElement received a push "
                      << "at a port different from port 0");

  // Now handle the result as a listener
  _myTracker->listener(t);
  return 1;
}


void
AggwrapTracker::process(std::vector< AggwrapTest > tests)
{
  for (std::vector< AggwrapTest >::iterator i = tests.begin();
       i != tests.end();
       i++) {
    // Create a script tracker
    AggwrapTracker tracker(*i);

    // Run it
    tracker.test();
  }
}


/**
 * Execute the script.  First parse the script string.  Then execute it.
 * Parse errors abort the current test.
 */
void
AggwrapTracker::test()
{
  _remainder = std::string(_test._script);
  
  while (!_remainder.empty()) {
    bool result = fetchCommand();
    
    if (result) {
      switch(_command) {
      case 'a':
        // Create an aggwrap element and plug it together
        {
          BOOST_CHECK_MESSAGE(_tuple->size() >= 4,
                              "Script in line "
                              << _test._line
                              << " with suffix "
                              << "\""
                              << _remainder
                              << "\" contains an aggwrap "
                              << "creation command with too few params: "
                              << _tuple->toString()
                              << ".");
          if (_tuple->size() < 4) {
            break;
          }

          _aggwrap = new Aggwrap2("TestedAggwrap",
                                  Val_Str::cast((*_tuple)[0]),
                                  Val_UInt32::cast((*_tuple)[1]),
                                  Val_Str::cast((*_tuple)[3]));
          
          _inner = new InnerElement("Inner");

          _receiver = new ReceiverElement("Receiver",
                                          this);

          /** aggwrap[1]->[0]inner */
          _aggwrap->connect_output(1, _inner, 0);
          _inner->connect_input(0, _aggwrap, 1);
          
          
          /** inner[0]->aggwrap[1] */
          _inner->connect_output(0, _aggwrap, 1);
          _aggwrap->connect_input(1, _inner, 0);
          
          /** aggwrap[0]->[0]consumer */
          _receiver->connect_input(0, _aggwrap, 0);
          _aggwrap->connect_output(0, _receiver, 0);


          // Set up the join callbacks
          int joins = Val_Int32::cast((*_tuple)[2]);
          for (int i = 0;
               i < joins;
               i++) {
            _joinCallbacks.push_back(_aggwrap->get_comp_cb());
          }


          // Set up group-by fields
          for (uint i = 4;
               i < _tuple->size();
               i++) {
            _aggwrap->
              registerGroupbyField(Val_UInt32::cast((*_tuple)[i]));
          }
        }
        break;
        
      case 'e':
        // Push a tuple from the outside
        _aggwrap->push(0, _tuple, NULL);
        break;

      case 'i':
        // Push a tuple from the inside
        _aggwrap->push(1, _tuple, NULL);
        break;

      case 'j':
        // Invoke the given join callback
        {
          int joinNum = Val_Int32::cast((*_tuple)[0]);
          b_cbv joinCallback = _joinCallbacks.at(joinNum);
          joinCallback();
        }
        break;

      case 'o':
        // Expect tuple in the output. This should not be found here
        BOOST_ERROR("Error in test line "
                    << _test._line
                    << " with suffix "
                    << "\""
                    << _remainder
                    << "\". Expected output '"
                    << _tuple->toString()
                    << "' but didn't get it.");
        break;

      default:
        // I should not receive anything else
        BOOST_ERROR("Error in test line "
                    << _test._line
                    << " with suffix "
                    << "\""
                    << _remainder
                    << "\". Unknown command '"
                    << _command
                    << "'.");
        return;
      }
    } else {
      return;
    }
  }
}



void
AggwrapTracker::listener(TuplePtr t)
{
  // Fetch a command
  if (!_remainder.empty()) {
    bool result = fetchCommand();

    if (result) {
      // Is it an output expectation?
      if (_command != 'o') {
        // I should have an output expectation in there
        BOOST_CHECK_MESSAGE(false,
                            "Script in line "
                            << _test._line
                            << " with suffix "
                            << "\""
                            << _remainder
                            << "\" obtained unexpected output tuple '"
                            << t->toString()
                            << "'.");
        _remainder = std::string();
        return;
      }
      
      // Compare the tuples
      BOOST_CHECK_MESSAGE(t->compareTo(_tuple) == 0,
                          "Error in test line "
                          << _test._line
                          << " with suffix "
                          << "\""
                          << _remainder
                          << "\". Script expected tuple '"
                          << _tuple->toString()
                          << "' but received '"
                          << t->toString()
                          << "' instead.");
      
      _remainder = std::string();
    } else {
      BOOST_CHECK_MESSAGE(false,
                          "Error in test line "
                          << _test._line
                          << " with suffix "
                          << "\""
                          << _remainder
                          << "\". Script seems to have "
                          << "a syntax error.");
      
      // Something went wrong with script parsing.
      _remainder = std::string();
    }
  } else {
    // The script appears to have ended, not expecting my output.
    BOOST_ERROR("Error in test line "
                << _test._line
                << " with suffix "
                << "\""
                << _remainder
                << "\". Script should expect an output with '"
                << t->toString()
                << "' but didn't.");
    _remainder = std::string();
  }
}



/**
 * Fetch another command from the beginning of the _remainder script.
 * The command is placed in the _command field.  The tuple argument of
 * the command is placed in the _tuple field.  The _remainder string is
 * shrunk by the removed command.
 *
 * This assumes that _remainder is not empty.
 *
 * If the fetch was successful, returns true. False if a syntax error in
 * the test specification was identified.
 */
bool
AggwrapTracker::fetchCommand()
{
  // This will require a new tuple
  _tuple = Tuple::mk();

  // Fetch another token
  std::string::size_type tokenEnd = _remainder.find(';');
  if (tokenEnd == std::string::npos) {
    // Syntax error. I should always end with a semicolon
    BOOST_ERROR("Syntax error in test line "
                << _test._line
                << " with suffix "
                << "\""
                << _remainder
                << "\". Missing a trailing semicolon.");
    return false;
  }
  
  // Take out token, leaving semicolon behind
  std::string token = _remainder.substr(0, tokenEnd);
  
  // Check command
  _command = token[0];
  switch(_command) {
  case 'i':
  case 'e':
  case 'o':
  case 'j':
  case 'a':
    break;
  default:
    BOOST_ERROR("Syntax error in test line "
                << _test._line
                << " with token "
                << "\""
                << token
                << "\". Unknown command '"
                << _command
                << "'.");
    return false;
  }
  
  // Check tuple containers
  if ((token[1] != '<') ||
      (token[token.length() - 1] != '>')) {
    BOOST_ERROR("Syntax error in test line "
                << _test._line
                << " with token "
                << "\""
                << token
                << "\". Tuple must be enclosed in '<>'.");
    return false;
  }
  
  // Parse and construct tuple
  std::string tuple = token.substr(2, token.length() - 3);
  
  while (!tuple.empty()) {
    // Fetch another field delimiter
    std::string::size_type fieldEnd = tuple.find(',');
    
    // Isolate the field
    std::string field;
    if (fieldEnd == std::string::npos) {
      // The rest of the tuple makes up a field
      field = tuple;
      tuple = std::string();
    } else {
      // Take a field up to the delimiter
      field = tuple.substr(0, fieldEnd);
      tuple = tuple.substr(fieldEnd + 1);
    }
    
    // Interpret the field. If it starts with a quote, it's a string,
    // otherwise an integer.
    if (field[0] == '"') {
      // This is a string
      BOOST_CHECK_MESSAGE(field[field.size() - 1] == '"',
                          "Syntax error in test line "
                          << _test._line
                          << " with token "
                          << "\""
                          << token
                          << "\". No matching end quote in field "
                          << field
                          << ".");
      ValuePtr strField = Val_Str::mk(field.substr(1, field.size() - 2));
      _tuple->append(strField);
    } else if (field == "null") { // is this the null tuple?
      _tuple->append(Val_Null::mk());
    } else {
      // Identify any type information.
      ValuePtr intField;
      switch (field[field.length() - 1]) {
      case 'U':
        // This should be unsigned 64bit
        intField = Val_UInt64::mk(Val_UInt64::cast(Val_Str::mk(field)));
        break;
      case 'u':
        // This should be unsigned 32bit
        intField = Val_UInt32::mk(Val_UInt32::cast(Val_Str::mk(field)));
        break;
      default:
        // Interpret as signed 32bit
        intField = Val_Int32::mk(Val_Int32::cast(Val_Str::mk(field)));
        break;
      }
      _tuple->append(intField);
    }
  }
  
  // Shrink remainder
  _remainder = _remainder.substr(tokenEnd + 1);

  // Freeze the tuple
  _tuple->freeze();

  return true;
}


void
testAggwrap::testScripts()
{
  AggwrapTest t[] =
    {
      AggwrapTest("a<\"MAX\",2,1,\"inner\",2,1>;"
                  "e<\"input\",6,1>;"
                  "j<0>;"
                  "o<\"inner\",1,null,6>;",
                  __LINE__),
      
      // No group-by aggregations with non-empty result sets
      AggwrapTest("a<\"MIN\",1,1,\"inner\">;" // aggregate on 1st join field, 1
                                // join total, no group-by
                  "e<\"input\",8>;"
                  "i<\"inner\",5>;"
                  "i<\"inner\",7>;"
                  "i<\"inner\",1>;"
                  "j<0>;"
                  "o<\"inner\",1>;",
                  __LINE__),
      
      AggwrapTest("a<\"MAX\",1,1,\"inner\">;" // aggregate on 1st join field, 1
                                      // join total, no group-by
                  "e<\"input\",8>;"
                  "i<\"inner\",5>;"
                  "i<\"inner\",7>;"
                  "i<\"inner\",1>;"
                  "j<0>;"
                  "o<\"inner\",7>;",
                  __LINE__),
      
      AggwrapTest("a<\"COUNT\",1,1,\"inner\">;" // aggregate on 1st join field, 1
                                      // join total, no group-by
                  "e<\"input\",8>;"
                  "i<\"inner\",5>;"
                  "i<\"inner\",7>;"
                  "i<\"inner\",1>;"
                  "j<0>;"
                  "o<\"inner\",3U>;",
                  __LINE__),
      
      // No group-by aggregations with empty result sets
      AggwrapTest("a<\"MIN\",1,1,\"inner\">;" // aggregate on 1st join field, 1
                                      // join total, no group-by
                  "e<\"input\",8>;"
                  "j<0>;"
                  "o<\"inner\",null>;",
                  __LINE__),
      
      AggwrapTest("a<\"MAX\",1,1,\"inner\">;" // aggregate on 1st join field, 1
                                      // join total, no group-by
                  "e<\"input\",8>;"
                  "j<0>;"
                  "o<\"inner\",null>;",
                  __LINE__),
      
      AggwrapTest("a<\"COUNT\",1,1,\"inner\">;" // aggregate on 1st join field, 1
                                      // join total, no group-by
                  "e<\"input\",8>;"
                  "j<0>;"
                  "o<\"inner\",0U>;",
                  __LINE__),
      

      // Group-by aggregations with non-empty result sets, aggregate
      // first on result
      AggwrapTest("a<\"MIN\",1,1,\"inner\",2,1>;"
                  "e<\"input\",6,1>;"
                  "i<\"inner\",5,1,6>;"
                  "i<\"inner\",7,1,6>;"
                  "i<\"inner\",1,1,6>;"
                  "j<0>;"
                  "o<\"inner\",1,1,6>;",
                  __LINE__),
      
      AggwrapTest("a<\"MAX\",1,1,\"inner\",2,1>;"
                  "e<\"input\",6,1>;"
                  "i<\"inner\",5,1,6>;"
                  "i<\"inner\",7,1,6>;"
                  "i<\"inner\",1,1,6>;"
                  "j<0>;"
                  "o<\"inner\",7,1,6>;",
                  __LINE__),
      
      AggwrapTest("a<\"COUNT\",1,1,\"inner\",2,1>;"
                  "e<\"input\",6,1>;"
                  "i<\"inner\",5,1,6>;"
                  "i<\"inner\",7,1,6>;"
                  "i<\"inner\",1,1,6>;"
                  "j<0>;"
                  "o<\"inner\",3U,1,6>;",
                  __LINE__),
      
      // Group-by aggregations with empty result sets, aggregate first
      // on result
      AggwrapTest("a<\"MIN\",1,1,\"inner\",2,1>;"
                  "e<\"input\",6,1>;"
                  "j<0>;"
                  "o<\"inner\",null,1,6>;",
                  __LINE__),
      
      AggwrapTest("a<\"MAX\",1,1,\"inner\",2,1>;"
                  "e<\"input\",6,1>;"
                  "j<0>;"
                  "o<\"inner\",null,1,6>;",
                  __LINE__),
      
      AggwrapTest("a<\"COUNT\",1,1,\"inner\",2,1>;"
                  "e<\"input\",6,1>;"
                  "j<0>;"
                  "o<\"inner\",0U,1,6>;",
                  __LINE__),
      
      // Group-by aggregations with non-empty result sets, aggregate
      // middle on result
      AggwrapTest("a<\"MIN\",2,1,\"inner\",2,1>;"
                  "e<\"input\",6,1>;"
                  "i<\"inner\",1,5,6>;"
                  "i<\"inner\",1,7,6>;"
                  "i<\"inner\",1,1,6>;"
                  "j<0>;"
                  "o<\"inner\",1,1,6>;",
                  __LINE__),
      
      AggwrapTest("a<\"MAX\",2,1,\"inner\",2,1>;"
                  "e<\"input\",6,1>;"
                  "i<\"inner\",1,5,6>;"
                  "i<\"inner\",1,7,6>;"
                  "i<\"inner\",1,1,6>;"
                  "j<0>;"
                  "o<\"inner\",1,7,6>;",
                  __LINE__),
      
      AggwrapTest("a<\"COUNT\",2,1,\"inner\",2,1>;"
                  "e<\"input\",6,1>;"
                  "i<\"inner\",1,5,6>;"
                  "i<\"inner\",1,7,6>;"
                  "i<\"inner\",1,1,6>;"
                  "j<0>;"
                  "o<\"inner\",1,3U,6>;",
                  __LINE__),
      
      // Group-by aggregations with empty result sets, aggregate middle
      // on result
      AggwrapTest("a<\"MIN\",2,1,\"inner\",2,1>;"
                  "e<\"input\",6,1>;"
                  "j<0>;"
                  "o<\"inner\",1,null,6>;",
                  __LINE__),
      
      AggwrapTest("a<\"MAX\",2,1,\"inner\",2,1>;"
                  "e<\"input\",6,1>;"
                  "j<0>;"
                  "o<\"inner\",1,null,6>;",
                  __LINE__),
      
      AggwrapTest("a<\"COUNT\",2,1,\"inner\",2,1>;"
                  "e<\"input\",6,1>;"
                  "j<0>;"
                  "o<\"inner\",1,0U,6>;",
                  __LINE__),
      
      // Group-by aggregations with non-empty result sets, aggregate
      // last on result
      AggwrapTest("a<\"MIN\",3,1,\"inner\",2,1>;"
                  "e<\"input\",6,1>;"
                  "i<\"inner\",1,6,5>;"
                  "i<\"inner\",1,6,7>;"
                  "i<\"inner\",1,6,1>;"
                  "j<0>;"
                  "o<\"inner\",1,6,1>;",
                  __LINE__),
      
      AggwrapTest("a<\"MAX\",3,1,\"inner\",2,1>;"
                  "e<\"input\",6,1>;"
                  "i<\"inner\",1,6,5>;"
                  "i<\"inner\",1,6,7>;"
                  "i<\"inner\",1,6,1>;"
                  "j<0>;"
                  "o<\"inner\",1,6,7>;",
                  __LINE__),
      
      AggwrapTest("a<\"COUNT\",3,1,\"inner\",2,1>;"
                  "e<\"input\",6,1>;"
                  "i<\"inner\",1,6,5>;"
                  "i<\"inner\",1,6,7>;"
                  "i<\"inner\",1,6,1>;"
                  "j<0>;"
                  "o<\"inner\",1,6,3U>;",
                  __LINE__),
      
      // Group-by aggregations with empty result sets, aggregate middle
      // on result
      AggwrapTest("a<\"MIN\",3,1,\"inner\",2,1>;"
                  "e<\"input\",6,1>;"
                  "j<0>;"
                  "o<\"inner\",1,6,null>;",
                  __LINE__),
      
      AggwrapTest("a<\"MAX\",3,1,\"inner\",2,1>;"
                  "e<\"input\",6,1>;"
                  "j<0>;"
                  "o<\"inner\",1,6,null>;",
                  __LINE__),
      
      AggwrapTest("a<\"COUNT\",3,1,\"inner\",2,1>;"
                  "e<\"input\",6,1>;"
                  "j<0>;"
                  "o<\"inner\",1,6,0U>;",
                  __LINE__),
      
      // Group-by aggregations with empty result sets and multiple joins
      AggwrapTest("a<\"MIN\",3,3,\"inner\",2,1>;"
                  "e<\"input\",6,1>;"
                  "j<0>;"
                  "j<1>;"
                  "j<2>;"
                  "o<\"inner\",1,6,null>;",
                  __LINE__),
      
      AggwrapTest("a<\"MAX\",3,3,\"inner\",2,1>;"
                  "e<\"input\",6,1>;"
                  "j<0>;"
                  "j<1>;"
                  "j<2>;"
                  "o<\"inner\",1,6,null>;",
                  __LINE__),
      
      AggwrapTest("a<\"COUNT\",3,3,\"inner\",2,1>;"
                  "e<\"input\",6,1>;"
                  "j<0>;"
                  "j<1>;"
                  "j<2>;"
                  "o<\"inner\",1,6,0U>;",
                  __LINE__),
      
      // Group-by aggregations with non-empty result sets and
      // interspersed multiple joins
      AggwrapTest("a<\"MIN\",3,3,\"inner\",2,1>;"
                  "e<\"input\",6,1>;"
                  "i<\"inner\",1,6,5>;"
                  "i<\"inner\",1,6,7>;"
                  "i<\"inner\",1,6,1>;"
                  "j<0>;"
                  "j<1>;"
                  "j<2>;"
                  "o<\"inner\",1,6,1>;",
                  __LINE__),
      
      AggwrapTest("a<\"MAX\",3,3,\"inner\",2,1>;"
                  "e<\"input\",6,1>;"
                  "i<\"inner\",1,6,5>;"
                  "j<0>;"
                  "i<\"inner\",1,6,7>;"
                  "i<\"inner\",1,6,1>;"
                  "j<1>;"
                  "j<2>;"
                  "o<\"inner\",1,6,7>;",
                  __LINE__),
      
      AggwrapTest("a<\"COUNT\",3,3,\"inner\",2,1>;"
                  "e<\"input\",6,1>;"
                  "i<\"inner\",1,6,5>;"
                  "j<0>;"
                  "i<\"inner\",1,6,7>;"
                  "j<1>;"
                  "i<\"inner\",1,6,1>;"
                  "j<2>;"
                  "o<\"inner\",1,6,3U>;",
                  __LINE__),
      
    };
  std::vector< AggwrapTest > vec(t,
                                 t + sizeof(t)/sizeof(t[0]));
  
  AggwrapTracker::process(vec);
}






























testAggwrap_testSuite::testAggwrap_testSuite()
  : boost::unit_test_framework::test_suite("testAggwrap: Marshaling/Unmarshaling")
{
  boost::shared_ptr<testAggwrap> instance(new testAggwrap());


  add(BOOST_CLASS_TEST_CASE(&testAggwrap::testScripts, instance));
  add(BOOST_CLASS_TEST_CASE(&testAggwrap::simpleTest, instance));
}

