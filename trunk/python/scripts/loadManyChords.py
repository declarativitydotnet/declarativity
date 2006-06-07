#!/usr/bin/python
import dfparser
from libp2python import *
import sys
import os
import getopt
import time
import random

DATAFLOW_NAME = "ChordLoader"
myaddress     = "localhost"
myport        = 9999

class ChordLoader(Element):
  def __init__(self, name):
      Element.__init__(self, name, 1, 1)
      self.myaddress = "%s:%d" % (myaddress, myport)
  def class_name(self): return DATAFLOW_NAME
  def processing(self): return "h/h"
  def flow_code(self):  return "-/-"
  def initialize(self): 
      self.set_delay(0, lambda: self.push_program(0)) 
      return 0
  def push_program(self, node):
      if node < self.nodes:
          print "\tprogram push to node %s:%d" % (self.address, self.port+node)
          if self.send(self.address+":"+str(self.port+node), node) == 0:
              return
          self.set_delay(flags["delay"], lambda: self.push_program(node+1)) 
      else:
          self.set_delay(5, lambda: sys.exit(0)) 
  def push(self, port, tp, cb):
      # Received status of some sent tuple
      nodeID = Val_Str.cast(tp.at(1))
      status = Val_Int32.cast(tp.at(2))
      mesg   = Val_Str.cast(tp.at(3))

      print "=== RECEIVED STATUS FROM NODE %s ===" % nodeID
      if status == 0: print "Overlog installation successful!"
      else: print "Overlog installation failure!"
      print mesg
      return 1
  def send(self, dest, node):
      id = "0x"
      for i in range(20):
          id += random.choice(["0","1","2","3","4","5","6","7","8","9","a","b","c","d","e","f"])
      id += "I"
      try:
          print "NODE %d ID: %s" % (node, id)
          if dest == flags["landmark"]:
              os.system("cpp -P -DLANDMARK=\\\"--\\\" -DIPADDRESS=\\\"%s\\\" -DNODEID=%s %s %s.processed" % \
                        (dest, id, self.input, self.input))
          else:
              os.system("cpp -P -DLANDMARK=\\\"%s\\\" -DIPADDRESS=\\\"%s\\\" -DNODEID=%s %s %s.processed" % \
                        (flags["landmark"], dest, id, self.input, self.input))
          file = open(self.input+".processed", 'r') 
          program = file.read()
          file.close()
          os.remove("%s.processed" % self.input)
          print "File %s text added to overlog program." % self.input
      except:
          print "ERROR: open file error on file", self.input
          sys.exit(1)
      tuple = Tuple.mk()
      payload = Tuple.mk()
      tuple.append(Val_Str.mk(dest))
      payload.append(Val_Str.mk("overlog"))
      payload.append(Val_Str.mk(dest))
      payload.append(Val_Str.mk(self.myaddress))
      payload.append(Val_Str.mk(program))
      payload.freeze()
      tuple.append(Val_Tuple.mk(payload))
      tuple.freeze()
      return self.py_push(0, tuple, lambda: self.push_program(node+1))
  

def print_usage():
    print r"""
Usage: loadManyChords.py [-d <sec_delay>=20] \
                         -f <input_file> -n <nodes> \
                         -a <ip_address> -p <start_port> -l <landmark>"""

def parse_cmdline(argv):
    shortopts = "d:f:n:a:p:l:"
    flags = {"delay" : 20, "input" : None, "nodes" : 0, "ip" : None, "port" : 0, "landmark" : None}
    opts, args = getopt.getopt(argv[1:], shortopts)
    for o, v in opts:
        if o   == "-f": flags["input"]    = v
        elif o == "-n": flags["nodes"]    = int(v)
        elif o == "-a": flags["ip"]       = v
        elif o == "-p": flags["port"]     = int(v)
        elif o == "-d": flags["delay"]    = int(v)
        elif o == "-l": flags["landmark"] = v 
        else:
            print_usage()
            sys.exit(3)
    return flags, args

def get_stub():
    stub = r"""dataflow %s {
      let udp = Udp2("udp", %s);

      TimedPushSource("dummy_source", 0)            ->
      Sequence("output", 1, 1)                      ->
      Frag("fragment", 1)                           ->
      PelTransform("package", "$0 pop swallow pop") ->
      MarshalField("marshal", 1)                    ->
      StrToSockaddr("addr_conv", 0)                 -> 
      udp -> UnmarshalField("unmarshal", 1) ->
      PelTransform("unpackage", "$1 unboxPop") ->
      Defrag("defragment", 1) ->
      PelTransform("get_payload", "$2 unboxPop") ->
      TimedPullPush("input", 0) ->
      Discard("dummy_discard");
      }
      .   # END OF DATAFLOW DEFINITION""" % (DATAFLOW_NAME, myport)
    return stub

def gen_stub(plumber):
    stub = get_stub() 

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
  
    eventLoopInitialize()
  
    plumber = Plumber()
    stub    = gen_stub(plumber)

    if plumber.install(stub) != 0:
        print "** Stub Failed to initialize correct spec\n"

    edit   = plumber.new_dataflow_edit(DATAFLOW_NAME);
    input  = edit.find("input");
    output = edit.find("output");

    term = edit.addElement(ChordLoader(DATAFLOW_NAME))
    edit.hookUp(input, 0, term, 0)
    edit.hookUp(term, 0, output, 0)

    if flags["input"] and flags["nodes"] and flags["ip"] and flags["port"] and flags["landmark"]:
        term.element().nodes   = flags["nodes"]
        term.element().address = flags["ip"]
        term.element().port    = flags["port"]
        term.element().input   = flags["input"]
    else:
        print_usage()
        sys.exit(0)

    if plumber.install(edit) == 0:
        print "Edit Correctly initialized.\n"

    # plumber.toDot("p2terminal.dot")
    # os.system("dot -Tps p2terminal.dot -o p2terminal.ps")
    # os.remove("p2terminal.dot")
    # Run the plumber
    eventLoop()
