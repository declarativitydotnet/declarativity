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
    for(std::vector<Arg>::iterator a=t->args.begin(); a!=t->args.end(); a++) {
      if (a->var >= 0) {
	r << args.at(a->var);
      } else {
	r << a->val->toString();
      }
      r << " ";
    }
    r << "), ";
  }
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
void OL_Context::add_rule( Parse_Term *lhs, Parse_TermList *rhs) 
{
  TRC_FN;
  // Get hold of the functor (rule head) for this new rule.  This
  // holds a list of all the rule bodies that match the head.  Note
  // that this match is only performed on (name,arity), since we
  // convert all bound values to extra "eq" terms in the body down
  // below. 

  TRC("Functor " << lhs->fn->name << "@" << lhs->fn->loc );
  Functor *fn = retrieve_functor( lhs->fn->name, lhs->args->size(),
				  true, true );
  assert(fn != NULL);
  int fict_varnum = 1;	// Counter for inventing anonymous variables. 

  // Create a new rule and register it. 
  Rule *r = New Rule();
  fn->rules.push_back(r);

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
    case Parse_Expr::VAR:
      // The argument is a free variable - the usual case. 
      if (r->find_arg((*i)->val->toString()) >= 0) {
	// We've found a duplicate variable in the head. Add a new
	// "eq" term to the front of the term list. 
	Term t;
	int new_arg = r->add_arg( strbuf() << "$" << fict_varnum++ );
	t.fn = retrieve_functor( "eq", 2, true, false );
	t.location = -1;
	t.args.push_back(Arg(r->find_arg((*i)->val->toString())));
	t.args.push_back(Arg(new_arg));
	r->terms.push_back(t);
      } else {
	// Otherwise, just add the new argument.
	r->add_arg((*i)->val->toString());
      }
      break;
    case Parse_Expr::VAL:
      // The argument is a literal value.  We add a new "eq" term to
      // the front of the term list and invent a new free variable which
      // appears in this clause. 
      Term t;
      int new_arg = r->add_arg( strbuf() << "$" << fict_varnum++ );
      t.fn = retrieve_functor( "eq", 2, true, false );
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
    t.fn = retrieve_functor( (*j)->fn->name, (*j)->args->size(), true, false );
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
      case Parse_Expr::VAR:
	t.args.push_back(Arg(r->add_arg((*arg)->val->toString())));
	break;
      case Parse_Expr::VAL:
	t.args.push_back(Arg((*arg)->val));
	break;
      }
    }
    r->terms.push_back(t);
  }
  
  delete lhs;
  delete rhs;

  DBG(" New rule: " << r->toString());
}

//
// Look up a functor by its name and arity, creating it if necessary.
//
OL_Context::Functor *OL_Context::retrieve_functor( str name, int arity, 
						     bool create,
						     bool define )
{
  //TRC_FN;
  str key = OL_Context::Functor::calc_key(name, arity);
  FunctorMap::iterator fni = functors.find(key);
  if (fni == functors.end() ) {
    if (create == false) {
      return NULL;
    } else {
      OL_Context::Functor *fn = New Functor(name, arity);
      fn->defined = define;
      functors.insert(std::make_pair(key, fn));
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
  
  for( FunctorMap::iterator f=functors.begin(); f != functors.end(); f++) {
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
  str tblname;
  int arity;
  int timeout;
  int size;

  if (l->size() != 4) {
    error("bad number of arguments to materialize");
    goto mat_error;
  }

  if (l->at(0)->t != Parse_Expr::VAL ) {
    error("bad table name for materialization");
    goto mat_error;
  }
  tblname = l->at(0)->val->toString();
  
  arity = Val_UInt32::cast(l->at(1)->val);
  if ( arity < 1 ) { 
    error("bad arity for materialized table");
    goto mat_error;
  }
  
  if (l->at(2)->val->toString() == "infinity") {
    timeout = -1; 
  } else {
    timeout = Val_UInt32::cast(l->at(2)->val);
  }
  if (timeout == 0) {
    error("bad timeout for materialized table");
    goto mat_error;
  }

  if (l->at(3)->val->toString() == "infinity") {
    size = -1; 
  } else {
    size = Val_UInt32::cast(l->at(3)->val);
  }
  if (size == 0) {
    error("bad size for materialized table");
    goto mat_error;
  }
  
  DBG( "Materialize " << tblname << "/" << arity 
       << ", timeout " << timeout << ", size " << size );
    
 mat_error:
  delete l;
}

//
// Adding a fact
//
void OL_Context::add_fact( Parse_Term *t)
{
  TupleRef tpl = Tuple::mk();
  str tblname;

  if (t->fn->loc != "") {
    error("Location not allowed in fact declarations");
    goto fact_error;
  }
  tblname = t->fn->name;

  for(Parse_ExprList::iterator i = t->args->begin(); i != t->args->end(); i++){
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
