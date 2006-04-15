# P2 dataflow parser

import sys
import getopt
import copy
import libp2python
from string import strip

plumber    = None
dataflows  = {}
edits      = []
macros     = []
local      = []
flags      = {"debug" : False}

def error(e):
    print "ERROR: ", e
    sys.exit(1)

def parse_cmdline(argv): 
    shortopts = "d"
    opts, args = getopt.getopt(argv[1:], shortopts)
    for o, v in opts:
        if   o == "-d": flags["debug"]     = True
        else: exit(3)
    return flags, args

def eval_value(val, args):
    arguments = []
    for a in args: 
        if a[0] == 'var':
            error("Value types do not take variable arguments.")
        arguments.append(a[1])
    if flags["debug"]: return val + str(arguments)
    else: 
        v = apply(getattr(getattr(libp2python,val), 'mk'), tuple(arguments))
        if not v: error("Unable to evaluate value %s" % val)
        return v

class Dataflow:
    def __init__(self, name):
        self.globalvars = {}       # We will store global variables here
        self.strands    = []
        if not plumber:
            error("PLUMBER NOT SET")
        self.conf = libp2python.Plumber.Dataflow(name)
        self.name = name
    def eval_dataflow(self):
        for s in self.strands:
            s.eval_strand(self)
        return self.conf
    def print_dataflow(self):
        print "\n************* DATAFLOW PRINT *************"
        print "DATAFLOW GLOBAL CONTEXT:  ", self.globalvars
        for s in self.strands:
            s.print_strand()
    def eval_arg(self, arg):
        if arg[0] == 'var':
            arg = self.lookup([], arg[1])
            if not arg: 
                error("Variable arguement does not exist")
            if not flags["debug"] and not isinstance(arg, libp2python.ValueVec):
                error("Variable arguement must be of type ValueVec")
            return arg
        return arg[1]
    def eval_element(self, type, args):
        arguments = []
        for a in args: 
            arguments.append(self.eval_arg(a)) 
        if flags["debug"]: return type + str(arguments)
        else: 
            element = apply(getattr(libp2python, type), tuple(arguments))
            if not element: error("Unable to create element %s %s" % (type, str(arguments)))
            elementSpec = self.conf.addElement(element)
            if not elementSpec: error("Unable to add element %s %s" % (type, str(arguments)))
            return elementSpec
    def operation(self, oper, name, port):
        error("Operations only supported under edits.")
    def eval_ref(self, d, r):
        error("Basic Dataflows can't reference other dataflow elements.")
    def lookup(self, map, name):
        for x,v,a in map:  
            if x == name: return v
        for x,v,a in macros:  
            if x == name: return v
        return self.globalvars.get(name, 0)
    def strand(self, s):
        self.strands.append(s)

class Edit(Dataflow):
    def __init__(self, name):
        Dataflow.__init__(self, name)
        self.conf = plumber.new_dataflow_edit(name)
        if not self.conf:
            error("Unable to create a dataflow edit on %s." % name)
    def operation(self, oper, name, port):
          element = conf.find(name)
          if oper == "add_input" or oper == "add_output": 
            if port: return element.attr(oper)(port)
            else: return element.attr(oper)()
          elif oper == "remove_input" or oper == "remove_output":
            element.attr(oper)(port)
          else: error("Unknown dataflow operation.")
          return -1
    def eval_ref(self, r):
        if flags["debug"]: return "REF(" + d + "." + r + ")"
        else: 
            ref = self.conf.find(r)
            if not ref: 
                error("Unable to resolve reference %s in dataflow %s" %
                      (r, self.name))
            return ref

