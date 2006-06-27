import sys;
import getopt;

import libp2python;

plumber = libp2python.Plumber(libp2python.LoggerI.Level.WARN)

def print_usage():
    print
    print "Usage: netcc {(-s <port> <src_addr> <dest_addr:port>) | -d <port>} [<drop_probability>]\n"
    print

def parse_cmdline(argv):
    shortopts = "sd"
    flags = {"source" : False, "destination" : False}
    opts, args = getopt.getopt(argv[1:], shortopts)
    for o, v in opts:
        if   o == "-s": flags["source"]      = True
        elif o == "-d": flags["destination"] = True
        else:
            print_usage()
            exit(3)
    return flags, args


def UdpCC_source(udp, src, dest, drop):
  conf = libp2python.Plumber.Dataflow("source")

  udp      = conf.addElement(udp)
  data     = conf.addElement(libp2python.TimedPushSource("source", .01))
  dqueue   = conf.addElement(libp2python.Queue("Source Data Q", 1000))
  seq      = conf.addElement(libp2python.Sequence("Sequence", 1))
  srcAddr  = conf.addElement(libp2python.PelTransform("SRC: " + src, "\""+src+"\""+" pop swallow unboxPop"))
  destAddr = conf.addElement(libp2python.PelTransform("DEST: " + dest, "\""+dest+"\""+" pop swallow unboxPop"))
  retry    = conf.addElement(libp2python.RDelivery("Retry"))
  retryMux = conf.addElement(libp2python.Mux("Retry Mux", 2))
  cct      = conf.addElement(libp2python.CCT("Transmit CC", 1, 2048))
  box      = conf.addElement(libp2python.PelTransform("box", "$0 pop swallow pop"))
  marshal  = conf.addElement(libp2python.MarshalField("marshal data", 1))
  route    = conf.addElement(libp2python.StrToSockaddr("Convert dest addr", 0))
  netsim   = conf.addElement(libp2python.SimpleNetSim("Simple Net Sim (Sender)", 10, 100, drop))
  printer  = conf.addElement(libp2python.Print("Printer"))

  # The receiving data flow
  unmarshal = conf.addElement(libp2python.UnmarshalField("unmarshal ack", 1))
  unbox     = conf.addElement(libp2python.PelTransform("unRoute", "$1 unboxPop"))
  discard   = conf.addElement(libp2python.Discard("DISCARD"))

  # The local data flow
  conf.hookUp(data, 0, dqueue, 0)
  conf.hookUp(dqueue, 0, seq, 0)
  conf.hookUp(seq, 0, srcAddr, 0)
  conf.hookUp(srcAddr, 0, destAddr, 0)
  conf.hookUp(destAddr, 0, retry, 0)
  conf.hookUp(retry, 0, cct, 0)
  conf.hookUp(cct, 0, box, 0)
  conf.hookUp(box, 0, printer, 0)
  conf.hookUp(printer, 0, marshal, 0)
  conf.hookUp(marshal, 0, route, 0)
  conf.hookUp(route, 0, netsim, 0)
  conf.hookUp(netsim, 0, udp, 0)

  # The local ack flow
  conf.hookUp(udp, 0, unmarshal, 0)
  conf.hookUp(unmarshal, 0, unbox, 0)
  conf.hookUp(unbox, 0, cct, 1)
  conf.hookUp(cct, 1, retry, 1)
  conf.hookUp(retry, 1, retryMux, 0)
  conf.hookUp(retry, 2, retryMux, 1)
  conf.hookUp(retryMux, 0, discard, 0)

  return conf

def UdpCC_sink(udp, drop):
  conf = libp2python.Plumber.Dataflow("sink")

  udp      = conf.addElement(udp)
  # The remote data elements
  bw        = conf.addElement(libp2python.Bandwidth("Destination Bandwidth"))
  unmarshal = conf.addElement(libp2python.UnmarshalField("unmarshal", 1))
  unroute   = conf.addElement(libp2python.PelTransform("unRoute", "$1 unboxPop"))
  printS    = conf.addElement(libp2python.Print("Print Sink"))
  ccr       = conf.addElement(libp2python.CCR("CC Receive", 2048))
  discard   = conf.addElement(libp2python.Discard("DISCARD"))

  # The remote ack elements
  netsim   = conf.addElement(libp2python.SimpleNetSim("Simple Net Sim (Sender)", 10, 100, drop))
  printR    = conf.addElement(libp2python.Print("Print ACK"))
  route    = conf.addElement(libp2python.StrToSockaddr("Convert src addr", 0))
  marshal  = conf.addElement(libp2python.MarshalField("marshal ack", 1))
  destAddr = conf.addElement(libp2python.PelTransform("RESPONSE ADDRESS", "$0 pop swallow pop"))


  # PACKET RECEIVE DATA FLOW
  conf.hookUp(udp, 0, bw, 0)
  conf.hookUp(bw, 0, unmarshal, 0)
  conf.hookUp(unmarshal, 0, unroute, 0)
  conf.hookUp(unroute, 0, printS, 0)
  conf.hookUp(printS, 0, ccr, 0)
  conf.hookUp(ccr, 0, discard, 0)

  # ACK DATA FLOW
  conf.hookUp(ccr, 1, destAddr, 0)
  conf.hookUp(destAddr, 0, printR, 0)
  conf.hookUp(printR, 0, marshal, 0)
  conf.hookUp(marshal, 0, route, 0)
  conf.hookUp(route, 0, netsim, 0)
  conf.hookUp(netsim, 0, udp, 0)

  return conf

def testUdpCC(conf):
  print "INSTALL THE DATAFLOW"
  if plumber.install(conf) == 0:
    print "Correctly initialized.\n"
  else:
    print "** Failed to initialize correct spec\n"

  # Run the plumber
  libp2python.eventLoop()

if __name__ == "__main__":
  try:
    flags, args = parse_cmdline(sys.argv)
  except:
    print "EXCEPTION"
    print_usage()
    sys.exit(3)
  if len(args) < 1:
    print_usage()
    sys.exit(3)

  print args

  libp2python.eventLoopInitialize()

  port = int(args[0])
  drop = 0.
  if flags["source"]:
      src = libp2python.Udp2("SOURCE", port)
      if len(args) == 4: drop = float(args[3])
      testUdpCC(UdpCC_source(src, args[1]+":"+str(port), args[2], drop))
  elif flags["destination"]:
      dest = libp2python.Udp2("DESTINATION", port)
      if len(args) == 2: drop = float(args[1])
      testUdpCC(UdpCC_sink(dest, drop))
