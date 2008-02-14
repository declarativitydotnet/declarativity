/*
 * Copyright (c) 2005 Intel Corporation. All rights reserved.
 *
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 * DESCRIPTION: System table definitions and relationships. All definitions
 *              are given by macros, which are redifined in tableManager.C
 *              to create the appropriate table definitions and relationships.
 */

#ifndef SYS_TABLE_GLOBALS
#define SYS_TABLE_GLOBALS
  /** Entity table names */
  #define NODEID            "sys::nodeId"
  #define ARGUMENT          "sys::argument"
  #define PROGRAM           "sys::program"
  #define PERROR            "sys::error"
  #define GLOBAL_EVENT      "sys::globalEvent"
  #define SIDE_AFFECT       "sys::sideAffect"
  #define REWRITE           "sys::rewrite"
  #define ATTRIBUTE         "sys::attribute"
  #define TABLE             "sys::table"
  #define REF               "sys::ref"
  #define TABLESTATS        "sys::tableStats"
  #define INDEX             "sys::index"
  #define FACT              "sys::fact"
  #define SAYS              "sys::says"
  #define NEW               "sys::new"
  #define STAGE             "sys::stage"
  #define RULE              "sys::rule"
  #define FUNCTOR           "sys::predicate"
  #define ASSIGN            "sys::assign"
  #define SELECT            "sys::select"
  #define PROJECTION        "sys::projection"
  #define WATCH             "sys::watch"
  #define FUNCTION          "sys::function"
  #define ACCESS_METHOD     "sys::accessMethod"
  #define COMPILE_STATUS    "sys::compileStatus"

  #define PROGRAM_STREAM  "sys::program_add"
  #define ERROR_STREAM    "sys::error_add"

  /** The following are expression types */
  #define AGG          "Aggregation"
  #define BOOL         "Bool"
  #define RANGE        "Range"
  #define SETS         "Set"
  #define MATH         "Math"
  #define VAL          "AtomicValue"
  #define VAR          "Variable"  
  #define NEWLOCSPEC   "NewLocSpec"
  #define LOC          "Location"
  #define VEC          "Vector"
  #define MAT          "Matrix"
  #define LOCSPECTUPLE "LocSpec"
  #define VERSIONTUPLE "Version"
  #define HASH_INDEX   "Hash"
  #define BTREE_INDEX   "BTree"

/**
 * Every schema has a table name and tuple identifier.
 * This enum is a shortcut to extract the position of
 * these values. The CommonTable::Manager class provides
 * a more general way of extract attribute field positions. */
enum SchemaGlobal{TNAME=0, NODE_ID, TUPLE_ID};
enum VariableTuple{CONTENTPOS=2}; //pos of content in variable tuple
enum GLOBALUNIQUE{NAME = 0, NODE, ID, STRONGPOS, HASHPOS};
enum RefType{WEAKLINK = 0, STRONGLINK, WEAKSAYS, STRONGSAYS, ROOT, ROOTSAYS};
enum NewPos{LOCPOS = 1, OPAQUEPOS, HINTPOS};
enum TypeCode{CREATESAYS = 0, COPYSAYS};
enum SaysTuple{LOC_POS = 1, PPOS, RPOS, KPOS, VPOS, PROOFPOS};

#endif

#ifndef SCHEMA
#define SCHEMA(name, pos)
#endif

/** A TABLEDEF defines an entity relation. A table will be 
  * created and given the name indicated by the first formal
  * and a primary key indicated by the second formal.
  */
#ifndef TABLEDEF
#define TABLEDEF(name, keys, schema)
#endif
// TABLE AND INDEX MUST APPEAR FIRST!
TABLEDEF(TABLE, CommonTable::theKey(CommonTable::KEY3), \
         SCHEMA("TNAME", 0) SCHEMA("LOCATION", 1) SCHEMA("TID", 2) \
         SCHEMA("TABLENAME", 3) SCHEMA("LIFETIME", 4) SCHEMA("SIZE", 5) \
         SCHEMA("KEY", 6) SCHEMA("CARD", 7) SCHEMA("SORT", 8) SCHEMA("PID", 9))

TABLEDEF(INDEX, CommonTable::theKey(CommonTable::KEY34), \
         SCHEMA("TNAME", 0) SCHEMA("LOCATION", 1) SCHEMA("IID", 2) \
         SCHEMA("TABLENAME", 3) SCHEMA("KEY", 4) SCHEMA("TYPE", 5) \
         SCHEMA("SELECTIVITY", 6))

