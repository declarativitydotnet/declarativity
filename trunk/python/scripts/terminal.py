import dfparser
from libp2python import *
import sys
import os
import getopt

DATAFLOW_NAME = "Terminal"

class Terminal(Element):
  def __init__(self, name, address):
      Element.__init__(self,name, 1, 1)
      self.self(self)
      self.address = address
  def class_name(self): return "Terminal"
  def processing(self): return "h/h"
  def flow_code(self):  return "-/-"
  def initialize(self): 
      print "INITIALIZE CALLED"
      self.set_delay(0, "delay_callback") 
      return 0
  def callback(self, port):
      self.set_delay(0, "delay_callback") 
  def delay_callback(self):
      # Read string from terminal and send it in a tuple
      line = raw_input("P2 Terminal >> ") 
      t = Tuple.mk()
      t.append(Val_Str.mk("overlog"))
      t.append(Val_Str.mk(address))
      t.append(Val_Str.mk(line))
      t.freeze()
      if self.py_push(0, t, "callback") > 0:
        self.set_delay(1, "delay_callback") 
  def push(self, port, tp, cb):
      # Received status of some sent tuple
      return 0
  

def print_usage():
    print
    print "Usage: terminal.py -d <address> <port>\n"
    print

def parse_cmdline(argv):
    shortopts = "d"
    flags = {"debug" : False}
    opts, args = getopt.getopt(argv[1:], shortopts)
    for o, v in opts:
        if   o == "-d": flags["debug"]      = True
        else:
            print_usage()
            exit(3)
    return flags, args

def get_test_stub():
    stub = r"""dataflow %s {
      TimedPushSource("source", 0) -> Print("output") -> Discard("output_discard");
      TimedPushSource("input", 0) -> Discard("input_discard");
      }
      .   # END OF DATAFLOW DEFINITION""" % DATAFLOW_NAME
    return stub

def get_stub(address, port):
    stub = r"""dataflow %s {
      let udp = Udp2("udp", %s);

      TimedPushSource("dummy_source", 0) -> Queue("output") -> 
      PelTransform("box", "$1 pop swallow pop")      ->
      PelTransform("source_address", ""%s:%s" pop swallow unboxPop") ->
      Sequence("seq_number", 1) ->
      PelTransform("package", "$2 pop swallow pop") ->
      MarshalField("marshal", 1) -> StrToSockaddr("addr_conv", 0) -> 
      udp -> UnmarshalField("unmarshal", 1) -> PelTransform("unRoute", "$1 unboxPop") ->
      Print("input") -> Discard("dummy_discard");
      }
      .   # END OF DATAFLOW DEFINITION""" % (DATAFLOW_NAME, port, address, port)
    return stub

def gen_stub(plumber, address, port):
    stub = get_test_stub() 

    print "COMPILING: ", stub
    dfparser.compile(plumber, stub)

    if dfparser.dataflows.has_key(DATAFLOW_NAME):
      m = dfparser.dataflows[DATAFLOW_NAME]
      m.eval_dataflow()
      return m.conf
    print "DATAFLOW COMPILATION PROBLEM"
    return None

if __name__ == "__main__":
    try:
      flags, args = parse_cmdline(sys.argv)
    except:
      print "EXCEPTION"
      print_usage()
      sys.exit(3)
    if len(args) < 2:
      print_usage()
      sys.exit(3)
  
    eventLoopInitialize()
  
    address = args[0]
    port    = int(args[1])
  
    plumber = Plumber()
    stub    = gen_stub(plumber, address, port)
  
    print "INSTALL THE DATAFLOW"
    if plumber.install(stub) == 0:
      print "Stub Correctly initialized.\n"
    else:
      print "** Stub Failed to initialize correct spec\n"
  
    edit   = plumber.new_dataflow_edit(DATAFLOW_NAME);
    input  = edit.find("input");
    output = edit.find("output");
  
    term = edit.addElement(Terminal("terminal", address+":"+str(port)))
    edit.hookUp(input, 0, term, 0)
    edit.hookUp(term, 0, output, 0)
  
    print "INSTALL THE EDIT"
    if plumber.install(edit) == 0:
      print "Edit Correctly initialized.\n"
    else:
      print "** Edit Failed to initialize correct spec\n"

    # plumber.toDot("terminal.dot")
    # os.system("dot -Tps terminal.dot -o terminal.ps")
    # os.remove("terminal.dot")
    # Run the plumber
    eventLoop()
