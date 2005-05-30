// -*- c-basic-offset: 2; related-file-name: "ol_context.h" -*-
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

#include "ol_context.h"
#include "ol_lexer.h"
#include "val_uint32.h"
#include "tuple.h"
#define DEBUG_OFF
#include "trace.h"

/**********************************************************************
 *
 * Rule methods
 *
 *********************************************************************/

//
// Given a variable name, return it's positional number, or -1 if it
// doesn't exist. 
//
int OL_Context::Rule::find_arg(str varname)
{
  for( u_int i=0; i < args.size(); i++ ) {
    if ( args[i] == varname) {
      return i;
    } 
  } 
  return -1;
}

//
// Given a variable name, return its position number, creating a new
// one if necessary;
//
int OL_Context::Rule::add_arg(str varname)
{
  int pos = find_arg(varname);
  if (pos < 0) {
    args.push_back(varname);
    return args.size() -1;
  } else {
    return pos;
  }
}

//
// Print out the rule for debugging purposes
//
str OL_Context::Rule::toString()
{
  strbuf r;
  r << ":- ";

  for(std::vector<Term>::iterator t = terms.begin(); t!=terms.end(); t++) {
    assert(t->fn != NULL);
    r << t->fn->name;
    if (t->location >= 0) {
      r << "@" << args.at(t->location);
    }
    r << "( ";

    for (unsigned int k = 0; k < t->argNames.size(); k++) {
      r << t->argNames.at(k) << " ";
    }
    r << "), ";
  }

  /*for (unsigned int k = 0; k < args.size(); k++) {
    str nextStr = args.at(k);
    r << nextStr << ",";
    }*/

  return r;
}

/**********************************************************************
 *
 * Context methods
 *
 *********************************************************************/

