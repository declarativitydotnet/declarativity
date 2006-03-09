import dfparser
import p2python
import sys
import getopt

def parse_cmdline(argv): 
    shortopts = "d"
    flags = {"debug" : False}
    opts, args = getopt.getopt(argv[1:], shortopts)
    for o, v in opts:
        if   o == "-d": flags["debug"]     = True
        else: exit(3)
    return flags, args

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

    p2python.eventLoopInitialize()
    try: s = open(args[0], 'r').read()
    except EOFError: print "FILE READ ERROR"
    dfparser.parse('program', s)

    conf = dfparser.install()

    # Okay, lets run it
    plumber = p2python.Plumber(conf, p2python.LoggerI.Level.WARN)
    if plumber.initialize(plumber) == 0:
      print "Correctly initialized.\n"
    else:
      print "** Failed to initialize correct spec\n"

    # Activate the plumber
    plumber.activate()

    # Run the plumber
    p2python.eventLoop()
