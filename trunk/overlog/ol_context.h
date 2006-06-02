// -*- c-basic-offset: 2; related-file-name: "ol_context.C" -*-
/*
 * @(#)$Id$
 *
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
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
#include "table2.h"
#include <list>

#include "parser_util.h"

class OL_Context {

public:
  static int ruleCount;

  struct Error {
    int  line_num;      // Line number
    string  msg;           // What?
    Error(int l, string m) : line_num(l), msg(m) {};
  };
  
  struct Rule {
    Rule(string r, Parse_Functor *h, bool d) 
      : ruleID(r), head(h), deleteFlag(d), 
	ruleNum(OL_Context::ruleCount++) {};
    
    string toString();
    
    string ruleID;

    Parse_Functor *head;
    
    bool deleteFlag;

    std::list<Parse_Term*> terms; 	// List of terms in the left hand side.

    int ruleNum;
  };

  // create a ECA_Rule struct

  /* The meta-data of the table */
  struct TableInfo {
    string toString();

    string tableName;

    boost::posix_time::time_duration timeout;

    size_t size;

    Table2::Key primaryKeys;
  };
  
  struct WatchDef {
    WatchDef(Parse_Expr *e) : watch(e->v) {};
    string toString() { return "watch( " + watch->toString() + " )"; };

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

  void fact( Parse_Expr *name, Parse_ExprList *args );

  void watch( Parse_Expr *t );

  void traceTuple( Parse_Expr *t );

  void error(string msg);

  OL_Lexer *lexer;

  string toString();
  
  typedef std::vector<Rule*> RuleList;

  typedef std::map<string, OL_Context::TableInfo *>  TableInfoMap;

  typedef std::vector<OL_Context::Error *> ErrorList;


private:
  TableInfoMap*      tables;
  RuleList*          rules;
  std::set<string>      watchTables;
  std::vector<TuplePtr> facts;
  Parse_Functor* singleQuery;
  std::set<string> tuplesToTrace;

public: 
  ErrorList          errors;
  RuleList*          getRules()       { return rules; };
  TableInfoMap*      getTableInfos()  { return tables;   };
  std::set<string>      getWatchTables() { return watchTables; };
  std::vector<TuplePtr> getFacts()       { return facts; };
  std::set<string> getTuplesToTrace() { return tuplesToTrace;};  
  
};

extern int ol_parser_parse( OL_Context *env );

#endif /* __OL_PARSEENV_H_ */