TABLEDEF(ATTRIBUTE, CommonTable::theKey(CommonTable::KEY34), \
         SCHEMA("TNAME", 0) SCHEMA("LOCATION", 1) SCHEMA("ATTRID", 2) \
         SCHEMA("TABLENAME", 3) SCHEMA("ATTRNAME", 4) SCHEMA("POSITION", 5))

TABLEDEF(REF, CommonTable::theKey(CommonTable::KEY2), \
         SCHEMA("TNAME", 0) SCHEMA("LOCATION", 1) SCHEMA("REFID", 2) \
	 SCHEMA("PID", 3) SCHEMA("FROM", 4) SCHEMA("TO", 5) \
	 SCHEMA("LOCSPECFIELD", 6) SCHEMA("REFTYPE", 7))

TABLEDEF(SAYS, CommonTable::theKey(CommonTable::KEY2), \
         SCHEMA("TNAME", 0) SCHEMA("LOCATION", 1) SCHEMA("SAYSID", 2) \
	 SCHEMA("FID", 3) SCHEMA("ATTRIBUTES", 4))

TABLEDEF(NEW, CommonTable::theKey(CommonTable::KEY2), \
         SCHEMA("TNAME", 0) SCHEMA("LOCATION", 1) SCHEMA("NEWID", 2) \
	 SCHEMA("FID", 3) SCHEMA("EVENTLOC", 4) SCHEMA("OPAQUE", 5) \
	 SCHEMA("SYSTEMINFO", 6))

TABLEDEF(NODEID, CommonTable::theKey(CommonTable::KEY3), \
         SCHEMA("TNAME", 0) SCHEMA("LOCATION", 1) SCHEMA("NID", 2) \
         SCHEMA("ADDRESS", 3))

TABLEDEF(ARGUMENT, CommonTable::theKey(CommonTable::KEY2), \
         SCHEMA("TNAME", 0) SCHEMA("LOCATION", 1) SCHEMA("KEY", 2) \
         SCHEMA("VALUE", 3))

TABLEDEF(GLOBAL_EVENT, CommonTable::theKey(CommonTable::KEY45), \
         SCHEMA("TNAME", 0) SCHEMA("LOCATION", 1) SCHEMA("EID", 2) \
         SCHEMA("PID", 3) SCHEMA("NAME", 4) SCHEMA("TYPE", 5) \
         SCHEMA("TRIGGER", 6) SCHEMA("SCHEMA", 7))

TABLEDEF(SIDE_AFFECT, CommonTable::theKey(CommonTable::KEY45), \
         SCHEMA("TNAME", 0) SCHEMA("LOCATION", 1) SCHEMA("SID", 2) \
         SCHEMA("PID", 3) SCHEMA("NAME", 4) SCHEMA("TYPE", 5) \
         SCHEMA("TRIGGER", 6) SCHEMA("SCHEMA", 7))
         
TABLEDEF(PROGRAM, CommonTable::theKey(CommonTable::KEY2), \
         SCHEMA("TNAME", 0) SCHEMA("LOCATION", 1) SCHEMA("PID", 2) \
         SCHEMA("NAME", 3) SCHEMA(REWRITE, 4) SCHEMA("STATUS", 5) \
         SCHEMA("TEXT", 6) SCHEMA("RESULT_MESSAGE", 7) SCHEMA("P2DL", 8) \
         SCHEMA("SOURCE", 9))

TABLEDEF(PERROR, CommonTable::theKey(CommonTable::KEY25), \
         SCHEMA("TNAME", 0) SCHEMA("LOCATION", 1) SCHEMA("PID", 2) \
         SCHEMA("PNAME", 3) SCHEMA("NODEID", 4) SCHEMA("CODE", 5) \
         SCHEMA("DESC", 6))

TABLEDEF(REWRITE, CommonTable::theKey(CommonTable::KEY34),
         SCHEMA("TNAME", 0) SCHEMA("LOCATION", 1) SCHEMA("RID", 2) \
         SCHEMA("INPUT", 3) SCHEMA("OUTPUT", 4))

TABLEDEF(WATCH, CommonTable::theKey(CommonTable::KEY45), \
         SCHEMA("TNAME", 0) SCHEMA("LOCATION", 1) SCHEMA("WID", 2) \
         SCHEMA("PID", 3) SCHEMA("NAME", 4) SCHEMA("MOD", 5))

