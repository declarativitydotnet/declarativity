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

#include "p2.h"
#include "loop.h"
#include "udp.h"
#include "dDuplicateConservative.h"
#include "timedPullPush.h"
#include "queue.h"
#include "ol_lexer.h"
#include "ol_context.h"
#include "plmb_confgen.h"
#include "val_str.h"
#include <iostream>

#define DEBUG

P2::P2(string hostname, string port) 
  : _plumber(new Plumber()), _id(hostname+":"+port)
{
  eventLoopInitialize();
  Py_Initialize();
  boost::python::object 
    parser(boost::python::handle<> 
           (boost::python::borrowed(PyImport_ImportModule("dfparser"))));
  _parser = parser;
  install("dataflow", stub(hostname, port));
  _tupleSourceInterface = dynamic_cast<TupleSourceInterface*>( 
    _plumber->dataflow("P2")->find("tupleSourceInterface")->element().get());
}


int P2::install(string type, string program)
{
  if (type == string("overlog")) {
    ostringstream script;
    compileOverlog(program, script);
    program = script.str();
    type = "dataflow";
  }

  if (type == string("dataflow")) {
    _parser.attr("clear")();
    boost::python::tuple result =
      boost::python::extract<boost::python::tuple>(
        _parser.attr("compile")(_plumber, program));
    boost::python::dict dataflows = 
      boost::python::extract<boost::python::dict>(result[0]);
    boost::python::list edits     = 
      boost::python::extract<boost::python::list>(result[1]);

    int ndataflows = boost::python::extract<int>(dataflows.attr("__len__")());
    int nedits     = boost::python::extract<int>(edits.attr("__len__")());

    if (ndataflows) {
      boost::python::tuple t = dataflows.popitem();
      t[1].attr("eval_dataflow")();
      Plumber::DataflowPtr d = 
        boost::python::extract<Plumber::DataflowPtr>(t[1].attr("conf"));
      if (_plumber->install(d) < 0) {
        std::cerr << "DATAFLOW INSTALLATION FAILURE" << std::endl;
        return -1;
      }
    }
    if (nedits) {
      edits[0].attr("eval_dataflow")();
      Plumber::DataflowEditPtr e = 
        boost::python::extract<Plumber::DataflowEditPtr>(edits[0].attr("conf"));
      if (_plumber->install(e) < 0) {
        std::cerr << "EDIT INSTALLATION FAILURE" << std::endl;
        return -1;
      }
    }
#ifdef DEBUG
    _plumber->toDot("p2.dot");
#endif
  }
  return 0;
}

void P2::run()
{
  eventLoop();
}

P2::CallbackHandlePtr P2::subscribe(string tupleName, cb_tp callback)
{
  int port = -1;	// The port that the listener sits on
  // Create a new edit 
  Plumber::DataflowEditPtr edit = _plumber->new_dataflow_edit("P2");
  ElementSpecPtr listener = 
    edit->addElement(ElementPtr(new TupleListener("listener_"+tupleName, callback)));

  // The planner creates the duplicator name, see if one already exists 
  ElementSpecPtr duplicator = edit->find("dc_"+tupleName);
  if (!duplicator) {
    /** We want to listen for something that has no rule strand
     *  We first need to register it with the demux then hang a duplicator off of the demux
     *  A rule strand that is installed later, and assumes the tuple name is an event,
     *  will find this duplicator and install the strand properly. At this point this
     *  event can no longer be materialized */
    ElementSpecPtr ddemux   = edit->find("dDemux");
    ElementSpecPtr queue    = edit->addElement(
      ElementPtr(new Queue("demuxQueue_"+tupleName, 100)));
    ElementSpecPtr pullPush = edit->addElement(
      ElementPtr(new TimedPullPush("demuxTimedPullPush_"+tupleName, 0)));
    duplicator = edit->addElement(
      ElementPtr(new DDuplicateConservative("dc_"+tupleName, 0)));

    // Hook everything up except duplicator output
    edit->hookUp(ddemux, ddemux->add_output(Val_Str::mk(tupleName)), queue, 0);
    edit->hookUp(queue, 0, pullPush, 0);
    edit->hookUp(pullPush, 0, duplicator, 0);
  }
  // Not hookup the listener to the duplicator output
  port = duplicator->add_output();
  edit->hookUp(duplicator, port, listener, 0);
  if (_plumber->install(edit) < 0) {
    std::cerr << "ERROR: Couldn't install listener" << std::endl;
    return P2::CallbackHandlePtr();
  }
#ifdef DEBUG
  _plumber->toDot("p2.dot");
#endif
  // Return a handle to the caller for unsubscribing to the listener.
  return P2::CallbackHandlePtr(new P2::CallbackHandle(tupleName, port));	
}

