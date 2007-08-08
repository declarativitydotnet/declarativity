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
  #define NODEID            "nodeId"
  #define ARGUMENT          "argument"
  #define PROGRAM           "program"
  #define GLOBAL_EVENT      "globalEvent"
  #define REWRITE           "rewrite"
  #define ATTRIBUTE         "attribute"
  #define TABLE             "table"
  #define TABLESTATS        "tableStats"
  #define INDEX             "index"
  #define FACT              "fact"
  #define STAGE             "stage"
  #define RULE              "rule"
  #define FUNCTOR           "functor"
  #define FUNCTOR_ATTRIBUTE "functorAttribute"
  #define ASSIGN            "assign"
  #define SELECT            "select"
  #define AGG_VIEW          "aggregationView"
  #define WATCH             "watch"
  #define FUNCTION          "function"
  #define ACCESS_METHOD     "accessMethod"
  #define COMPILE_STATUS    "compileStatus"

  /** The following are expression types */
  #define AGG          "Aggregation"
  #define BOOL         "Bool"
  #define RANGE        "Range"
  #define MATH         "Math"
  #define VAL          "AtomicValue"
  #define VAR          "Variable"
  #define LOC          "Location"
  #define VEC          "Vector"
  #define MAT          "Matrix"

/**
 * Every schema has a table name and tuple identifier.
 * This enum is a shortcut to extract the position of
 * these values. The CommonTable::Manager class provides
 * a more general way of extract attribute field positions. */
enum SchemaGlobal{TNAME=0, NODE_ID, TUPLE_ID};

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
         SCHEMA("KEY", 6) SCHEMA("CARD", 7))

TABLEDEF(INDEX, CommonTable::theKey(CommonTable::KEY34), \
         SCHEMA("TNAME", 0) SCHEMA("LOCATION", 1) SCHEMA("IID", 2) \
         SCHEMA("TABLENAME", 3) SCHEMA("KEY", 4) SCHEMA("TYPE", 5))

TABLEDEF(ACCESS_METHOD, CommonTable::theKey(CommonTable::KEY3), \
         SCHEMA("TNAME", 0) SCHEMA("LOCATION", 1) SCHEMA("AMID", 2) \
         SCHEMA("NAME", 3) SCHEMA("INPUTS", 4) SCHEMA("ARGUMENTS", 5))

TABLEDEF(NODEID, CommonTable::theKey(CommonTable::KEY3), \
         SCHEMA("TNAME", 0) SCHEMA("LOCATION", 1) SCHEMA("NID", 2) \
         SCHEMA("ADDRESS", 3))

TABLEDEF(ARGUMENT, CommonTable::theKey(CommonTable::KEY2), \
         SCHEMA("TNAME", 0) SCHEMA("LOCATION", 1) SCHEMA("KEY", 2) \
         SCHEMA("VALUE", 3))

TABLEDEF(GLOBAL_EVENT, CommonTable::theKey(CommonTable::KEY34), \
         SCHEMA("TNAME", 0) SCHEMA("LOCATION", 1) SCHEMA("EID", 2) \
         SCHEMA("NAME", 3) SCHEMA("TYPE", 4) SCHEMA("TRIGGER", 5))
         
TABLEDEF(ATTRIBUTE, CommonTable::theKey(CommonTable::KEY34), \
         SCHEMA("TNAME", 0) SCHEMA("LOCATION", 1) SCHEMA("ATTRID", 2) \
         SCHEMA("TABLENAME", 3) SCHEMA("ATTRNAME", 4) SCHEMA("POSITION", 5))

TABLEDEF(PROGRAM, CommonTable::theKey(CommonTable::KEY2), \
         SCHEMA("TNAME", 0) SCHEMA("LOCATION", 1) SCHEMA("PID", 2) \
         SCHEMA("NAME", 3) SCHEMA(REWRITE, 4) SCHEMA("STATUS", 5) \
         SCHEMA("TEXT", 6) SCHEMA("RESULT_MESSAGE", 7) SCHEMA("P2DL", 8))

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
         SCHEMA("P2DL", 6) SCHEMA("DELETE", 7) SCHEMA("TERM_COUNT", 8))

TABLEDEF(FUNCTOR, CommonTable::theKey(CommonTable::KEY2), \
         SCHEMA("TNAME", 0) SCHEMA("LOCATION", 1) SCHEMA("FID", 2) \
         SCHEMA("RID", 3) SCHEMA("NAME", 4) SCHEMA("TID", 5) \
         SCHEMA("ECA", 6) SCHEMA("ATTRIBUTES", 7) SCHEMA("POSITION", 8) SCHEMA("AM", 9))

TABLEDEF(ASSIGN, CommonTable::theKey(CommonTable::KEY2), \
         SCHEMA("TNAME", 0) SCHEMA("LOCATION", 1) SCHEMA("AID", 2) \
         SCHEMA("RID", 3) SCHEMA("VAR", 4) SCHEMA("VALUE", 5) SCHEMA("POSITION", 6))

TABLEDEF(AGG_VIEW, CommonTable::theKey(CommonTable::KEY2), \
         SCHEMA("TNAME", 0) SCHEMA("LOCATION", 1) SCHEMA("AVID", 2) \
         SCHEMA("RID", 3) SCHEMA("FID", 5) \
         SCHEMA("OPER", 6) SCHEMA("GROUPBY", 7) SCHEMA("AGG", 8))

