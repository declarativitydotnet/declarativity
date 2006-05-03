import dfparser
import libp2python
import sys;
import os;
import getopt;

def print_usage():
    print
    print "Usage: p2stub.py -d <address> <port>\n"
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

def get_test_stub(name, address, port):
  stub = """ 
    dataflow %s {
      let udp = Udp2("udp", %s);

      udp -> UnmarshalField("unmarshal", 1) ->
      PelTransform("unRoute", "$1 unboxPop") ->
      Print("unRoute_tuple") ->
      PelTransform("unPackage", "swallow unbox drop pop pop pop pop pop")  ->
      Print("before_ddemux") ->
      DDemux("dDemux", {}, 1) ->
      Print("overlog_input") ->
      Queue("install_result_queue") ->
      DRoundRobin("dRoundRobin", 1) ->  
      Sequence -> 
      PelTransform("source_address", ""%s:%s" pop swallow unboxPop") ->
      PelTransform("package", "$2 pop swallow pop") ->
      MarshalField("marshal", 1) -> StrToSockaddr("addr_conv", 0) -> udp; 
    }
    .
  """ % (name, port, address, port)
  return stub

def get_stub(name, address, port):
  stub = """ 
    dataflow %s {
      let udp = Udp2("udp", %s);
      let wrapAroundDemux = Demux("wrapAroundSendDemux", {Val_Str(%s)}, 0);
      let unBoxWrapAround = UnboxField("unboxWrapAround", 1);

      DRoundRobin("dRoundRobin", 1) -> TimedPullPush("wrapPullPush", 0) ->
      wrapAroundDemux -> unBoxWrapAround;

      wrapAroundDemux[1] -> Queue("sendQueue", 1000) -> Print("print_remote_send") ->
      PelTransform("package", "$1 pop swallow pop") ->
      PelTransform("source_address", ""%s:%s" pop swallow unboxPop") ->
      Sequence -> 
      PelTransform("package", "$2 pop swallow pop") ->
      MarshalField("marshal", 1) -> StrToSockaddr("addr_conv", 0) -> udp; 

      # Generate the receiver side
      let wrapAroundMux = Mux("wrapAroundSendMux", 2)
      let ddemux        = DDemux("dDemux", 0)

      udp-> UnmarshalField("unmarshal", 1)         -> 
      PelTransform("unRoute", "$1 unboxPop")       ->
      PelTransform("unPackage", $3, unboxPop")     ->
      wrapAroundMux -> Queue("receiveQueue", 1000) -> 
      TimedPullPush("pullPush", 0) -> ddemux       -> 
      Discard("dummy_discard"); 
 
      wrapAroundDemux -> [1]wrapAroundMux;
    }
    .	# END OF DATAFLOW DEFINITION
  """ % (name, port, address, address, port)
  return stub

def gen_stub(plumber, name, address, port):
    stub  = get_test_stub(name, address, port)

    dfparser.compile(plumber, stub)

    if dfparser.dataflows.has_key(name):
      m = dfparser.dataflows[name]
      m.eval_dataflow()
      return m.conf

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

  libp2python.eventLoopInitialize()

  address = args[0]
  port    = int(args[1])

  plumber = libp2python.Plumber()
  stub    = gen_stub(plumber, "Main", address, port)

  print "INSTALL THE DATAFLOW"
  if plumber.install(stub) == 0:
    print "Stub Correctly initialized.\n"
  else:
    print "** Stub Failed to initialize correct spec\n"

  edit   = plumber.new_dataflow_edit("Main")
  input  = edit.find("overlog_input")
  output = edit.find("install_result_queue")

  oc = edit.addElement(libp2python.OverlogCompiler("ocompiler", "Main", address+":"+str(port))) 
  di = edit.addElement(libp2python.DataflowInstaller("dinstaller", plumber, dfparser))
  edit.hookUp(input, 0, oc, 0)
  edit.hookUp(oc, 0, di, 0)
  edit.hookUp(di, 0, output, 0)

  print "INSTALL THE EDIT"
  if plumber.install(edit) == 0:
    print "Edit Correctly initialized.\n"
  else:
    print "** Edit Failed to initialize correct spec\n"

  plumber.toDot("p2stub.dot")
  os.system("dot -Tps p2stub.dot -o p2stub.ps")
  os.remove("p2stub.dot")
  # Run the plumber
  libp2python.eventLoop()