void P2::unsubscribe(P2::CallbackHandlePtr handle)
{
  if (!handle || handle->_valid == false) {
    std::cerr << "ERROR: invalid callback handle." << std::endl;
  } 
  else {
    // Invalidate the handle
    handle->_valid = false;
  }
  unsigned port    = handle->_port;
  string tupleName = handle->_name;

  /* All this routine does it take out an edit, uses it to locate the
   * duplicator, and removes the port. */
  Plumber::DataflowEditPtr edit = _plumber->new_dataflow_edit("P2");
  ElementSpecPtr     duplicator = edit->find("dc_"+tupleName);
  if (!duplicator) {
    std::cerr << "ERROR: no listener registered for " << tupleName << std::endl;
    return;
  }
  edit->disconnect_output(duplicator, port);
  if (_plumber->install(edit) < 0) {
    std::cerr << "ERROR: uninstall listener" << std::endl;
    return;
  }

}

int P2::tuple(TuplePtr tp)
{
  /* Using the tupleSourceInterface, attached right before the demux, to 
   * inject a tuple into the system. */
  return _tupleSourceInterface->tuple(tp);
}

void P2::compileOverlog(string overlog, std::ostringstream& script) {
  std::istringstream overlog_iss(overlog, std::istringstream::in);
  boost::shared_ptr< OL_Context > ctxt(new OL_Context());
  ctxt->parse_stream(&overlog_iss);

  Plumber::DataflowEditPtr conf = _plumber->new_dataflow_edit("P2");
  Plmb_ConfGen *gen = new Plmb_ConfGen(ctxt.get(), conf, false, false, false, 
                                       string("overlogCompiler"), false, script, true);
  gen->createTables(_id);
  gen->configurePlumber(boost::shared_ptr<Udp>(), _id);
}

string P2::stub(string hostname, string port)
{
  ostringstream stub;
  stub << "dataflow P2 { \
      let udp = Udp2(\"udp\"," << port << "); \
      let wrapAroundDemux = Demux(\"wrapAroundSendDemux\", \
                                  {Val_Str(\""<<hostname<<":"<<port<< "\")}, 0); \
      let wrapAroundMux = Mux(\"wrapAroundSendMux\", 3); \
      udp-> UnmarshalField(\"unmarshal\", 1)      -> \
      PelTransform(\"unRoute\", \"$1 unboxPop\")    -> \
      Defrag(\"defragment\", 1)                   -> \
      TimedPullPush(\"demux_in_pullPush\", 0)     -> \
      PelTransform(\"unPackage\", \"$2 unboxPop\")  -> \
      wrapAroundMux -> \
      DDemux(\"dDemux\", {value}, 0) ->  \
      Queue(\"install_result_queue\") -> \
      DRoundRobin(\"dRoundRobin\", 1) -> \
      TimedPullPush(\"rrout_pullPush\", 0) -> \
      wrapAroundDemux -> \
      UnboxField(\"unboxWrapAround\", 1) -> \
      [1]wrapAroundMux;\
      wrapAroundDemux[1] -> \
      Sequence(\"terminal_sequence\", 1, 1)          -> \
      Frag(\"fragment\", 1)                          -> \
      PelTransform(\"package\", \"$0 pop swallow pop\") -> \
      MarshalField(\"marshalField\", 1)              -> \
      StrToSockaddr(\"addr_conv\", 0)                -> \
      udp;  \
      TupleSourceInterface(\"tupleSourceInterface\") -> Queue(\"injectQueue\",1000)-> \
      TimedPullPush(\"injectPullPush\", 0) -> [2]wrapAroundMux; \
    } \
    .	# END OF DATAFLOW DEFINITION"; 
  return stub.str();
}