TABLEDEF(SELECT, CommonTable::theKey(CommonTable::KEY2), \
         SCHEMA("TNAME", 0) SCHEMA("LOCATION", 1) SCHEMA("SID", 2) \
         SCHEMA("RID", 3) SCHEMA("BOOL", 4) SCHEMA("POSITION", 5) \
         SCHEMA("AM", 6))

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
FOREIGN_KEY(FACT,              CommonTable::theKey(CommonTable::KEY4), TABLE)
FOREIGN_KEY(RULE,              CommonTable::theKey(CommonTable::KEY3), PROGRAM)
FOREIGN_KEY(INDEX,             CommonTable::theKey(CommonTable::KEY3), TABLE)
FOREIGN_KEY(FUNCTOR,           CommonTable::theKey(CommonTable::KEY3), RULE)
FOREIGN_KEY(ASSIGN,            CommonTable::theKey(CommonTable::KEY3), RULE)
FOREIGN_KEY(SELECT,            CommonTable::theKey(CommonTable::KEY3), RULE)
#undef FOREIGN_KEY

#ifndef SECONDARY_INDEX
#define SECONDARY_INDEX(table, key)
#endif
SECONDARY_INDEX(ATTRIBUTE,         CommonTable::theKey(CommonTable::KEY3))
SECONDARY_INDEX(FUNCTION,          CommonTable::theKey(CommonTable::KEY3))
SECONDARY_INDEX(FUNCTION,          CommonTable::theKey(CommonTable::KEY34))
SECONDARY_INDEX(RULE,              CommonTable::theKey(CommonTable::KEY3))
SECONDARY_INDEX(FUNCTOR,           CommonTable::theKey(CommonTable::KEY3))
SECONDARY_INDEX(FUNCTOR,           CommonTable::theKey(CommonTable::KEY38))
SECONDARY_INDEX(ASSIGN,            CommonTable::theKey(CommonTable::KEY3))
SECONDARY_INDEX(ASSIGN,            CommonTable::theKey(CommonTable::KEY36))
SECONDARY_INDEX(SELECT,            CommonTable::theKey(CommonTable::KEY3))
SECONDARY_INDEX(SELECT,            CommonTable::theKey(CommonTable::KEY35))
SECONDARY_INDEX(FACT,              CommonTable::theKey(CommonTable::KEY3))
SECONDARY_INDEX(STAGE,              CommonTable::theKey(CommonTable::KEY3))
SECONDARY_INDEX(REWRITE,           CommonTable::theKey(CommonTable::KEY3))
SECONDARY_INDEX(REWRITE,           CommonTable::theKey(CommonTable::KEY4))
SECONDARY_INDEX(WATCH,             CommonTable::theKey(CommonTable::KEY3))
#undef SECONDARY_INDEX

#ifndef FUNCTIONDEF
#define FUNCTIONDEF(name, numargs, pel)
#endif
FUNCTIONDEF("f_coinFlip",    1, "coin")
FUNCTIONDEF("f_rand",        0, "rand")
FUNCTIONDEF("f_now",         0, "now")
FUNCTIONDEF("f_sha1",        1, "sha1")
FUNCTIONDEF("f_test",        1, "test")
FUNCTIONDEF("f_match",       2, "match")
FUNCTIONDEF("f_append",      1, "lappend") // Append with null
FUNCTIONDEF("f_append",      2, "lappend") // Append
FUNCTIONDEF("f_member",      2, "member")
FUNCTIONDEF("f_concat",      2, "concat")
FUNCTIONDEF("f_intersect",   2, "intersect")
FUNCTIONDEF("f_msintersect", 2, "msintersect")
FUNCTIONDEF("f_initlist",    2, "initlist")
FUNCTIONDEF("f_cons",        2, "cons")
FUNCTIONDEF("f_car",         1, "car")
FUNCTIONDEF("f_cdr",         1, "cdr")
FUNCTIONDEF("f_contains",    2, "contains")
FUNCTIONDEF("f_removeLast",  1, "removeLast")
FUNCTIONDEF("f_last",        1, "last")
FUNCTIONDEF("f_size",        1, "size")
FUNCTIONDEF("f_offset",      2, "offset")    // For vector
FUNCTIONDEF("f_offset",      3, "offset")    // For matrix
FUNCTIONDEF("f_setOffset",   3, "setoffset") // For vector
FUNCTIONDEF("f_setOffset",   4, "setoffset") // For matrix
FUNCTIONDEF("f_compare",     2, "compare")
FUNCTIONDEF("f_max",         2, "max")
FUNCTIONDEF("f_min",         2, "min")
FUNCTIONDEF("f_getattr",     2, "getattr")
FUNCTIONDEF("f_posattr",     2, "posattr")
FUNCTIONDEF("f_aggattr",     1, "aggattr")
FUNCTIONDEF("f_merge",       2, "merge")
FUNCTIONDEF("f_idgen",       0, "idgen")
FUNCTIONDEF("f_tostr",       1, "tostr")
FUNCTIONDEF("f_tovar",       1, "tovar")

FUNCTIONDEF("f_status",       2, "status")
FUNCTIONDEF("f_selectivity",  3, "selectivity")
FUNCTIONDEF("f_rangeAM",      2, "rangeAM")
FUNCTIONDEF("f_filter",       2, "filter")
#undef FUNCTIONDEF