//
// Adding a new rule to the environment.  Subtle: we need to resolve
// all variable references at this point and convert them to
// positional arguments. 
// 
void OL_Context::add_rule( Parse_Expr *e, Parse_Term *lhs, Parse_TermList *rhs, bool deleteFlag ) 
{
  TRC_FN;
  // Get hold of the functor (rule head) for this new rule.  This
  // holds a list of all the rule bodies that match the head.  Note
  // that this match is only performed on (name,arity), since we
  // convert all bound values to extra "eq" terms in the body down
  // below. 

  //std::cout << "Add rule for functor " << lhs->fn->name << "@" << lhs->fn->loc << "\n";
  Functor *fn = retrieve_functor( lhs->fn->name, lhs->args->size(), lhs->fn->loc,
				  true, true );
  assert(fn != NULL);
  int fict_varnum = 1;	// Counter for inventing anonymous variables. 

  // Create a new rule and register it. 
  Rule *r = New Rule();
  r->ruleID = e->val->toString();
  r->aggField = -1;
  fn->rules.push_back(r);
  r->deleteFlag = deleteFlag;

  // Next, we canonicalize all the args in the rule head.  We build up
  // a list of argument names - the first 'arity' of these will be the
  // free variables in the rule head.  Literals and duplicate free
  // variables here are eliminated, by a process of prepending extra
  // "eq" terms to the body, and inventing new free variables.
  for( Parse_ExprList::iterator i=lhs->args->begin(); i != lhs->args->end(); i++ ) {
    switch ((*i)->t) {
    case Parse_Expr::DONTCARE:
      r->args.push_back("_");
      break;
    case Parse_Expr::AGG:
    case Parse_Expr::VAR:
      // The argument is a free variable - the usual case. 
      if (r->find_arg((*i)->val->toString()) >= 0) {
	// We've found a duplicate variable in the head. Add a new
	// "eq" term to the front of the term list. 
	Term t;
	int new_arg = r->add_arg( strbuf() << "$" << fict_varnum++ );
	t.fn = retrieve_functor( "assign", 2, "-1", true, false );
	t.location = -1;
	t.args.push_back(Arg(r->find_arg((*i)->val->toString())));
	t.args.push_back(Arg(new_arg));
	r->terms.push_back(t);
      } else {
	// Otherwise, just add the new argument.
	r->add_arg((*i)->val->toString());
      }
      // The argument is an aggregate over a free variable
      if ((*i)->t == Parse_Expr::AGG) {
	r->aggFn = (*i)->agg->toString();
	r->aggField = r->find_arg((*i)->val->toString());
	DBG("Aggregate " << r->aggFn 
	    << "<" << (*i)->val->toString() << ">" 
	    << ", var num. " << r->aggField );
      }
      break;
    case Parse_Expr::VAL:
      // The argument is a literal value.  We add a new "eq" term to
      // the front of the term list and invent a new free variable which
      // appears in this clause. 
      Term t;
      int new_arg = r->add_arg( strbuf() << "$" << fict_varnum++ );
      t.fn = retrieve_functor( "assign", 2, "-1", true, false );
      t.location = -1;
      t.args.push_back(Arg(new_arg));
      t.args.push_back(Arg((*i)->val));
      r->terms.push_back(t);
      break;
    }
  }
  
  // Now we've taken care of the head (and possibly created a few
  // terms), we run through the rest of the terms converting all the
  // variables we encounter to indices. 
  for( Parse_TermList::iterator j=rhs->begin(); j != rhs->end(); j++ ) {
    Term t;
    t.fn = retrieve_functor( (*j)->fn->name, (*j)->args->size(), (*j)->fn->loc, true, false );
    if ((*j)->fn->loc == "") {
      t.location = -1;
    } else {
      t.location = r->add_arg((*j)->fn->loc);
    }
    for( Parse_ExprList::iterator arg = (*j)->args->begin();
	 arg != (*j)->args->end();	 arg++ ) {
      switch ((*arg)->t) {
      case Parse_Expr::DONTCARE:
	t.args.push_back(Arg(r->add_arg(strbuf() << "$" << fict_varnum++)));
	break;
      case Parse_Expr::AGG:
	// XXX We ignore aggregates not in the rule head
      case Parse_Expr::VAR:
	t.args.push_back(Arg(r->add_arg((*arg)->val->toString())));
	break;
      case Parse_Expr::VAL:
	t.args.push_back(Arg((*arg)->val));
	break;
      }
    }

    // generate the arg names for the term
    for (unsigned int k = 0; k < t.args.size(); k++) {      
      int pos = t.args.at(k).var;
      if (pos == -1) {
	t.argNames.push_back(t.args.at(k).val->toString());
      } else {
	t.argNames.push_back(r->args.at(pos));
      }
    }

    r->terms.push_back(t);
  }

  strbuf s;
  s << "( ";
  for (uint k = 0; k < fn->arity; k++) {
    s << r->args.at(k) << " ";
  }
  s << ")";

  std::cout << r->ruleID << ": New rule for functor " << lhs->fn->name << "@" 
	    << lhs->fn->loc << str(s) << r->toString() << "\n";
  
  delete lhs;
  delete rhs;
}

//
// Look up a functor by its name and arity, creating it if necessary.
//
OL_Context::Functor *OL_Context::retrieve_functor( str name, int arity, str loc,
						   bool create,
						   bool define )
{
  //TRC_FN;
  str key = OL_Context::Functor::calc_key(name, arity, loc);
  FunctorMap::iterator fni = functors->find(key);
  if (fni == functors->end() ) {
    if (create == false) {
      return NULL;
    } else {
      OL_Context::Functor *fn = New Functor(name, arity, loc);
      fn->defined = define;
      functors->insert(std::make_pair(key, fn));
      return fn;
    }
  } else {
    return fni->second;
  }
}

//
// Print out the whole parse result, if we can
//
str OL_Context::toString()
{
  strbuf r;
  for( TableInfoMap::iterator i = tables->begin(); i != tables->end(); i++ ) {
    TableInfo *t=i->second;
    r << "materialise(" << t->tableName << ","
      << t->arity << ","
      << t->timeout << ","
      << t->size << ").\n";
  }
  
  for( FunctorMap::iterator f=functors->begin(); f != functors->end(); f++) {
    r << f->first << " :- \n";
    for( std::vector<Rule *>::iterator ri = f->second->rules.begin();
	 ri != f->second->rules.end(); 
	 ri++) {
    }
  }
  return r;
}

OL_Context::OL_Context()
  : lexer(NULL)
{
  functors = New FunctorMap();
  tables = New TableInfoMap();
}

OL_Context::~OL_Context()
{
  delete functors;
  delete tables;
}

