# P2 dataflow parser

import sys
import getopt
import copy
from p2python import *
from string import strip

conf       = Plumber.Configuration()
globalvars = {}       # We will store global variables here
macros    = []
dataflows  = []
flags      = {"debug" : False}

def install():
    for d in dataflows:
        d.install()
    return conf

def eval_element(type, args):
    cmd = type + "("
    # Take care of all but the last arguement
    if len(args) > 1:
       for a in args[:-1]: cmd += a + ", " 
    if args: cmd += args[-1]  # Get the last one
    cmd += ")" 
    if flags["debug"]: return cmd
    else: return conf.addElement(eval(cmd))

def lookup(map, name):
    for x,v,a in map:  
        if x == name: return v
    if not globalvars.has_key(name): print 'Undefined (defaulting to 0):', name
    return globalvars.get(name, 0)

def parse_cmdline(argv): 
    shortopts = "d"
    opts, args = getopt.getopt(argv[1:], shortopts)
    for o, v in opts:
        if   o == "-d": flags["debug"]     = True
        else: exit(3)
    return flags, args

class DataflowMacro:
    def __init__(self, n):
        self.name      = n
        self.input     = None
        self.output    = None
        self.context   = []
        self.dataflows = []
        self.formals   = []
        self.evaluated = False
        self.installed = False

    def add_dataflow(self, d):
        self.dataflows.append(d) 

    def print_macro(self):
        print "[DATAFLOW CLASS"
        print "CLASS NAME: %s" % self.name
        print "CLASS CONTEXT: ", self.context
        print "CLASS INPUT: ", self.input
        print "CLASS OUTPUT: ", self.output
        print "CLASS FORMALS: ", self.formals
        for d in self.dataflows: 
            for e in d.elements:
                print e, "->",
            print "END LOCAL DATAFLOW"
        print "END DATAFLOW CLASS %s]" % self.name

    def apply_args(self, args, actuals):
        for i in range(len(args)):
            for j in range(len(self.formals)):
                if self.formals[j] == args[i]:
                    args[i] = actuals[j]

    def eval_macro(self, actuals):
        if self.evaluated: return
        assert(len(actuals) == len(self.formals))
        self.context   = copy.deepcopy(self.context)
        self.dataflows = copy.deepcopy(self.dataflows)
        # Evaluate the local context
        for c in self.context:
            if isinstance(c[1], DataflowMacro):
                assert(c[1].name != self.name)
                self.apply_args(c[2], actuals)
                c[1].eval_macro(c[2])
            else: 
                self.apply_args(c[2], actuals)
                c[1] = eval_element(c[1],c[2])
        # Evaluate the local dataflows
        for d in self.dataflows:
            # Apply actuals to elements and dataflow macros
            d.context = self.context	# Copy evaluated context into dataflow
            # Apply actuals to elements
            for i,e,o in d.elements:
                # Apply macro arguments where appropriate
                if e[0] == 'new':
                    self.apply_args(e[2], actuals)
                elif e[0] == 'macro':
                    assert(e[1].name != self.name)
                    self.apply_args(e[2], actuals)
            d.eval_dataflow()
        self.evaluated = True

    def install(self, actuals):
        if self.installed: return
        self.eval_macro(actuals)
        for d in self.dataflows:
            d.install()
        self.installed = True

class Dataflow:
    def __init__(self, c):
        self.elements = []
        self.context  = c 
        self.evaluated = False
    def element(self, e):
        self.elements.append(e)
    def print_dataflow(self):
        print "\n************* DATAFLOW PRINT *************"
        print "DATAFLOW CONTEXT:  ", self.context
        print "ELEMENTS"
        for e in self.elements:
            if isinstance(e[1], DataflowMacro):
                e[1].print_macro()    
            else: print e,
            print "->"
        print "END"
    def eval_dataflow(self):
        if self.evaluated: return
        elements = []
        for i,e,o in self.elements:
            if e[0] == 'new':
                elements.append([i, eval_element(e[1],e[2]), o])
            elif e[0] == 'var':
                elements.append([i, lookup(self.context, e[1]), o])
            elif e[0] == 'macro':
                c = lookup(self.context, e[1])
                assert(c)
                c.eval_macro(e[2])
                elements.append([i, c, o]) 
        self.elements = elements
        self.evaluated = True
    def install(self):
        self.eval_dataflow()
        # Hook everyone together
        for i in range(len(self.elements)-1):
            f = self.elements[i]
            t = self.elements[i+1]
            if isinstance(f, DataflowMacro): 
                f.install()
                f = f.output
            if isinstance(t, DataflowMacro): 
                t.install()
                t = t.input
            conf.hookUp(f[1], f[2], t[1], t[0])

