#!/usr/bin/env python
"""
 @(#)$Id: pel_gen.py 1416 2007-10-02 06:35:28Z prince $

This file is distributed under the terms in the attached LICENSE file.
If you do not find this file, copies can be found by writing to:
Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
Or
UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
Berkeley, CA,  94707. Attention: P2 Group.

DESCRIPTION: Generate the defintion of the PEL (P2 Expression
             Language) virtual machine
"""

import sys
import string

decls=[]
curop = 0

def emit_opcode( op, ar, va, desc ):
  global curop
  decls.append((curop, op, ar, va, desc))
  curop += 1
  
for op, ar, va, desc in [
  ("drop",  1, "DROP",  "Discard the top of stack"),
  ("swap",  2, "SWAP",  "Swap top two stack values"),
  ("dup",   1, "DUP",   "Duplicate the top stack value"),
  (""      ,0, "PUSH_CONST",  "Push a constant"),
  ("" , 0,        "PUSH_FIELD",   "Push a field of the tuple"),
  ("pop",1,     "POP",          "Pop a value into the result tuple"),
  ("popall",1,  "POP_ALL",      "Pop remaining stack to result tuple"),
  ("peek", 1,   "PEEK",         "Push a duplicate of a stack element"),
  ("ifelse",3,  "IFELSE",       "If first arg, then return second, else third"),
  ("ifpop",2,   "IFPOP",        "If first arg, then pop second, else nothing"),
  ("ifpoptuple",1,"IFPOP_TUPLE","If first arg, then pop entire tuple, else nothing"),
  ("ifstop",1,   "IFSTOP",      "If first arg, then stop execution"),
  ("dumpStack",1,"DUMPSTACK",   "Dump the contents of the stack prefixed by the string arg"),



  ("func0", 1,   "FUNC0",       "Perform the named function given in the arg without parameters"),
  


  ("sha1",1,"H_SHA1","Replace the top of the stack with its SHA-1 value"),

  ("max",2,"MAX","Return the max of 2 values."),
  ("min",2,"MIN","Return the min of 2 values."),
 
  ("idgen",0,"T_IDGEN","Generate a unique tuple identifier."),
  ("->t", 1,    "T_MKTUPLE",    "Create a tuple out of the argument"),
  ("append", 2, "T_APPEND",     "Append first argument to second tuple"),
  ("unbox", 1,  "T_UNBOX",      "Replaces a tuple value with its fields top to bottom"),
  ("unboxPop",1,"T_UNBOXPOP",   "Pops the fields of the argument tuple value in order"),
  ("field", 2,  "T_FIELD",      "Extracts a field of a tuple value"),
  ("swallow", 0, "T_SWALLOW",    "Swallows the entire input tuple into a single tuple value in the stack"),
  
  ("typeOf", 1, "TYPEOF", "Get the type name of a value"),
  ("totalComp", 2, "TOTALCOMP", "Compare two value in the total order of the type system"),

  ("fact", 2, "L_FACT", "Creates a fact tuple: 1st arg = tablename, 2nd arg = fact values."),
  ("createkey", 1, "L_CREATEKEY", "Create key from all passed in schema values."),
  ("lappend", 2, "L_APPEND", "Insert first element into second list, or create liust if second list is null"),
  ("concat", 2, "L_CONCAT", "Insert all elements of first list into second list"),
  ("member", 2, "L_MEMBER", "Push 1 if first arg is in second list, 0 otherwise"),
  ("subset", 2, "L_SUBSET", "Push 1 if first list is subset second list, 0 otherwise"),
  ("intersect", 2, "L_INTERSECT", "Intersects first list with second, using Lisp intersection rules"),
  ("msintersect", 2, "L_MULTISET_INTERSECT", "Intersects first list with second, using multiset intersection rules"),

  ("initvec", 1, "V_INITVEC", "Create a vector with size of the argument"),
  ("getvectoroffset", 2, "V_GETOFFSET", "Extracts a value from an vector offset"),
  ("setvectoroffset", 3, "V_SETOFFSET", "Insert first item into the offset specified in second item of vector in 3rd item"),
  ("vectorcompare", 2, "V_COMPAREVEC", "Compare 2 vectors"),

  ("initmat", 2, "M_INITMAT", "Create a matrix of arg1 by arg2"),
  ("getmatrixoffset", 3, "M_GETOFFSET", "Extracts a value from a matrix"),
  ("setmatrixoffset", 4, "M_SETOFFSET", "Insert first item into the offsets specified in second and 3rd items of matrix in 4th item"),
  ("matrixcompare", 3, "M_COMPAREMAT", "Compare 2 matrices"),
  ("matrixtranspose", 1, "M_TRANSPOSE", "Inverse a matrix"),
  ("initzero", 2, "M_INITMATZERO", "Create a zero matrix of arg1 by arg2"),   

  ("registervar", 3, "FACTOR_REGISTERVAR", "register variable with type, size and name information"),
  ("combine", 2, "COMBINE", "combine two factors"),
  ("combineall", 1, "COMBINE_ALL", "combine list of factors"),
  ("collapse", 2, "COLLAPSE", "collapse a factor to a set of variables"),
  ("weightedupdate", 3, "WEIGHTED_UPDATE", "mixes two factors"),
  ("defaultcanonicalfactor", 0, "FACTOR_DEFAULT_CANONICAL_FACTOR", "create a default canonical gaussian factor"),
  ("createcanonicalfactor", 3, "FACTOR_CREATE_CANONICAL_FACTOR", "create canonical gaussian factor"),
  ("gaussianmean", 1, "FACTOR_GAUSS_MEAN", "compute gaussian mean of a factor"),
  ("gaussiacov", 1, "FACTOR_GAUSS_COV", "comput gaussian variance of a factor"),
  ("defaulttablefactor", 0, "DEFAULT_TABLE_FACTOR", "creates an empty table factor"),
  ("createtablefactor", 2, "CREATE_TABLE_FACTOR", "creates a table factor with teh specified variables and assignments"),
  ("normalize", 1, "FACTOR_NORMALIZE", "returns a normalized version of a factor"),
  ("values", 1, "FACTOR_VALUES", "returns the values of a factor as a vector"),
  ("norm1", 2, "FACTOR_NORM1", "returns the sum of absolute differences between two factors"),
  ("norminf", 2, "FACTOR_NORM_INF", "returns the maximum of absolute differences between two factors"),
  ("emengine", 4, "EM_ENGINE", "initializes the expectation maximization engine"),
  ("eminit", 1, "EM_INIT", "initializes the estimate gaussian mixture factor"),
  ("emupdate", 2, "EM_UPDATE", "performs one EM update"),

  ("not",1, 	"NOT",          "Boolean negation"),
  ("and",2,	"AND",          "Boolean AND"),
  ("or",2,	"OR",           "Boolean inclusive-OR"),
  ("rand",0,	"RAND",         "A random Int32"),
  ("coin",1,	"COIN",         "A coin flip biased to 1 with probability given as the argument"),
  ("sampleexponential", 1, "SAMPLE_EXPONENTIAL", "Draws a sample from an exponential distribution with the given parameter."),
  ("initlist",  0,"L_INIT",       "Initialize a list."),
  ("cons",      2,"L_CONS",       "Take first aregument and prepend it to the second list argument."),
  ("car",       1,"L_CAR",        "Return the first element of the passed in list."),
  ("cdr",       1,"L_CDR",        "Return a list with all but the first element of the passed in list."),
  ("contains",  2,"L_CONTAINS",   "Check if the second arguement is contained in the first list argument."),
  ("removeLast",1,"L_REMOVELAST", "Remove last item in list"),
  ("last",      1,"L_LAST",       "Get last item in list"),
  ("size",      1,"L_SIZE",       "Numbers of items in list"),

  (">>",2,	"ASR",          "Arithmetic shift right"),
  ("<<",2,	"ASL",          "Arithmetic shift left"),
  ("&",2,       "BIT_AND",      "Bitwise AND"),
  ("|",2,       "BIT_OR",       "Bitwise inclusive-OR"),
  ("^",2,       "BIT_XOR",      "Bitwise exclusive-OR"),
  ("~",1,       "BIT_NOT",      "1's complement"),
  ("%",2,       "MOD",          "Integer modulus"),
  ("|||",2,       "APPEND",      "Append operation"),

  ("<time",2,      "TIME_LT",       "Time less-than comparison"),
  ("<=time",2,     "TIME_LTE",      "Time less-than-or-eq comparison"),
  (">time",2,      "TIME_GT",       "Time greater-than comparison"),
  (">=time",2,     "TIME_GTE",      "Time greater-than-or-eq comparison"),
  ("==time",2,     "TIME_EQ",       "Time compare equality"),
  ("-time",2,      "TIME_MINUS",    "Time subtraction"),
  ("now",0,        "TIME_NOW",      "The current time token"),

  ("<time_duration",2,      "TIME_DURATION_LT",       "Time_Duration less-than comparison"),
  ("<=time_duration",2,     "TIME_DURATION_LTE",      "Time_Duration less-than-or-eq comparison"),
  (">time_duration",2,      "TIME_DURATION_GT",       "Time_Duration greater-than comparison"),
  (">=time_duration",2,     "TIME_DURATION_GTE",      "Time_Duration greater-than-or-eq comparison"),
  ("==time_duration",2,     "TIME_DURATION_EQ",       "Time_Duration compare equality"),
  ("+time_duration",2,      "TIME_DURATION_PLUS",     "Time_Duration addition"),
  ("-time_duration",2,      "TIME_DURATION_MINUS",    "Time_Duration subtraction"),

  ("--",1,      "MINUSMINUS",   "decrement by one"),
  ("++",1,      "PLUSPLUS",     "increment by one"),
  ("()",3,	 "INOO",       "Interval open-open containment"),
  ("(]",3,	 "INOC",       "Interval open-closed containment"),
  ("[)",3,	 "INCO",       "Interval closed-open containment"),
  ("[]",3,	 "INCC",       "Interval closed-closed containment"),

  ("strcat",2,	"STR_CAT",      "String concatenation"),
  ("strlen",1,	"STR_LEN",      "String length"),
  ("upper",1,	"STR_UPPER",    "Convert string to upper case"),
  ("lower",1,	"STR_LOWER",    "Convert string to lower case"),
  ("substr",3,	"STR_SUBSTR",   "Extract substring"),
  ("match",2,	"STR_MATCH",    "Perl regular expression matching"),
  ("tostr",1,   "STR_CONV",     "Convert to a string (not ->s)"),
  ("strreplace",3,   "STR_REPLACE", "Replace string"),
  ("strfind", 2,"STR_FIND",     "Find the first string into the second"),
  ("strtok", 2, "STR_TOK",      "Extracts tokens from a string"),
  ("status",     2, "O_STATUS", "Extract the overlog program status of a program attribute."),
  ("selectivity",3, "O_SELECT", "Compute the selectivity of a join."),
  ("rangeAM",    2, "O_RANGEAM","Determine if a range access method can be used."),
  ("filter",     2, "O_FILTER", "Determine if a filter can be applied to the schema."),
  ("tovar",1,   "A_TO_VAR",   "Convert attribute to a variable."),
  ("uniqueSchema",1,   "A_UNIQUE_SCHEMA", "Convert schema into a list of unique variables."),
  ("istheta",1,   "ISTHETA",   "Test the boolean predicate for theta."),
  ("getattr",2, "L_GET_ATTR",   "Extract attribute from list."),
  ("mktype",2, "T_MK_TYPE",   "Make a Type Tuple."),
  ("mkbool",3, "T_MK_BOOL",   "Make a Bool Tuple."),
  ("mkmath",3, "T_MK_MATH",   "Make a Math Tuple."),
  ("mkagg",2, "T_MK_AGG",   "Make an Aggregation Tuple."),
  ("posattr",2, "L_POS_ATTR",   "Get the attribute position from list."),
  ("aggattr",1, "L_AGG_ATTR",   "Get the aggregation position from list."),
  ("groupbyattr",1, "L_GROUPBY_ATTR",   "Get the groupby attributes from schema."),
  ("flatten",1, "L_FLATTEN",   "Flatten all arguments in the schema to be variables."),
  ("indexMatch",3, "L_INDEXMATCH", "Tests index key according to join operation."),
  ("castassign",3, "L_CASTASSIGN", "Substitute a selection predicate for an assignment."),
  ("variables",1, "L_VARIABLES", "Extracts all variables from a math or boolean expression."),
  ("merge",2,   "L_MERGE",      "Merge two argument lists according join operation."),
  ("removevar",2,   "L_REMOVEVAR", "removes any instance of the first arg in the list."),
  ("equivalent",2,"L_EQUIVALENT","Checks if the two plans are equivalent."),
  ("prefix",2,   "L_PREFIX",      "Is the first list a prefix of the second?"),
  ("sortattr",4,   "L_SORTATTR", "REturn sort attributes ."),
  ("project",2,   "L_PROJECT",  "Project the schema (arg 2) onto the fields in arg 1."),
  ("adornment",2,   "L_ADORNMENT",  "Determine the adornment from bound variables and schema."),
  ("assignschema",2, "L_ASSIGNSCHEMA", "Determine output schema of assignment operation."),
  ("verify",6,   "VERIFY",      "Decrypt the first argument decrypted using the second key."),
  ("gen",3,   "GEN",      "encrypt the first argument using the second encryption algorithm and the third key."),
  ("mod",1,   "SETMOD",      "Return the size of the passed set or list."),
  ("empty",0,   "EMPTY",      "Return a new empty set."),
  ("initSet",1,   "INITSET",      "Return a new set with the element passed."),

  ("serialize",2,   "SERIALIZE",   "serialize the graph rooted at the given tuple"),
  ("deserialize",2,   "DESERIALIZE",   "deserialize the graph using the given buffer and root tuple name"),
  ("createVersion",0,   "CREATEVER",   "create a new version"),
  ("createLocSpec",0,   "CREATELOCSPEC",   "create a new locspec"),
  ("isLocSpec",1,   "IS_LOCSPEC",   "returns 1 if the passed arg is a loc spec, 0 otherwise"),
  ("getCert",1,   "GET_CERT",   "returns the cert from a strong locspec or version"),
  ("isSays",1,   "IS_SAYS",   "returns the cert from a strong locspec or version"),
  ("loadKeyFile",1,   "LOADKEYFILE",      "Load a encryption key. from the filename "),
  ("initMask",1,   "INITMASK",      "Initialize the mask using a list of fields from the head functor"),
  ("combineMask",2,   "COMBINEMASK",      "Combine a mask (list) with another mask"),
  ("getMask",1,   "GETMASK",      "Calculate the mask from an expression"),
  ("mask",3,   "MASK",      "Apply the mask onto the scheme to get the masked scheme"),
  ("quit",0, "QUIT", "Exits P2"),
  ("timerRestart", 0, "TIMER_RESTART", "Resets the timer to 0"),
  ("timerElapsed", 0, "TIMER_ELAPSED", "Returns the number seconds elapsed")
  ]:  emit_opcode(op, ar, va, desc)