void OL_Context::parse_string(const char *prog)
{
  assert(lexer==NULL);
  lexer = New OL_Lexer(prog);
  ol_parser_parse(this);
  delete lexer;
  lexer = NULL;
}
void OL_Context::parse_stream(std::istream *str)
{
  assert(lexer==NULL);
  lexer = New OL_Lexer(str);
  ol_parser_parse(this);
  delete lexer;
  lexer = NULL;
}

void OL_Context::error(str msg)
{
  std::cerr << "*** line " << lexer->line_num() << ": " << msg << "\n"; 
}

//
// Materialize a table
// 
void OL_Context::materialize( Parse_ExprList *l)
{
  TableInfo *tableInfo = new TableInfo();
  
  if (l->size() != 4) {
    error("bad number of arguments to materialize");
    goto mat_error;
  }

  // tablename
  if (l->at(0)->t != Parse_Expr::VAL ) {
    error("bad table name for materialization");
    goto mat_error;
  }
  tableInfo->tableName = l->at(0)->val->toString();
  
  // arity
  tableInfo->arity = Val_UInt32::cast(l->at(1)->val);
  if ( tableInfo->arity < 1 ) { 
    error("bad arity for materialized table");
    goto mat_error;
  }
  
  // timeout
  if (l->at(2)->val->toString() == "infinity") {
    tableInfo->timeout = -1; 
  } else {
    tableInfo->timeout = Val_UInt32::cast(l->at(2)->val);
  }
  if (tableInfo->timeout == 0) {
    error("bad timeout for materialized table");
    goto mat_error;
  }

  // size
  if (l->at(3)->val->toString() == "infinity") {
    tableInfo->size = -1; 
  } else {
    tableInfo->size = Val_UInt32::cast(l->at(3)->val);
  }
  if (tableInfo->size == 0) {
    error("bad size for materialized table");
    goto mat_error;
  }

  tables->insert(std::make_pair(tableInfo->tableName, tableInfo));
  
  DBG( "Materialize " << tableInfo->tableName << "/" << tableInfo->arity 
       << ", timeout " << tableInfo->timeout << ", size " << tableInfo->size); 

 mat_error:
  delete l;
}

void OL_Context::primaryKeys( Parse_ExprList *l)
{
  
  if (l->size() <= 1) {
    error("bad number of arguments to form primary keys");
    delete l;
    return;
  }

  TableInfoMap::iterator _iterator = tables->find(l->at(0)->val->toString());
  if (_iterator != tables->end()) {
    for (uint k = 0; k < l->size()-1; k++) {
      _iterator->second->primaryKeys.push_back(atoi(l->at(k+1)->val->toString().cstr()));
    }
  }
}

void OL_Context::watchVariables( Parse_ExprList *l)
{
  std::cout << "Watch variables " << l->at(0)->val->toString() << "\n";
  watchTables.insert(l->at(0)->val->toString());
}

//
// Adding a fact
//
void OL_Context::add_fact( Parse_Term *t)
{
  str tblname;

  /*if (t->fn->loc != "") {
    error("Location not allowed in fact declarations");
    goto fact_error;
    }*/
  tblname = t->fn->name;

  if (tblname == "materialize" || tblname == "materialise" ) {
    materialize(t->args);
  } else if (tblname == "watch") { 
    watchVariables(t->args);
  } else if (tblname == "primarykeys") {
    primaryKeys(t->args);
  } else {
    TupleRef tpl = Tuple::mk();
    for( Parse_ExprList::iterator i = t->args->begin(); 
	 i != t->args->end(); 
	 i++) {
      if ((*i)->t != Parse_Expr::VAL) {
	error("free variables and don't-cares not allowed in facts.");
	goto fact_error;
      } else {
	ValueRef v=(*i)->val;
	tpl->append(v);
      }
    }
    tpl->freeze();
    DBG( "Fact: " << tblname << " <- " << tpl->toString() );
    
  fact_error:
    delete t;
  }
}

void OL_Context::add_event( Parse_Term *t)
{
}

OL_Context::FunctorMap* OL_Context::getFunctors() { return functors; }

OL_Context::TableInfoMap* OL_Context::getTableInfos() { return tables; } 

std::set<str> OL_Context::getWatchTables() { return watchTables; } 