TABLEDEF(STAGE, CommonTable::theKey(CommonTable::KEY2), \
         SCHEMA("TNAME", 0) SCHEMA("LOCATION", 1) SCHEMA("SID", 2) \
         SCHEMA("PID", 3) SCHEMA("PROCESSOR", 4) SCHEMA("INPUT", 5) \
         SCHEMA("OUTPUT", 6))

TABLEDEF(FACT, CommonTable::theKey(CommonTable::KEY2), \
         SCHEMA("TNAME", 0) SCHEMA("LOCATION", 1) SCHEMA("FID", 2) \
         SCHEMA("PID", 3) SCHEMA("TABLENAME", 4) SCHEMA("TUPLE", 5))

TABLEDEF(RULE, CommonTable::theKey(CommonTable::KEY2), \
         SCHEMA("TNAME", 0) SCHEMA("LOCATION", 1) SCHEMA("RID", 2) \
         SCHEMA("PID", 3) SCHEMA("NAME", 4) SCHEMA("HEAD_FID", 5) \
         SCHEMA("P2DL", 6) SCHEMA("DELETE", 7) SCHEMA("TERM_COUNT", 8) \
	 SCHEMA("NEW", 9))

TABLEDEF(FUNCTOR, CommonTable::theKey(CommonTable::KEY2), \
         SCHEMA("TNAME", 0) SCHEMA("LOCATION", 1) SCHEMA("FID", 2) \
         SCHEMA("RID", 3) SCHEMA("NOTIN", 4) SCHEMA("NAME", 5) SCHEMA("TID", 6) \
         SCHEMA("ECA", 7) SCHEMA("ATTRIBUTES", 8) SCHEMA("POSITION", 9) \
         SCHEMA("AM", 10) SCHEMA("NEW", 11)) 

TABLEDEF(ASSIGN, CommonTable::theKey(CommonTable::KEY2), \
         SCHEMA("TNAME", 0) SCHEMA("LOCATION", 1) SCHEMA("AID", 2) \
         SCHEMA("RID", 3) SCHEMA("VAR", 4) SCHEMA("VALUE", 5) SCHEMA("POSITION", 6))

TABLEDEF(SELECT, CommonTable::theKey(CommonTable::KEY2), \
         SCHEMA("TNAME", 0) SCHEMA("LOCATION", 1) SCHEMA("SID", 2) \
         SCHEMA("RID", 3) SCHEMA("BOOL", 4) SCHEMA("POSITION", 5) \
         SCHEMA("AM", 6))

TABLEDEF(PROJECTION, CommonTable::theKey(CommonTable::KEY2), \
         SCHEMA("TNAME", 0) SCHEMA("LOCATION", 1) SCHEMA("PID", 2) \
         SCHEMA("RID", 3) SCHEMA("ATTRIBUTES", 4) SCHEMA("POSITION", 5))

TABLEDEF(FUNCTION, CommonTable::theKey(CommonTable::KEY2), \
         SCHEMA("TNAME", 0) SCHEMA("LOCATION", 1) SCHEMA("FID", 2) \
         SCHEMA("NAME", 3) SCHEMA("NUMARGS", 4) SCHEMA("PEL", 5))

TABLEDEF(COMPILE_STATUS, CommonTable::theKey(CommonTable::KEY1), \
         SCHEMA("TNAME", 0) SCHEMA("LOCATION", 1) SCHEMA("STATUS", 2))

#undef TABLEDEF
#undef SCHEMA

#ifndef FOREIGN_KEY
#define FOREIGN_KEY(table1, foreignKey, table2)
#endif
FOREIGN_KEY(FACT,              CommonTable::theKey(CommonTable::KEY3), PROGRAM)
FOREIGN_KEY(REF,               CommonTable::theKey(CommonTable::KEY3), PROGRAM)
FOREIGN_KEY(FACT,              CommonTable::theKey(CommonTable::KEY4), TABLE)
FOREIGN_KEY(RULE,              CommonTable::theKey(CommonTable::KEY3), PROGRAM)
FOREIGN_KEY(INDEX,             CommonTable::theKey(CommonTable::KEY3), TABLE)
FOREIGN_KEY(SAYS,              CommonTable::theKey(CommonTable::KEY3), FUNCTOR)
FOREIGN_KEY(NEW,               CommonTable::theKey(CommonTable::KEY3), FUNCTOR)
FOREIGN_KEY(FUNCTOR,           CommonTable::theKey(CommonTable::KEY3), RULE)
FOREIGN_KEY(ASSIGN,            CommonTable::theKey(CommonTable::KEY3), RULE)
FOREIGN_KEY(SELECT,            CommonTable::theKey(CommonTable::KEY3), RULE)
#undef FOREIGN_KEY

