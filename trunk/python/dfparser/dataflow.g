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
            n = arg[1]
            arg = self.lookup([], arg[1])
            if not arg: 
                error("Variable argument does not exist: argument name %s" % n)
            return arg
        return arg[1]
    def eval_function(self, var, functions):
        if flags["debug"]: 
            fn_str = ""
            for f in functions:
                fn_str.append(self.eval_function_helper(var, f[0], f[1])) 
                var = ""
            return fn_str
        obj = self.lookup([], var) 
        if not obj: 
            obj = getattr(libp2python, var)	# static class method? 
        if not obj: 
            error("Unable to locate object variable %s" % var)	# Give up
        for f in functions:
            obj = self.eval_function_helper(obj, f[0], f[1])
        if isinstance(obj, libp2python.Element):
            elementSpec = self.conf.addElement(obj)
            if not elementSpec: error("Unable to add element %s" % (obj.class_name()))
            return elementSpec
        return obj
    def eval_function_helper(self, context, func, args):
        arguments = []
        for a in args: 
            arguments.append(self.eval_arg(a)) 
        if flags["debug"]: 
            return context + "." + func + str(arguments)
        else: 
            return apply(getattr(context, func), tuple(arguments))
    def eval_object(self, type, args):
        arguments = []
        for a in args: 
            arguments.append(self.eval_arg(a)) 
        if flags["debug"]: return type + str(arguments)
        else: 
            if type == "Table2":
                print "Table2 arguments ", arguments
                print "Method: ", getattr(self.conf, "table")
                obj = apply(getattr(self.conf, "table"), tuple(arguments))    
                print "TABLE: ", obj
            else:
                obj = apply(getattr(libp2python, type), tuple(arguments))
                if not obj: error("Unable to create object %s %s" % (type, str(arguments)))
                if isinstance(obj, libp2python.Element):
                    elementSpec = self.conf.addElement(obj)
                    if not elementSpec: error("Unable to add element %s %s" % (type, str(arguments)))
                    return elementSpec
            return obj
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
          if not isinstance(name, libp2python.ElementSpec):
            element = self.eval_ref(name)
          else: element = name
          fn = getattr(element, oper)
          if oper == "add_input" or oper == "add_output": 
            if port: 
                return apply(fn, tuple([port]))
            else: 
                return apply(fn, ())
          elif oper == "remove_input" or oper == "remove_output":
            apply(fn, tuple(port))
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
                c[1] = d.eval_object(c[1],c[2])
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
                    elements.append([i, d.eval_object(e[1],e[2]), o])
            elif e[0] == 'var':
                var = d.lookup(self.context, e[1])
                if not var: error("Unable to resolve p2dl variable %s in dataflow %s." % (e[1], d.name))
                elements.append([i, var, o])
            elif e[0] == 'ref':
                elements.append([i, d.eval_ref(e[1]), o])
        self.elements = elements

        # Hook everyone together
        if not flags["debug"]:
            for i in range(len(self.elements)-1):
                f = copy.copy(self.elements[i])
                t = copy.copy(self.elements[i+1])
                if isinstance(f[1], Macro): 
                    f[1] = f[1].output
                if isinstance(t[1], Macro): 
                    t[1] = t[1].input
                # Make sure ports are numbers and not keys
                if f[2] == None:
                    # Try to add a port (will throw if not supported or not an edit)
                    f[2] = d.operation("add_output", f[1], None)
                elif not isinstance(f[2], int):
                  try:
                    # See if port already exists
                    f[2] = f[1].attr(element)().attr(input)(f[2])
                  except: 
                    # Try to add the port (will throw if not supported or not an edit)
                    f[2] = d.operation("add_output", f[1], f[2])

                if t[0] == None:
                    # Try to add a port (will throw if not supported or not an edit)
                    t[0] = d.operation("add_input", t[1], None)
                if not isinstance(t[0], int):
                  try:
                    # See if port already exists
                    t[0] = t[1].attr(element)().attr(input)(t[0])
                  except: 
                    # Try to add the port (will throw if not supported or not an edit)
                    t[0] = d.operation("add_input", t[1], t[0])
                d.conf.hookUp(f[1], f[2], t[1], t[0])
        self.evaluated = True

