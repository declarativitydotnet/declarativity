// -*- c-basic-offset: 2; related-file-name: "ol_context.h" -*-
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

#include "ol_context.h"
#include "ol_lexer.h"
#include "val_uint32.h"
#include "tuple.h"
#define DEBUG_OFF
#include "trace.h"

int OL_Context::ruleCount = 0;

/**********************************************************************
 *
 * Rule methods
 *
 *********************************************************************/

//
// Print out the rule for debugging purposes
//
string OL_Context::Rule::toString() {
  ostringstream r;
  r << ruleID << " ";
  r << head->toString() << " :- ";

  std::list<Parse_Term*>::iterator t = terms.begin();
  r << (*t)->toString();
  t++;
  for(;
      t != terms.end();
      t++) {
    r << "," 
      << (*t)->toString();
  }
  r << ".";

  return r.str();
}

string OL_Context::TableInfo::toString() {
  ostringstream t;
  t << "MATERIALIZE( " << tableName << ", "
    << timeout << ", "
    << size;

  if (primaryKeys.size() > 0) {
    t << ", " << primaryKeys.at(0);
  }
  t << " )";

  return t.str();
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
void
OL_Context::rule(Parse_Term *lhs,
                 Parse_TermList *rhs, 
                 bool deleteFlag,
                 Parse_Expr *n) 
{
  TRC_FN;
  Parse_Functor *h    = dynamic_cast<Parse_Functor*>(lhs);
  string     ruleName    = (n) ? n->v->toString() : "";
  int     fict_varnum = 1;		// Counter for inventing anonymous variables. 


  // Get hold of the functor (rule head) for this new rule.  This
  // holds a list of all the rule bodies that match the head.  Note
  // that this match is only performed on (name,arity), since we
  // convert all bound values to extra "eq" terms in the body down
  // below. 

  // Create a new rule and register it. 
  Rule *r = new Rule(ruleName, h, deleteFlag);

  // Next, we canonicalize all the args in the rule head.  We build up
  // a list of argument names - the first 'arity' of these will be the
  // free variables in the rule head.  Literals and duplicate free
  // variables here are eliminated, by a process of appending extra
  // "eq" terms to the body, and inventing new free variables.
  for(int i = 0; i < h->args(); i++) {
    Parse_Agg *agg = NULL;
    Parse_Var *var = NULL;
    Parse_Val *val = NULL;

    if ((agg = dynamic_cast<Parse_Agg*>(h->arg(i))) != NULL) {
      agg->position(i); 
    }
    else if ((var = dynamic_cast<Parse_Var*>(h->arg(i))) != NULL) {
      // The argument is a free variable - the usual case. 
      int loc = h->find(var->toString());
      if (loc < i) {
        ostringstream oss;
        oss << "$" << fict_varnum++;
	// We've found a duplicate variable in the head. Add a new
	// "eq" term to the front of the term list. 
        Parse_Var *tmp = new Parse_Var(oss.str());
        tmp->position(i);
        h->replace(i, tmp);
        r->terms.push_back(new Parse_Assign(tmp, h->arg(loc)));
      } else {
        var->position(i);
      }
    }
    else if ((val = dynamic_cast<Parse_Val*>(h->arg(i))) != NULL) {
      ostringstream oss;
      oss << "$" << fict_varnum++;
      Parse_Var *tmp = new Parse_Var(oss.str());
      tmp->position(i);
      h->replace(i, tmp);
      r->terms.push_back(new Parse_Assign(tmp, val));
    }
    else {
      error("Parse rule unknown functor body type.");
    }
  }

  // Now we've taken care of the head (and possibly created a few
  // terms), we run through the rest of the terms converting all the
  // variables we encounter to indices. 
  int tpos = 1;
  for(Parse_TermList::iterator iter = rhs->begin(); iter != rhs->end(); 
      (*iter)->position(tpos++), iter++) {
    Parse_Functor       *f  = NULL;
    Parse_Assign        *a  = NULL;
    Parse_Select        *s  = NULL;

    if ((f = dynamic_cast<Parse_Functor*>(*iter)) != NULL) {

      for(int i = 0; i < f->args(); i++) {
        Parse_Var *var = NULL;
        Parse_Val *val = NULL;

        if ((var = dynamic_cast<Parse_Var*>(f->arg(i))) != NULL) {
          var->position(i);
        }
        else if ((val = dynamic_cast<Parse_Val*>(f->arg(i))) != NULL) {
          val->position(i);
        }
        else {
          error("Parse functor term unknown argument type.");
        }
      }
    }
    else if ((a = dynamic_cast<Parse_Assign*>(*iter)) != NULL) {

    }
    else if ((s = dynamic_cast<Parse_Select*>(*iter)) != NULL) {

    }
    else error("Internal parse error: unknown term type!");

    r->terms.push_back(*iter);
  }
  rules->push_back(r);
}

void OL_Context::aggRule( Parse_Term *lhs, Parse_AggTerm *rhs, 
			  bool deleteFlag, Parse_Expr *n ) 
{
  Parse_Functor *h    = dynamic_cast<Parse_Functor*>(lhs);
  string     ruleName    = (n) ? n->v->toString() : "";
  Rule *r = new Rule(ruleName, h, deleteFlag);  
  r->terms.push_back(rhs);
  rules->push_back(r);
}


//
// Print out the whole parse result, if we can
//
string OL_Context::toString()
{
  ostringstream r;

  // Errors first
  for( ErrorList::iterator e=errors.begin(); e!=errors.end(); e++) {
    r << "error(" << (*e)->line_num << ",'" << (*e)->msg << "').\n";
  }
  for (TableInfoMap::iterator i = tables->begin(); i != tables->end(); i++ ) {
    r << i->second->toString() << "\n";
  }
  
  for(RuleList::iterator rule=rules->begin(); rule != rules->end(); rule++) {
    r << (*rule)->toString() << "\n";
  }
  return r.str();
}

OL_Context::OL_Context() : lexer(NULL)
{
  rules  = new RuleList();
  tables = new TableInfoMap();
}

OL_Context::~OL_Context()
{
  delete rules;
  delete tables;
}

void OL_Context::parse_string(const char *prog)
{
  assert(lexer==NULL);
  lexer = new OL_Lexer(prog);
  ol_parser_parse(this);
  delete lexer;
  lexer = NULL;
}
void OL_Context::parse_stream(std::istream *str)
{
  assert(lexer==NULL);
  lexer = new OL_Lexer(str);
  ol_parser_parse(this);
  delete lexer;
  lexer = NULL;
}

void OL_Context::error(string msg)
{
  std::cerr << "PARSER ERROR (line " << lexer->line_num() << "): " << msg << "\n";
  errors.push_back(new OL_Context::Error(lexer->line_num(), msg));
}

void OL_Context::watch(Parse_Expr *w)
{
  std::cout << "Add watch variable " << w->toString() << "\n";
  watchTables.insert(w->v->toString());
}

void
OL_Context::traceTuple(Parse_Expr *w)
{
  std::cout << "Add trace variable " << w->toString() << "\n";
  tuplesToTrace.insert(w->v->toString());
}


void
OL_Context::traceTable(Parse_Expr *w)
{
  std::cout << "Add traced table " << w->toString() << "\n";
  tablesToTrace.insert(w->v->toString());
}



void
OL_Context::table(Parse_Expr *name,
                  Parse_Expr *ttl, 
                  Parse_Expr *size,
                  Parse_ExprList *keys)
{
  TableInfo  *tableInfo = new TableInfo();
  tableInfo->tableName = name->toString();
  
  int myTtl = Val_Int64::cast(ttl->v);
  if (myTtl == -1) {
    tableInfo->timeout = Table2::NO_EXPIRATION;
  } else if (myTtl == 0) {
    error("bad timeout for materialized table");
  } else {
    tableInfo->timeout = boost::posix_time::seconds(myTtl);
  }


  int mySize = Val_Int64::cast(size->v);
  // Hack because infinity token has a -1 value
  if (mySize == -1) {
    tableInfo->size = Table2::NO_SIZE;
  } else {
    tableInfo->size = mySize;
  }

  if (keys) {
    for (Parse_ExprList::iterator i = keys->begin();
         i != keys->end();
         i++)
      tableInfo->primaryKeys.push_back(Val_UInt32::cast((*i)->v));
  }
  
  tables->insert(std::make_pair(tableInfo->tableName, tableInfo));
  
  DBG("Materialize " << tableInfo->tableName << "/"
      << ", timeout " << tableInfo->timeout
      << ", size " << tableInfo->size); 
}

//
// Adding a fact
//
void OL_Context::fact(Parse_Term *term)
{
  Parse_Functor     *functor = dynamic_cast<Parse_Functor*>(term);
  Parse_FunctorName *name    = functor->fn;
  Parse_ExprList    *args    = functor->args_;
  TuplePtr tpl = Tuple::mk();
  tpl->append(Val_Str::mk(name->name));
  for (Parse_ExprList::iterator iter = args->begin(); 
       iter != args->end(); iter++) {
    Parse_Val  *v = dynamic_cast<Parse_Val*>(*iter);
    Parse_Math *m = dynamic_cast<Parse_Math*>(*iter);
    if (v != NULL) {
      tpl->append(v->value());
    } 
    else if (m != NULL) {
      tpl->append(m->value());
    }
    else {
      error("free variables and don't-cares not allowed in facts:" 
	    + (*iter)->toString());
      goto fact_error;
    }
  }
  tpl->freeze();
  facts.push_back(tpl);

  DBG("Fact: " << name->v->toString() << " <- " << tpl->toString());
    
  fact_error:
    delete name;
    delete args;
}


void OL_Context::query( Parse_Term *term) 
{
  singleQuery = dynamic_cast<Parse_Functor*>(term);
  std::cout << "Add query " << singleQuery->toString() << "\n";  
}