#ifndef SECONDARY_INDEX
#define SECONDARY_INDEX(table, key)
#endif
SECONDARY_INDEX(REF,               CommonTable::theKey(CommonTable::KEY3))
SECONDARY_INDEX(REF,               CommonTable::theKey(CommonTable::KEY4))
SECONDARY_INDEX(SAYS,              CommonTable::theKey(CommonTable::KEY3))
SECONDARY_INDEX(NEW,               CommonTable::theKey(CommonTable::KEY3))
SECONDARY_INDEX(FUNCTION,          CommonTable::theKey(CommonTable::KEY3))
SECONDARY_INDEX(FUNCTION,          CommonTable::theKey(CommonTable::KEY34))
SECONDARY_INDEX(RULE,              CommonTable::theKey(CommonTable::KEY3))
SECONDARY_INDEX(FUNCTOR,           CommonTable::theKey(CommonTable::KEY3))
SECONDARY_INDEX(FUNCTOR,           CommonTable::theKey(CommonTable::KEY5))
SECONDARY_INDEX(FUNCTOR,           CommonTable::theKey(CommonTable::KEY39))
SECONDARY_INDEX(ASSIGN,            CommonTable::theKey(CommonTable::KEY3))
SECONDARY_INDEX(ASSIGN,            CommonTable::theKey(CommonTable::KEY36))
SECONDARY_INDEX(SELECT,            CommonTable::theKey(CommonTable::KEY3))
SECONDARY_INDEX(SELECT,            CommonTable::theKey(CommonTable::KEY35))
SECONDARY_INDEX(FACT,              CommonTable::theKey(CommonTable::KEY3))
SECONDARY_INDEX(STAGE,             CommonTable::theKey(CommonTable::KEY3))
SECONDARY_INDEX(REWRITE,           CommonTable::theKey(CommonTable::KEY3))
SECONDARY_INDEX(REWRITE,           CommonTable::theKey(CommonTable::KEY4))
SECONDARY_INDEX(WATCH,             CommonTable::theKey(CommonTable::KEY4))
SECONDARY_INDEX(PROJECTION,        CommonTable::theKey(CommonTable::KEY35))
SECONDARY_INDEX(TABLE,             CommonTable::theKey(CommonTable::KEY9))
#undef SECONDARY_INDEX

#ifndef FUNCTIONDEF
#define FUNCTIONDEF(name, numargs, pel)
#endif
FUNCTIONDEF("f_coinFlip",    1, "coin")
FUNCTIONDEF("f_ifelse",      3, "ifelse")
FUNCTIONDEF("f_rand",        0, "rand")
FUNCTIONDEF("f_drand",       0, "drand48")
FUNCTIONDEF("f_now",         0, "now")
FUNCTIONDEF("f_sha1",        1, "sha1")
FUNCTIONDEF("f_match",       2, "match")
/*FUNCTIONDEF("f_append",      1, "lappend") // Append with null*/
FUNCTIONDEF("f_append",      2, "lappend") // Append
FUNCTIONDEF("f_member",      2, "member")
FUNCTIONDEF("f_concat",      2, "concat")
FUNCTIONDEF("f_intersect",   2, "intersect")
FUNCTIONDEF("f_msintersect", 2, "msintersect")
FUNCTIONDEF("f_initlist",    0, "initlist")
FUNCTIONDEF("f_cons",        2, "cons")
FUNCTIONDEF("f_car",         1, "car")
FUNCTIONDEF("f_cdr",         1, "cdr")
FUNCTIONDEF("f_contains",    2, "contains")
FUNCTIONDEF("f_posattr",     2, "posattr")
FUNCTIONDEF("f_removeLast",  1, "removeLast")
FUNCTIONDEF("f_last",        1, "last")
FUNCTIONDEF("f_size",        1, "size")
FUNCTIONDEF("f_equivalent",  2, "equivalent")
FUNCTIONDEF("f_moffset",      2, "getmatrixoffset")    // For matrix
FUNCTIONDEF("f_voffset",      3, "getvectoroffset")    // For vector
FUNCTIONDEF("f_setVOffset",   3, "setvectoroffset") // For vector
FUNCTIONDEF("f_setMOffset",   4, "setmatrixoffset") // For matrix
FUNCTIONDEF("f_compare",     2, "compare")
FUNCTIONDEF("f_max",         2, "max")
FUNCTIONDEF("f_min",         2, "min")
FUNCTIONDEF("f_getattr",     2, "getattr")
FUNCTIONDEF("f_mktype",      2, "mktype")
FUNCTIONDEF("f_mkbool",      3, "mkbool")
FUNCTIONDEF("f_posattr",     2, "posattr")
FUNCTIONDEF("f_aggattr",     1, "aggattr")
FUNCTIONDEF("f_groupbyattr", 1, "groupbyattr")
FUNCTIONDEF("f_flatten",     1, "flatten")
FUNCTIONDEF("f_merge",       2, "merge")
FUNCTIONDEF("f_sortattr",    4, "sortattr")
FUNCTIONDEF("f_prefix",      2, "prefix")
FUNCTIONDEF("f_adornment",   2, "adornment")
FUNCTIONDEF("f_project",     2, "project")
FUNCTIONDEF("f_fact",        2, "fact")
FUNCTIONDEF("f_assignschema",2, "assignschema")
FUNCTIONDEF("f_idgen",       0, "idgen")
FUNCTIONDEF("f_tostr",       1, "tostr")
FUNCTIONDEF("f_strReplace",  3, "strreplace")
FUNCTIONDEF("f_strfind",     2, "strfind")
FUNCTIONDEF("f_tovar",       1, "tovar")
FUNCTIONDEF("f_uniqueSchema",1, "uniqueSchema")
FUNCTIONDEF("f_istheta",     1, "istheta")
FUNCTIONDEF("f_subset",      2, "subset")

