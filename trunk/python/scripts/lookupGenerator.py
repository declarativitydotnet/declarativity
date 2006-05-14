import dfparser
from libp2python import *
import sys
import os
import getopt
import random

DATAFLOW_NAME = "LookupGenerator"

class LookupGenerator(Element):
  def __init__(self, name, myaddress, nodes, freq):
      Element.__init__(self,name, 1, 1)
      self.mode      = "edit"
      self.myaddress = myaddress
      self.nodes     = nodes 
      self.freq      = freq
  def class_name(self): return "LookupGenerator"
  def processing(self): return "h/h"
  def flow_code(self):  return "-/-"
  def print_usage(self):
      print "===== Welcome to the P2 Chord LookupGenerator ====="
      print "The lookup generator will issue a lookup tuple destined for"
      print "a random identifier. This element can take a frequency at"
      print "which it is suppose to generator a new random lookup." 
      print "The generator sends the lookup to a set of chord nodes"
      print "specified by the command line argument. This set of nodes"
      print "represents the initial hop of the lookup as it traverses it's"
      print "way toward the final destination."
  def initialize(self): 
      self.print_usage()
      if not self.nodes or not self.freq:
          self.set_delay(0, self.delay_callback) 
      else:
          self.set_delay(0, self.issue_lookup) 
      return 0
  def callback(self):
      self.set_delay(self.freq, self.issue_lookup) 
  def issue_lookup(self):
      for node in nodes:
          tuple  = Tuple.mk()
          lookup = Tuple.mk()
          tuple.append(Val_Str.mk(node))

          lookup.append(Val_Str.mk("lookup"))
          lookup.append(Val_Str.mk(node))
          vec = IntVec()
          for i in range(5):
              vec.append(int(random.random()*sys.maxint)) 
          lookup.append(Val_ID.mk(vec))
          lookup.append(Val_Str.mk(self.myaddress))
          lookup.append(Val_Str.mk("lookupGenerator:%s:%d" % (node, random.random()*sys.maxint)))
          lookup.freeze()
          tuple.append(Val_Tuple.mk(lookup))
          tuple.freeze()
          if self.py_push(0, tuple, self.callback) == 0:
              return
      self.set_delay(self.freq, self.issue_lookup) 
  def delay_callback(self):
      # Read string from terminal and send it in a tuple
      line = raw_input("%s >> " % self.mode) 
      if   line[0:5] == "print":
          print "Random lookup sent to nodes: ", self.nodes
          print "A lookup is generated every %d seconds" % self.freq
      elif line[0:4] == "exit":
          sys.exit(0)
      elif line[0:5] == "clear":
          self.nodes = []
          self.freq  = 0
      elif line[0:4] == "run":
          return
      elif line == "." and self.mode != "edit": 
          self.mode = "edit" 
      elif self.mode == "edit" and line[0:4] == "node":
          self.mode = "node" 
      elif self.mode == "edit" and line[0:4] == "freq":
          self.mode = "freq" 
      elif self.mode == "node":
          self.nodes.append(line)
          print "Nodes entered: ", self.nodes
      elif self.mode == "freq":
          self.freq = int(line) 
      else:
          print "ERROR: unknown command or mode entered."
          self.print_usage()
      self.set_delay(0.5, self.delay_callback) 
  def push(self, port, tp, cb):
      # Received status of some sent tuple
      key    = tp.at(2).toString()
      nodeID = tp.at(3).toString()
      nodeIP = tp.at(4).toString()

      print "=== RECEIVED LOOKUP RESPONSE FOR KEY %s ===" % key
      print "\tNODE IP %s with KEY %s" % (nodeIP, nodeID)
      return 1
  

def print_usage():
    print
    print "Usage: lookupGenerator.py [-d] <local_ip_address> <local_port> [<freq> ",
    print "<node1_ip:node1_port> [node2_ip:node1_port ...]]\n"
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

def get_stub(port):
    stub = r"""dataflow %s {
      let udp = Udp2("udp", %s);

      TimedPushSource("dummy_source", 0)            ->
      Sequence("output", 1, 1)                      ->
      # Print("transport_out") ->
      Frag("fragment", 1)                           ->
      PelTransform("package", "$0 pop swallow pop") ->
      MarshalField("marshal", 1)                    ->
      StrToSockaddr("addr_conv", 0)                 -> 
      udp -> UnmarshalField("unmarshal", 1) ->
      PelTransform("unpackage", "$1 unboxPop") ->
      Defrag("defragment", 1) ->
      PelTransform("get_payload", "$2 unboxPop") ->
      # Print("transport_in") ->
      TimedPullPush("input", 0) ->
      Discard("dummy_discard");
      }
      .   # END OF DATAFLOW DEFINITION""" % (DATAFLOW_NAME, port)
    return stub

def gen_stub(plumber, port):
    stub = get_stub(port) 

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
  
    address  = args[0]
    port     = int(args[1])
    freq     = 0
    nodes    = []

    if len(args) >= 3:
        freq = int(args[2])
        for n in args[3:]:
            nodes.append(n)
  
    plumber = Plumber()
    stub    = gen_stub(plumber, port)
  
    if plumber.install(stub) != 0:
        print "** Stub Failed to initialize correct spec\n"
  
    edit   = plumber.new_dataflow_edit(DATAFLOW_NAME);
    input  = edit.find("input");
    output = edit.find("output");
  
    lookupGen = edit.addElement(LookupGenerator("lookupGenerator", address+":"+str(port), nodes, freq))
    edit.hookUp(input, 0, lookupGen, 0)
    edit.hookUp(lookupGen, 0, output, 0)
  
    if plumber.install(edit) != 0:
      print "Edit Correctly initialized.\n"

    # plumber.toDot("lookupGen.dot")
    # os.system("dot -Tps lookupGen.dot -o lookupGen.ps")
    # os.remove("lookupGen.dot")
    # Run the plumber
    eventLoop()
