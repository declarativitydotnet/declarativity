import dfparser
import p2python
import sys
import os
import getopt

p = p2python.Plumber(p2python.LoggerI.Level.ERROR)
p2python.eventLoopInitialize()

def parse_cmdline(argv): 
    shortopts = "d"
    flags = {"debug" : False}
    opts, args = getopt.getopt(argv[1:], shortopts)
    for o, v in opts:
        if   o == "-d": flags["debug"]     = True
        else: exit(3)
    return flags, args

def run():
    p2python.eventLoop()

def install(f):
    try: s = open(f + ".df", 'r').read()
    except EOFError: print "FILE READ ERROR"
    dfparser.dataflows = {}
    dfparser.compile(p, s)

    if dfparser.dataflows.has_key("Main"):
      m = dfparser.dataflows["Main"]
      m.eval_dataflow()
      install_config(m.conf)
    else:
      for d in dfparser.dataflows.values():
        d.eval_dataflow()
        install_config(d.conf)

def install_config(conf):
    # Okay, lets install it
    if p.install(conf) == 0:
      print "Correctly initialized.\n"
    else:
      print "** Failed to initialize correct spec\n"

def dump(f):
    p.toDot(f + ".dot")
    os.system("dot -Tps "+f+".dot -o "+f+".ps")

if __name__=='__main__':
    try:
        flags, args = parse_cmdline(sys.argv)
        if len(args) < 1: 
            # print_usage()
            sys.exit(0)
    except:
        print "EXCEPTION"
        # print_usage()
        sys.exit(0)

    install(args[0])

    if flags["debug"]:
      dump("debug")
    else: # Run the plumber
      run()
