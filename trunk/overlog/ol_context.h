// -*- c-basic-offset: 2; related-file-name: "ol_context.C" -*-
/*
 * @(#)$Id$
 *
 * Copyright (c) 2005 Intel Corporation. All rights reserved.
 *
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 * DESCRIPTION: Parsing environment for Overlog (the P2 dialect of datalog)
 *
 */

#ifndef __OL_PARSEENV_H__
#define __OL_PARSEENV_H__

#include <vector>
#include <map>
#include <set>
#include "value.h"
#include "tuple.h"

#include "parser_util.h"

class OL_Context {

public:

  struct Error {
    int  line_num;      // Line number
    str  msg;           // What?
    Error(int l, str m) : line_num(l), msg(m) {};
  };
  
  struct Rule {
    Rule(str r, Parse_Functor *h, bool d) 
      : ruleID(r), head(h), deleteFlag(d) {};

    str toString();

    str           ruleID;
    Parse_Functor *head;
    bool          deleteFlag;
    std::vector<Parse_Term*> terms; 	// List of terms in the left hand side.
  };

  // create a ECA_Rule struct

  /* The meta-data of the table */
  struct TableInfo {
    str toString();

    str tableName;
    int timeout;
    int size;
    std::vector<int> primaryKeys;
  };
  
  struct WatchDef {
    WatchDef(Parse_Expr *e) : watch(e->v) {};
    str toString() { return "watch( " << watch->toString() << " )"; };

    ValuePtr watch;
  };

  /*******************************************************************/
  OL_Context();
  ~OL_Context();

  //
  // Parsing programs
  //
  void parse_string( const char *prog );
  void parse_stream( std::istream *str );

  /////
  // 
  // Parser interface: functions called from the parser to create
  // state. 
  //
  //////

  //
  // Add a new rule to the system
  void rule( Parse_Term *lhs, Parse_TermList *rhs, bool deleteFlag, Parse_Expr *n=NULL );
  void aggRule( Parse_Term *lhs, Parse_AggTerm *rhs, bool deleteFlag, Parse_Expr *n=NULL );

  // Materialize a table
  void table( Parse_Expr *n, Parse_Expr *t, Parse_Expr *s, Parse_ExprList *k=NULL );

  void query( Parse_Term *term);

  void fact( Parse_Term *t );

  void watch( Parse_Expr *t );

  void error(str msg);

  OL_Lexer *lexer;

  str toString();
  
  typedef std::vector<Rule*>                      RuleList;
  typedef std::map<str, OL_Context::TableInfo *>  TableInfoMap;
  typedef std::vector<OL_Context::Error *> ErrorList;


private:
  TableInfoMap*      tables;
  RuleList*          rules;
  std::set<str>      watchTables;
  std::vector<TuplePtr> facts;
  Parse_Functor* singleQuery;

public: 
  ErrorList          errors;
  RuleList*          getRules()       { return rules; };
  TableInfoMap*      getTableInfos()  { return tables;   };
  std::set<str>      getWatchTables() { return watchTables; };
  std::vector<TuplePtr> getFacts()       { return facts; };
  
  
};

extern int ol_parser_parse( OL_Context *env );

#endif /* __OL_PARSEENV_H_ */