for op, ar, va, desc in [
  ("neg",1,     "NEG",          "negation"),
  ("+",2,       "PLUS",         "addition"),
  ("-",2,       "MINUS",        "subtraction"),
  ("*",2,       "MUL",          "multiplication"),
  ("/",2,       "DIV",          "division"),
  ("==",2,      "EQ",           "Compare equality"), 
  ("<",2,       "LT",           "less-than comparison"),
  ("<=",2,      "LTE",          "less-than-or-eq comparison"),
  (">",2,       "GT",           "greater-than comparison"),
  (">=",2,      "GTE",          "greater-than-or-eq comparison")
  ]: 
  emit_opcode(op, ar, va, desc + " for polymorphic types.")

for op, ar, va, desc in [
  ("abs",1,     "INT_ABS",      "Absolute value"),
  ("floor",1,   "DBL_FLOOR",    "Next lowest integer"),
  ("ceil",1,    "DBL_CEIL",     "Next highest integer"),
  ("exp",1,	"EXP",          "Natural exponentiation"),
  ("pow",1,     "POW",          "Exponentiation"),
  ("ln",1,	"LN",           "Natural logarithm"),
  ("drand48",0,	"DRAND48",      "Generate a random number")
  ]:
  emit_opcode(op, ar, va, desc)



