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

    uint32_t size;

    Table2::Key primaryKeys;
  };
  
  /*******************************************************************/
  OL_Context();
  ~OL_Context();

  //
  // Parsing programs
  //
  void parse_string(const char *prog);
  void parse_stream(std::istream *str);

  /////
  // 
  // Parser interface: functions called from the parser to create
  // state. 
  //
  //////

  //
  // Add a new rule to the system
  void rule(Parse_Term *lhs, Parse_TermList *rhs, bool deleteFlag, Parse_Expr *n=NULL);
  void aggRule(Parse_Term *lhs, Parse_AggTerm *rhs, bool deleteFlag, Parse_Expr *n=NULL);

  // Materialize a table
  void table(Parse_Expr *n, Parse_Expr *t, Parse_Expr *s, Parse_ExprList *k=NULL);

  void query(Parse_Term *term);

  void fact(Parse_Term *term);


  /** Register a watch fact */
  void
  watch(Parse_Expr *t,
        std::string modifiers);


  /** Register a stage fact, made of the stage name, the input tuple
      name and the output tuple name */
  void
  stage(Parse_Expr* stageName,
        Parse_Expr* inputTuple,
        Parse_Expr* outputTuple);


  void traceTuple(Parse_Expr *t);


  /** Keep track of tables to be traced */
  void
  traceTable(Parse_Expr *t);



  /** Error management */
  void
  error(string msg);


  /** Return true if I encountered errors during parsing */
  bool
  gotErrors();

  
  /** Dump any errors into the ERROR stream */
  void
  dumpErrors();


  OL_Lexer *lexer;

  string toString();
  
  typedef std::vector<Rule*> RuleList;

  typedef std::map<string, OL_Context::TableInfo *>  TableInfoMap;

  typedef std::vector<OL_Context::Error *> ErrorList;


  /** The type of watched table mappings */
  typedef std::map<string, string> WatchTableType;


  /** The external stage structure */
  struct ExtStageSpec{
    string stageName;
    

    string inputTupleName;
    

    string outputTupleName;
    

    ExtStageSpec(string name, string input, string output);


    ExtStageSpec(){};


    ExtStageSpec(const ExtStageSpec& s);
  };

  
  /** The type of stage catalogs */
  typedef std::map< string, ExtStageSpec* > ExternalStageSpecMap;



private:
  TableInfoMap*      tables;
  RuleList*          rules;


  /** The watched table map */
  WatchTableType watchTables;

  std::vector<TuplePtr> facts;
  Parse_Functor* singleQuery;
  std::set< string, std::less< string > > tuplesToTrace;


  /** A set of table names for those tables we wish to trace during
      execution */
  std::set< string > tablesToTrace;

  /** Declared external stages */
  ExternalStageSpecMap mStages;




public: 
  ErrorList          errors;
  RuleList*          getRules()       { return rules; };
  TableInfoMap*      getTableInfos()  { return tables;   };


  WatchTableType
  getWatchTables() { return watchTables; };


  std::vector<TuplePtr> getFacts()       { return facts; };
  std::set< string > getTuplesToTrace() { return tuplesToTrace;};  
  std::set< string > getTablesToTrace() { return tablesToTrace;};  


  const ExternalStageSpecMap&
  getExtStagesInfo();
};

extern int ol_parser_parse(OL_Context *env);

#endif /* __OL_PARSEENV_H_ */
