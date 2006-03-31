import dfparser
import p2python
import sys
import os
import getopt

p = p2python.Plumber(p2python.LoggerI.Level.ERROR)
p2python.eventLoopInitialize()

def print_usage():
    print "USAGE: make sure your PYTHONPATH environment variable is set as follows:"
    print "export PYTHONPATH=(top_buiddir)/python/dfparser:(top_builddir)/python/dfparser/yapps:(top_builddir)/python/lib"
    print "Where top_builddir is the top level P2 directory e.g., /../../phi/phi"
    print "EXEC: python dfrun.py [-d] <dataflow file>" 

def parse_cmdline(argv): 
    shortopts = "d"
    flags = {"debug" : False}
    opts, args = getopt.getopt(argv[1:], shortopts)
    for o, v in opts:
        if   o == "-d": flags["debug"] = True
        else:
            print_usage()
            sys.exit(1)
    return flags, args

def run():
    p2python.eventLoop()

def install(f):
    try: s = open(f + ".df", 'r').read()
    except EOFError: print "FILE READ ERROR"
    dfparser.dataflows = {}
    dfparser.compile(p, p2python, s)

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
    os.remove(f+".dot")

if __name__=='__main__':
    try:
        flags, args = parse_cmdline(sys.argv)
        if len(args) < 1: 
            sys.exit(1)
    except:
        print_usage()
        sys.exit(1)

    dataflowFile = args[0]
    if dataflowFile[-3:] == ".df":
        dataflowFile = dataflowFile[:-3]    
    install(dataflowFile)

    if flags["debug"]:
      dump(dataflowFile)
    else: # Run the plumber
      run()