for i in [ "i64", "dbl", "str", "time", "time_duration", "id" ]:
  emit_opcode("->"+i, 1, "CONV_" + i.upper(), "Convert to type "+i)

warning="""

/*
 * DO NOT EDIT THIS FILE.
 *
 * It is generated by %s
 *
 */
""" % sys.argv[0]


f = open("pel_opcode_decls.gen.h","w+")
f.write(warning)
map(lambda (n,o,a,v,d): f.write("  static const u_int32_t OP_%s = %d;\n" % (v, n)),
    decls)
f.write('  static const uint32_t NUM_OPCODES= %d;\n' % curop)

# f.write('private:\n')
map(lambda (n,o,a,v,d): f.write("  void op_%s(u_int32_t inst);\n" % v ),decls)
f.close()


f = open("pel_opcode_defns.gen.h","w+")
f.write(warning)
f.write('static JumpTableEnt_t jump_table[] = {\n')
f.write(string.join(map(lambda (n,o,a,v,d): '  {"%s",\t%d, \t&Pel_VM::op_%s}' % (o,a,v), decls),
                    ',\n'))
f.write('\n};\n')
f.write('#define DEF_OP(_name) void Pel_VM::op_##_name(u_int32_t inst)\n')
f.close()

f = open("pel_opcode_tokens.gen.h","w+")
f.write(warning)
f.write(string.join(map(lambda (n,o,a,v,d): '  {"%s", \tPel_VM::OP_%s}' % (o,v), decls),
                    ',\n'))
f.write('\n')
f.close()

f = open("pel_opcode_descriptions.gen.txt","w+")
f.write(warning)
f.write("mnemonic arity\tdescription\n");
f.write(string.join(map(lambda (n,o,a,v,d): '%s\t%s\t%s' % (o,a,d), decls),
                    ',\n'))
f.write('\n')
f.close()