class Macro:
    def __init__(self, n):
        self.name      = n
        self.input     = None
        self.output    = None
        self.context   = []
        self.strands   = []
        self.formals   = []
        self.evaluated = False

    def strand(self, s):
        self.strands.append(s) 

    def print_macro(self):
        print "[DATAFLOW CLASS"
        print "CLASS NAME: %s" % self.name
        print "CLASS CONTEXT: ", self.context
        print "CLASS INPUT: ", self.input
        print "CLASS OUTPUT: ", self.output
        print "CLASS FORMALS: ", self.formals
        for s in self.strands: 
            for e in s.elements:
                print e, "->",
            print "END LOCAL DATAFLOW"
        print "END DATAFLOW CLASS %s]" % self.name

    def apply_args(self, args, actuals):
        if len(args) > 0 and args[0] != 'name':
            args[0][1] = actuals[0][1] + "." + args[0][1]
        for i in range(len(args)):
            for j in range(len(self.formals)):
                if args[i][0] == 'var' and self.formals[j] == args[i][1]:
                    args[i] = actuals[j]

    def eval_macro(self, actuals, d):
        if self.evaluated: return
        if len(actuals) != len(self.formals):
            error("Wrong number of arguements to macro %s" % self.name)
        # Evaluate the local context (Can only be another macro or local variable)
        input_resolved = False
        output_resolved = False
        for c in self.context:
            if isinstance(c[1], Macro):
                if c[1].name == self.name:
                    error("Macro class self reference.")
                self.apply_args(c[2], actuals)
                macro = copy.deepcopy(c[1])
                macro.eval_macro(c[2], d)
            else: 
                self.apply_args(c[2], actuals)
                c[1] = d.eval_element(c[1],c[2])
            if self.input and self.input == c[0]: 
                self.input = c[1]
                input_resolved = True
            if self.output and self.output == c[0]: 
                self.output = c[1]
                output_resolved = True
        if self.input and not input_resolved: 
            error("Input \"%s\" was not resolved in macro \"%s\"" % (self.input, self.name))
        if self.output and not output_resolved: 
            error("Output \"%s\" was not resolved in macro \"%s\"" % (self.output, self.name))
        # Evaluate the local strands
        for s in self.strands:
            # Apply actuals to elements and dataflow macros
            s.context = self.context	# Copy evaluated context into strand
            # Apply actuals to elements
            for i,e,o in s.elements:
                # Apply macro arguments where appropriate
                if e[0] == 'new':
                    if e[1] == self.name:
                       error("Macro class self reference.")
                    self.apply_args(e[2], actuals)
            s.eval_strand(d)
        self.evaluated = True

class Strand:
    def __init__(self, c):
        self.elements = []
        self.context  = c 
        self.evaluated = False
    def element(self, e):
        self.elements.append(e)
    def print_strand(self):
        print "STRAND ELEMENTS: "
        for e in self.elements:
            if isinstance(e[1], Macro):
                e[1].print_macro()    
            else: print e,
            print "->"
        print "END"

    def eval_strand(self, d):
        if self.evaluated: return
        elements = []
        for i,e,o in self.elements:
            if e[0] == 'new':
                macro = d.lookup(self.context, e[1])
                if macro:
                    macro = copy.deepcopy(macro)
                    macro.eval_macro(e[2], d)
                    elements.append([i, macro, o]) 
                else:
                    elements.append([i, d.eval_element(e[1],e[2]), o])
            elif e[0] == 'var':
                var = d.lookup(self.context, e[1])
                if not var: error("Unable to resolve variable %s in dataflow %s." % (e[1], d.name))
                elements.append([i, var, o])
            elif e[0] == 'ref':
                elements.append([i, d.eval_ref(e[1]), o])
        self.elements = elements

        # Hook everyone together
        if not flags["debug"]:
            for i in range(len(self.elements)-1):
                f = self.elements[i]
                t = self.elements[i+1]
                if isinstance(f[1], Macro): 
                    f[1] = f[1].output
                if isinstance(t[1], Macro): 
                    t[1] = t[1].input
                # Make sure ports are numbers and not keys
                if not isinstance(f[2], int):
                  try:
                    f[2] = f[1].element().input(f[2])
                  except: 
                    print "EXCEPTION: %s\n" % str(sys.exc_info()[:2])
                    print "Unable to resolve value %s to a port number." % f[2].toString()
                if not isinstance(t[0], int):
                  try:
                    t[0] = t[1].element().input(t[0])
                  except: 
                    print "EXCEPTION: %s\n" % str(sys.exc_info()[:2])
                    print "Unable to resolve value %s to a port number." % t[0].toString()
                d.conf.hookUp(f[1], f[2], t[1], t[0])
        self.evaluated = True