%%
parser P2Dataflow:
    token END:   "\."
    token NUM:   "[0-9]+"
    token VAR:   "[a-z][a-zA-Z0-9_]*"
    token TYPE:  "[A-Z][a-zA-Z0-9_]*"
    token DNAME: "_[a-zA-Z0-9]*"
    token LINK:  r"->"
    token ARG:   "[^ ][ \"'\$\._A-Za-z0-9\\\:]*"
    ignore:      "[ \r\t\n]+"

    rule program: (global_def<<macros>> {{ pass }}
                   |
                   macro                {{ macros.append(macro) }}
                   |
                   dataflow<<macros>>   {{ dataflows.append(dataflow) }}
                  )* END

    rule global_def<<V>>: "let" VAR ":=" 
                          (TYPE args
                               {{ globalvars[VAR] = eval_element(TYPE,args) }}
                           |
                           DNAME args 
                               {{ d = lookup(V, DNAME) }}
                               {{ d.eval_macro(args)   }}
                               {{ globalvars[VAR] = d  }}
                          ) ";" 

    rule local_def<<V>>: "let" VAR ":=" TYPE args ";" {{ V.append([VAR, TYPE, args]) }}

    rule macro: "macro" DNAME {{ d = DataflowMacro(DNAME) }}
                  [formals {{ d.formals = formals }}] "{"                            
                  (local_def<<d.context>>       {{ pass }})* 
                  (  "input"  VAR ";"           {{ d.input = VAR }} 
                   | "output" VAR ";"           {{ d.output = VAR }} )*
                  (dataflow<<d.context>>        {{ d.add_dataflow(dataflow) }} )*
                  "}"                           {{ return [d.name, d, []] }}

    rule dataflow<<V>>: term                                       {{ outp = 0 }} 
                                                                   {{ d = Dataflow(V) }}
                          [port {{ outp = port }}]                 {{ d.element([-1, term, outp]) }}
                        (LINK
                         (term                                     {{ outp = 0 }}
                            [";" {{ d.element([0, term, -1]) }}    {{ return d }} ]
                            [port                                  {{ outp = port }} ]        
                                                                   {{ d.element([0, term, outp]) }}
                          | port term                              {{ inp = port }}
                                                                   {{ outp = 0    }}
                            [";"  {{ d.element([inp, term, -1]) }} 
                                  {{ return d }} ]
                            [port {{ outp = port }} ]  
                                                                   {{ d.element([inp, term, outp]) }}
                         )
                      )+                            

    rule term:   
                 TYPE    [args {{ return ['new', TYPE, args] }} ]
                         {{ return ['new', TYPE, []] }}
               | VAR     {{ return ['var', VAR] }}
               | DNAME   [args {{ return ['macro', DNAME, args] }}]
                         {{ return ['macro', DNAME, []] }}

    rule port: "\[" NUM+ "\]" {{ return int(NUM) }}

    rule args: "\("           {{ a = [] }}
                 [ARG         {{ a.append(ARG) }}           
                   (',' ARG   {{ a.append(ARG) }})*]
                "\)"          {{ return a }}

    rule formals: "\("        {{ f = [] }}
                 [VAR         {{ f.append(VAR) }}           
                   (',' VAR   {{ f.append(VAR) }})*]
                "\)"          {{ return f}}

%%
def debug():
    if flags["debug"]:
        print "\n================= GLOBAL VARIABLES ================="
        print globalvars
        print "\n================= DATAFLOW CLASSES ================="
        for n,c,a in macros: c.print_macro()
        print "\n===================================================="
        print "\n================= GLOBAL DATAFLOWS ================="
        for d in dataflows: d.eval_dataflow()
        for d in dataflows: 
            print d.context
            d.print_dataflow()
        print "\n===================================================="

if __name__=='__main__':
    try:
        flags, args = parse_cmdline(sys.argv)
    except:
        print "EXCEPTION"
        # print_usage()
        sys.exit(0)

    if len(args) < 1:
        print 'Welcome to the P2 dataflow parser.'
        while 1:
            try: s = raw_input('>>> ')
	    except EOFError: break
            if not strip(s): break
            parse('dataflow', s)
    else:
        try: s = open(args[0], 'r').read()
	except EOFError: print "FILE READ ERROR"
        parse('program', s)
        if flags["debug"]: debug()
    print 'Bye.'
    sys.exit(0)