%%
parser P2Dataflow:
    token END:   "\."
    token FLOAT: "[0-9]*\.[0-9]+"
    token NUM:   "\-?[0-9]+"
    token HEX:   "0x[a-fA-F0-9]+"
    token VAL:   "Val_[a-zA-Z0-9_]*"
    token VAR:   "[a-z][a-zA-Z0-9_]*"
    token TYPE:  "[A-Z][a-zA-Z0-9_]*"
    token LINK:  r"->"
    token STR:   "\"[' \(\)\[\]'\$\._A-Za-z0-9\\:\-\+\*=<>]*\""
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
                  (macro_def<<m.context>>       {{ pass }})* 
                  (  "input"  VAR ";"           {{ m.input = VAR }} 
                   | "output" VAR ";"           {{ m.output = VAR }} )*
                  (strand<<m.context>>          {{ m.strand(strand) }} )*
                  "}"                           {{ macros.append([m.name, m, []]) }}

    rule dataflow: "dataflow" TYPE "{"   {{ d = Dataflow(TYPE) }} 
                        (global_def<<d>> {{ pass }}
                         |
                         strand<<[]>>    {{ d.strand(strand) }}
                         |
                         call<<d>> ";"   {{ pass }}
                        )*
                    "}"                  {{ dataflows[TYPE] = d }}

    rule edit: "edit" TYPE "{"           {{ e = Edit(TYPE) }}
                        (global_def<<e>> {{ pass }}
                         |
                         operation<<e>>  {{ pass }}
                         | 
                         strand<<[]>>    {{ e.strand(strand) }}
                         |
                         call<<e>> ";"   {{ pass }}
                        )*
                    "}"                  {{ edits.append(e) }}

    rule macro_def<<C>>: "let" VAR "=" TYPE args ";" {{ C.append([VAR, TYPE, args]) }}

    rule global_def<<D>>: "let" VAR "=" 
                          (TYPE args
                               {{ e = D.lookup([], TYPE) }}
                               {{ if e:     e = copy.deepcopy(e) }}
                               {{ if e:     e.eval_macro(args, D)   }}
                               {{ if not e: e = D.eval_object(TYPE,args) }}
                               {{ D.globalvars[VAR] = e }}
                           |
                           vector    {{ D.globalvars[VAR] = vector }}
                           |
                           tuple     {{ D.globalvars[VAR] = tuple }}
                           |
                           call<<D>> {{ D.globalvars[VAR] = call }}
                          ) 
                          ";"

    rule operation<<D>>: "add"    (port VAR {{ return D.operation('add_input', VAR, port) }}
                                   |
                                   VAR port {{ return D.operation('add_output', VAR, port) }}) 
                         "remove" (port VAR {{ return D.operation('remove_input', VAR, port) }}
                                   |
                                   VAR port {{ return D.operation('remove_output', VAR, port) }}) 
                         ";"

    rule call<<D>>: "call" callType     {{ obj = callType }} {{ functions = [] }}
                    ("\." callType args {{ functions.append([callType, args]) }})+
                                        {{ return D.eval_function(obj, functions) }}

    rule callType: (VAR {{ return VAR }} | TYPE {{ return TYPE}})

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
                "\." VAR    {{ ref = VAR }}
                  ("\." VAR {{ ref = ref + "." + VAR }})* {{ return ['ref', ref] }}
                |
                VAR      {{ return ['var', VAR] }})

    rule port: "\["       {{ port = None }}
               [(NUM      {{ port = int(NUM) }}
                |
                '\+'      {{ pass }}
                |
                VAL args  {{ port = eval_value(VAL, args) }} )]
               "\]"       {{ return port }}

    rule args: "\("           {{ a = [] }}
                 [arg         {{ a.append(arg) }}           
                   ("," arg   {{ a.append(arg) }})*]
                "\)"          {{ return a }}

    rule arg: (STR        {{ if STR[0] == "\"": STR = STR[1:] }} 
                          {{ if STR[-1]== "\"": STR = STR[:-1] }}
                          {{ return ['str', STR.replace("'", "\"")] }}
               |
               NUM        {{ return ['num', int(NUM)] }} 
               |
               HEX        {{ return ['hex', HEX] }} 
               |
               FLOAT      {{ return ['float', float(FLOAT)] }} 
               |
               VAL args   {{ return ['val', eval_value(VAL, args)] }} 
               |
               vector     {{ return ['vec', vector] }} 
               |
               tuple      {{ return ['tup', tuple] }} 
               |
               VAR        {{ return ['var', VAR] }} )

    rule formals: "\("        {{ f = [] }}
                 [VAR         {{ f.append(VAR) }}           
                   (',' VAR   {{ f.append(VAR) }})*]
                "\)"          {{ return f}}

    rule vector: "{"          {{ v = [] }} 
                   (  "str"   {{ v = libp2python.StrVec()   }}
                    | "int"   {{ v = libp2python.IntVec()   }}
                    | "value" {{ v = libp2python.ValueVec() }}
                    |
                    (VAL args      {{ if not flags["debug"]: v = libp2python.ValueVec() }} 
                                   {{ v.append(eval_value(VAL, args)) }}           
                     (',' VAL args {{ v.append(eval_value(VAL, args)) }})*
                     |
                     NUM       {{ if not flags["debug"]: v = libp2python.IntVec() }}
                               {{ v.append(int(NUM)) }}
                      (',' NUM {{ v.append(int(NUM)) }})*
                     |
                     STR       {{ if not flags["debug"]: v = libp2python.StrVec() }}
                               {{ if len(STR) >  0: v.append(STR[1:-1]) }}           
                               {{ if len(STR) == 0: v.append(STR) }}           
                      (',' STR {{ if len(STR) >  0: v.append(STR[1:-1]) }}
                               {{ if len(STR) == 0: v.append(STR) }})*)
                   )
                 "}" {{ return v}}

    rule tuple: "<"              {{ if flags["debug"]:     t = [] }} 
                                 {{ if not flags["debug"]: t = libp2python.Tuple() }}
                 [VAL args       {{ t.append(eval_value(VAL, args)) }}           
                   (',' VAL args {{ t.append(eval_value(VAL, args)) }})*]
                 ">"             {{ return t}}

%%

def compile(p, s):
    global plumber
    plumber = p
    parse('module', s)
    return dataflows, edits

def clear():
    global dataflows
    global edits
    dataflows = {}
    edits     = []

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
        compile(p, s)
        if flags["debug"]: debug()
    print 'Bye.'
    sys.exit(0)