%%
parser P2Dataflow:
    token END:   "\."
    token FLOAT: "[0-9]*\.[0-9]+"
    token NUM:   "[0-9]+"
    token VAL:   "Val_[a-zA-Z0-9_]*"
    token VAR:   "[a-z][a-zA-Z0-9_]*"
    token TYPE:  "[A-Z][a-zA-Z0-9_]*"
    token LINK:  r"->"
    token STR:   "[^ ][\" '\$\._A-Za-z0-9\\\:]*"
    ignore:      "[ \r\t\n]+"
    ignore:      r'#.*\r?\n'    # DL comments; sh/perl style

    rule module:      (dataflow {{ pass }}
                       |
                       macro    {{ pass }}
                       |
                       edit     {{ pass }}
                      )* END

    rule macro: "macro" TYPE {{ m = Macro(TYPE) }}
                  formals {{ if len(formals) == 0 or formals[0] != 'name': error("MACRO %s: First formal of  must be 'name'." % TYPE) }} 
                          {{ m.formals = formals }} "{"                            
                  (local_def<<m.context>>       {{ pass }})* 
                  (  "input"  VAR ";"           {{ m.input = VAR }} 
                   | "output" VAR ";"           {{ m.output = VAR }} )*
                  (strand<<m.context>>          {{ m.strand(strand) }} )*
                  "}"                           {{ macros.append([m.name, m, []]) }}

    rule dataflow: "dataflow" TYPE "{"   {{ d = Dataflow(TYPE) }} 
                        (global_def<<d>> {{ pass }}
                         |
                         strand<<[]>>    {{ d.strand(strand) }}
                        )*
                    "}"                  {{ dataflows[TYPE] = d }}

    rule edit: "edit" TYPE "{"           {{ e = Edit(TYPE) }}
                        (global_def<<e>> {{ pass }}
                         |
                         operation<<e>>  {{ pass }}
                         | 
                         strand<<[]>>    {{ e.strand(strand) }}
                        )*
                    "}"                  {{ edits.append(e) }}

    rule local_def<<C>>: "let" VAR "=" TYPE args ";" {{ C.append([VAR, TYPE, args]) }}

    rule global_def<<D>>: "let" VAR "=" 
                          (TYPE args
                               {{ e = D.lookup([], TYPE) }}
                               {{ if e:     e = copy.deepcopy(e) }}
                               {{ if e:     e.eval_macro(args, D)   }}
                               {{ if not e: e = D.eval_element(TYPE,args) }}
                               {{ D.globalvars[VAR] = e }}
                           |
                           values {{ D.globalvars[VAR] = values }}
                          ) 
                          ";" 

    rule operation<<D>>: "add"    (port VAR {{ return D.operation('add_input', VAR, port) }}
                                   |
                                   VAR port {{ return D.operation('add_output', VAR, port) }}) 
                         "remove" (port VAR {{ return D.operation('remove_input', VAR, port) }}
                                   |
                                   VAR port {{ return D.operation('remove_output', VAR, port) }}) 
                         ";"

    rule strand<<C>>: term {{ outp = 0 }} {{ s = Strand(C) }}
                           [port {{ outp = port }}] {{ s.element([-1, term, outp]) }}
                      (LINK
                        (term  {{ outp = 0 }}
                             [port {{ outp = port }} ] 
                             {{ s.element([0, term, outp]) }}
                         | 
                         port term {{ inp = port }} {{ outp = 0    }}
                             [port     {{ outp = port }} ]  
                             {{ s.element([inp, term, outp]) }}
                        )
                      )+
                      ";" {{ return s }}

    rule term: (TYPE  [args {{ return ['new', TYPE, args] }} ]
                       {{ return ['new', TYPE, []] }}
                | 
                "\." VAR {{ ref = VAR }}
                  ("\." VAR {{ ref = ref + "." + VAR }})* {{ return ['ref', ref] }}
                |
                VAR      {{ return ['var', VAR] }})

    rule port: "\["       {{ port = None }}
               [(NUM      {{ port = int(NUM) }}
                |
                VAL args  {{ port = eval_value(VAL, args) }} )]
               "\]"       {{ return port }}

    rule args: "\("           {{ a = [] }}
                 [arg         {{ a.append(arg) }}           
                   (',' arg   {{ a.append(arg) }})*]
                "\)"          {{ return a }}

    rule arg: (STR      {{ if STR[0] == "\"": STR = STR[1:] }} 
                        {{ if STR[-1]== "\"": STR = STR[:-1] }}
                        {{ return ['str', str(STR)] }}
               |
               NUM      {{ return ['num', int(NUM)] }} 
               |
               FLOAT    {{ return ['float', float(FLOAT)] }} 
               |
               VAL args {{ return ['val', eval_value(VAL, args)] }} 
               |
               values   {{ return ['vals', values] }} 
               |
               VAR      {{ return ['var', VAR] }} )

    rule formals: "\("        {{ f = [] }}
                 [VAR         {{ f.append(VAR) }}           
                   (',' VAR   {{ f.append(VAR) }})*]
                "\)"          {{ return f}}

    rule values: "{"             {{ if flags["debug"]: v = [] }} 
                                 {{ if not flags["debug"]: v = libp2python.ValueVec() }}
                 [VAL args       {{ v.append(eval_value(VAL, args)) }}           
                   (',' VAL args {{ v.append(eval_value(VAL, args)) }})*]
                 "}"             {{ return v}}

%%

def compile(p, s):
    global plumber
    plumber = p
    parse('module', s)
    return dataflows

def debug():
    if flags["debug"]:
        print "\n================= DATAFLOW MACROS ================="
        for n,c,a in macros: c.print_macro()
        print "\n===================================================="
        print "\n================= GLOBAL DATAFLOWS ================="
        for d in dataflows.values(): d.eval_dataflow()
        for k in dataflows.keys(): 
            print "DATAFLOW: ", k
            dataflows[k].print_dataflow()
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
            parse('strand', s)
    else:
        try: s = open(args[0], 'r').read()
	except EOFError: print "FILE READ ERROR"
        import libp2python
        p = libp2python.Plumber()
        compile(p, libp2python, s)
        if flags["debug"]: debug()
    print 'Bye.'
    sys.exit(0)
