import dfparser
import libp2python
import sys;
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

def stub_begin(name, port):
  stub = """
    dataflow """ + name + """ {
      let udp = Udp2("udp", """ + port + """);
  """
  return stub

def sub_end():
  stub = """
    }
    .	# END OF DATAFLOW DEFINITION
  """
  return stub

def stub_send(address, port):
  stub = """ 
    let wrapAroundDemux = Demux("wrapAroundSendDemux", {Val_Str(%s)}, 0);
    let unBoxWrapAround = UnboxField("unboxWrapAround", 1);

    DRoundRobin("dRoundRobin", 1) -> TimedPullPush("wrapPullPush", 0) ->
    wrapAroundDemux -> unBoxWrapAround;

    wrapAroundDemux[1] -> Queue("sendQueue", 1000) -> Print("print_remote_send") ->
    PelTransform("package", "$1 pop swallow pop") ->
    PelTransform("source_address", ""%s:%s" pop swallow unboxPop") ->
    Sequence("seq_number", 1) -> 
    PelTransform("package", "$2 pop swallow pop") ->
    MarshalField("marshal", 1) -> StrToSockaddr("addr_conv", 0) -> udp; """ % (address, address, port)
  return stub

# STUB SEND MUST BE CALLED FIRST!
def stub_receive():
  stub = """
    let wrapAroundMux = Mux("wrapAroundSendMux", 2)
    let ddemux        = DDemux("dDemux", 0)

    udp-> UnmarshalField("unmarshal", 1)         -> 
    PelTransform("unRoute", "$1 unboxPop")       ->
    PelTransform("unPackage", $3, unboxPop")     ->
    wrapAroundMux -> Queue("receiveQueue", 1000) -> 
    TimedPullPush("pullPush", 0) -> ddemux       -> 
    Discard("dummy_discard"); 
 
    wrapAroundDemux -> [1]wrapAroundMux;
  """
  return stub

def gen_stub(plumber, name, address, port):
    stub  = stub_begin(name, port)   + 
            stub_send(address, port) +
            stub_receive()           +
            stub_end()

    dfparser.compile(plumber, stub)

    if dfparser.dataflows.has_key(name):
      m = dfparser.dataflows[name]
      m.eval_dataflow()
      return m.conf

def intall_overlog(plumber, name):

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

  address = args[1]
  port    = int(args[0])

  plumber = libp2python.Plumber()
  stub    = gen_stub(plumber, "Main", address, port)

  print "INSTALL THE DATAFLOW"
  if plumber.install(stub) == 0:
    print "Stub Correctly initialized.\n"
  else:
    print "** Stub Failed to initialize correct spec\n"

  edit   = plumber.new_dataflow_edit("Main");
  ddemux = edit.find("dDemux");
  drr    = edit.find("dRoundRobin");

  oc = edit.addElement(libp2python.OverlogCompiler("ocompiler", "Main", 
                                                   address+":"+port)) 
  di = edit.addElement(libp2python.DataflowInstaller("dinstaller", plumber, 
                                                     dfparser)
  edit.hookUp(ddemux, 0, oc, 0)
  edit.hookUp(oc, 0, di, 0)
  edit.hookUp(di, 0, drr.element().add_input(), 0)

  print "INSTALL THE EDIT"
  if plumber.install(edit) == 0:
    print "Edit Correctly initialized.\n"
  else:
    print "** Edit Failed to initialize correct spec\n"

  # Run the plumber
  libp2python.eventLoop()
