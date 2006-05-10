import dfparser
from libp2python import *
import sys
import os
import getopt

DATAFLOW_NAME = "Terminal"

class Terminal(Element):
  def __init__(self, name, myaddress):
      Element.__init__(self,name, 1, 1)
      self.mode      = "terminal"
      self.myaddress = myaddress
      self.address   = None 
      self.program_header = r"""
/** DESCRIPTION: P2 Overlog Program to edit Dataflow %s */
      """ % name
      self.program = self.program_header
  def class_name(self): return "Terminal"
  def processing(self): return "h/h"
  def flow_code(self):  return "-/-"
  def print_usage(self):
      print "===== Welcome to the P2 Overlog Terminal ====="
      print "This terminal is setup to edit the %s dataflow." % self.name()
      print "Create an overlog program by accumulating overlog program text"
      print "from within the \"overlog\" or \"input\" mode (described below)."
      print ""
      print "The terminal has the following modes:"
      print "terminal: Accepts commands or mode change commands."
      print "input:    Enter a filename to be read in and"
      print "          added to the overlog program."
      print "overlog:  Adds the typed text to the overlog program."
      print "address:  Enter the node address to which the overlog"
      print "          program is to be sent." 
      print "The default mode is the terminal mode. To exit a given"
      print "mode back into the terminal type a single \".\" in the prompt." 
      print "You can only enter a given node from the terminal mode."
      print ""
      print "The following commands can be issued in any mode."
      print "print: Print to stdout, the current overlog program."
      print "clear: Clear the current overlog program."
      print "send:  Sends the current overlog program to the last node address"
      print "       entered in the address mode." 
      print "exit:  Exit the terminal"
  def initialize(self): 
      self.print_usage()
      self.set_delay(0, self.delay_callback) 
      return 0
  def callback(self):
      self.set_delay(0, self.delay_callback) 
  def delay_callback(self):
      # Read string from terminal and send it in a tuple
      line = raw_input("%s >> " % self.mode) 
      if   line[0:5] == "print":
          print self.program
      elif line[0:5] == "clear":
          self.program = self.program_header
      elif line[0:4] == "send":
          if not self.address:
              print "ERROR: no address entered!!!"
              self.print_usage()
          else:
              t = Tuple.mk()
              t.append(Val_Str.mk(self.address))
              t.append(Val_Str.mk("overlog"))
              t.append(Val_Str.mk(self.myaddress))
              t.append(Val_Str.mk(self.name()))
              t.append(Val_Str.mk(self.program))
              t.freeze()
              if self.py_push(0, t, self.callback) > 0:
                  self.set_delay(1, self.delay_callback) 
              return
      elif line[0:4] == "exit":
          sys.exit(0)
      elif line == "." and self.mode != "terminal": 
          self.mode = "terminal" 
      elif self.mode == "terminal" and line[0:7] == "overlog":
          self.mode = "overlog" 
      elif self.mode == "terminal" and line[0:5] == "input":
          self.mode = "input" 
      elif self.mode == "terminal" and line[0:7] == "address":
          self.mode = "address" 
      elif self.mode == "input":
          try:
              file = open(line, 'r') 
              self.program += file.read()
              print "File %s text added to overlog program." % line
          except:
              print "ERROR: open file error on file", line
      elif self.mode == "address":
          self.address = line
          print "Address entered: ", self.address
      elif self.mode == "overlog":
          self.program += line + "\n" 
      else:
          print "ERROR: unknown command or mode entered."
          self.print_usage()
      self.set_delay(0.1, self.delay_callback) 
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
  

def print_usage():
    print
    print "Usage: terminal.py [-d] <dataflow_edit_name> <terminal_ip_address> <terminal_port>\n"
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

def get_stub(port):
    stub = r"""dataflow %s {
      let udp = Udp2("udp", %s);

      TimedPushSource("dummy_source", 0)            ->
      PelTransform("output", "$0 pop swallow pop")  ->
      Sequence("sequence", 1, 1)                    ->
      Frag("fragment", 1)                           ->
      PelTransform("package", "$0 pop swallow pop") ->
      MarshalField("marshal", 1)                    ->
      StrToSockaddr("addr_conv", 0)                 -> 
      udp -> Print("udp_in") -> UnmarshalField("unmarshal", 1) ->
      PelTransform("unpackage", "$1 unboxPop") ->
      Print("before_defrag") ->
      Defrag("defragment", 1) ->
      PelTransform("get_payload", "$2 unboxPop") ->
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
  
    dataflow = args[0]
    address  = args[1]
    port     = int(args[2])
  
    plumber = Plumber()
    stub    = gen_stub(plumber, port)
  
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
