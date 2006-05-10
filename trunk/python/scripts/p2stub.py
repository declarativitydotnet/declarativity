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

def get_stub(name, address, port):
  stub = """ 
    dataflow %s {
      let udp = Udp2("udp", %s);
      let wrapAroundDemux = Demux("wrapAroundSendDemux", {Val_Str("%s")}, 0);
      let wrapAroundMux = Mux("wrapAroundSendMux", 2);

      udp-> UnmarshalField("unmarshal", 1)      -> 
      PelTransform("unRoute", "$1 unboxPop")    ->
      Defrag("defragment", 1)                   -> 
      TimedPullPush("demux_in_pullPush", 0)     -> 
      PelTransform("unPackage", "$2 unboxPop")  ->
      wrapAroundMux ->
      DDemux("dDemux", {value}, 1) -> 
      Queue("install_result_queue") ->
      DRoundRobin("dRoundRobin", 1) ->  
      TimedPullPush("rr_out_pullPush", 0) -> 
      wrapAroundDemux -> 
      # UnboxField("unboxWrapAround", 1) -> 
      Print("wrap_around_print") -> [1]wrapAroundMux;

      wrapAroundDemux[1] -> 
      Print("transport_out") ->
      PelTransform("package_payload", "$0 pop swallow pop") ->
      Sequence("terminal_sequence", 1, 1)          ->
      Frag("fragment", 1)                          ->
      PelTransform("package", "$0 pop swallow pop") ->
      MarshalField("marshalField", 1)              ->
      StrToSockaddr("addr_conv", 0)                ->
      udp; 

    }
    .	# END OF DATAFLOW DEFINITION
  """ % (name, port, address)
  return stub

def gen_stub(plumber, name, address, port):
    stub  = get_stub(name, address, port)

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

  edit    = plumber.new_dataflow_edit("Main")
  # ddemux  = edit.find("dDemux")
  ddemux = edit.find("dDemux")
  rqueue = edit.find("install_result_queue")

  oc = edit.addElement(libp2python.OverlogCompiler("ocompiler", plumber, address+":"+str(port))) 
  di = edit.addElement(libp2python.DataflowInstaller("dinstaller", plumber, dfparser))
  edit.hookUp(ddemux, 0, oc, 0)
  edit.hookUp(oc, 0, di, 0)
  edit.hookUp(di, 0, rqueue, 0)

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
