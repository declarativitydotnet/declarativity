import dfparser
from libp2python import *
import sys
import os
import getopt

DATAFLOW_NAME = "Terminal"

class Terminal(Element):
  def __init__(self, name, address):
      Element.__init__(self,name, 1, 1)
      self.address = address
  def class_name(self): return "Terminal"
  def processing(self): return "h/h"
  def flow_code(self):  return "-/-"
  def initialize(self): 
      self.set_delay(0, self.delay_callback) 
      return 0
  def callback(self):
      self.set_delay(0, self.delay_callback) 
  def delay_callback(self):
      # Read string from terminal and send it in a tuple
      program = r"""
/*
 * Copyright (c) 2005 Intel Corporation
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 *
 * DESCRIPTION: P2 Overlog Program to Dataflow %s
 */
""" % self.name()
      while True:
          line = raw_input("P2 Overlog Terminal >> ") 
          if line == ".": break
          if line[0:5] == "input":
             try:
                 file = open(line[6:], 'r') 
                 while l in file:
                     program += line + "\n"
             except:
                 print "ERROR: open file error on file", line[6:]
          else: program += line + "\n" 
      t = Tuple.mk()
      t.append(Val_Str.mk("overlog"))
      t.append(Val_Str.mk(address))
      t.append(Val_Str.mk(program))
      t.freeze()
      if self.py_push(0, t, self.callback) > 0:
        self.set_delay(1, self.delay_callback) 
  def push(self, port, tp, cb):
      # Received status of some sent tuple
      return 0
  

def print_usage():
    print
    print "Usage: terminal.py [-d] <dataflow_edit_name> <node_address> <port>\n"
    print

def parse_cmdline(argv):
    shortopts = "df:"
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
  
    dataflow = args[0]
    address  = args[1]
    port     = int(args[2])
  
    plumber = Plumber()
    stub    = gen_stub(plumber, address, port)
  
    if plumber.install(stub) != 0:
      print "** Stub Failed to initialize correct spec\n"
  
    edit   = plumber.new_dataflow_edit(DATAFLOW_NAME);
    input  = edit.find("input");
    output = edit.find("output");
  
    term = edit.addElement(Terminal(dataflow, address+":"+str(port)))
    edit.hookUp(input, 0, term, 0)
    edit.hookUp(term, 0, output, 0)
  
    if plumber.install(edit) != 0:
      print "Edit Correctly initialized.\n"

    # plumber.toDot("terminal.dot")
    # os.system("dot -Tps terminal.dot -o terminal.ps")
    # os.remove("terminal.dot")
    # Run the plumber
    eventLoop()