FUNCTIONDEF("f_status",       2, "status")
FUNCTIONDEF("f_selectivity",  3, "selectivity")
FUNCTIONDEF("f_rangeAM",      2, "rangeAM")
FUNCTIONDEF("f_filter",       2, "filter")
FUNCTIONDEF("f_verify",       6, "verify")
FUNCTIONDEF("f_gen",          3, "gen")
FUNCTIONDEF("f_mod",          1, "mod")
FUNCTIONDEF("f_empty",        0, "empty")
FUNCTIONDEF("f_initSet",      1, "initSet")
FUNCTIONDEF("f_serialize",    2, "serialize")
FUNCTIONDEF("f_deserialize",  2, "deserialize")
FUNCTIONDEF("f_createVersion",0, "createVersion")
FUNCTIONDEF("f_createLocSpec",0, "createLocSpec")
FUNCTIONDEF("f_isLocSpec",    1, "isLocSpec")
FUNCTIONDEF("f_getCer",      1, "getCert")
FUNCTIONDEF("f_isSays",       1, "isSays")
FUNCTIONDEF("f_loadKeyFile",  1, "loadKeyFile")
FUNCTIONDEF("f_initMask",     1, "initMask")
FUNCTIONDEF("f_combineMask",  2, "combineMask")
FUNCTIONDEF("f_getMask",      1, "getMask")
FUNCTIONDEF("f_mask",         3, "mask")
FUNCTIONDEF("f_indexMatch",   3, "indexMatch")
FUNCTIONDEF("f_castassign",   3, "castassign")
FUNCTIONDEF("f_variables",   1, "variables")
FUNCTIONDEF("f_createKey",   1, "createkey")
FUNCTIONDEF("f_matrixTranspose",     1, "matrixtranspose")
FUNCTIONDEF("f_initZeroMatrix",  2, "initzero")
FUNCTIONDEF("f_registerVariable",      3, "registervar")
FUNCTIONDEF("f_combine",         2, "combine")
FUNCTIONDEF("f_combineAll",   1, "combineall")
FUNCTIONDEF("f_collapse",   2, "collapse")
FUNCTIONDEF("f_defaultCanonicalFactor",   0, "defaultcanonicalfactor")
FUNCTIONDEF("f_createCanonicalFactor",   3, "createcanonicalfactor")
FUNCTIONDEF("f_defaultTableFactor", 0, "defaulttablefactor")
FUNCTIONDEF("f_createTableFactor", 2, "createtablefactor")
FUNCTIONDEF("f_gaussianMean",   1, "gaussianmean")
FUNCTIONDEF("f_gaussianCov",   1, "gaussiacov")
#undef FUNCTIONDEF
