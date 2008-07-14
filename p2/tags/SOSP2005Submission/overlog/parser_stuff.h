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

#ifndef __PARSER_STUFF_H__
#define __PARSER_STUFF_H__

#include <async.h>
#include <deque>
#include <iostream>
#include <map>
#include "value.h"

// Boxing up a ValueRef see we can pass it through the Bison parser
// union unscathed. 
struct Parse_Val { 
  Parse_Val(ValueRef val) : v(val) {};
  ValueRef v;
};

// Really should be a discriminated union
struct Parse_Expr {
  enum Type { DONTCARE=0, VAL, VAR, AGG };
  Parse_Expr() : t(DONTCARE) {};
  ~Parse_Expr() { std::cerr << "Deleting a Parse_Expr\n"; };
  Parse_Expr( Parse_Val *v, Parse_Val *a ) : t(AGG), val(v->v), agg(a->v) { 
    delete v;
    delete a;
  };
  Parse_Expr( Parse_Val *v, bool is_var=false ) : val(v->v) { 
    t = is_var ? VAR : VAL;
    delete v;
  };
  Type	t;
  ValuePtr	val;
  ValuePtr	agg;
};
typedef std::deque<Parse_Expr *> Parse_ExprList;

struct Parse_FunctorName {
  Parse_FunctorName( Parse_Val *n, Parse_Val *l=NULL) { 
    name = n->v->toString(); delete n;
    if (l) {
      loc = l->v->toString();
      delete l;
    } else {
      loc = "";
    }
  };
  str name;
  str loc;
};
struct Parse_Term {
  Parse_FunctorName *fn;
  Parse_ExprList *args;
  Parse_Term(Parse_FunctorName *f, Parse_ExprList *a) : fn(f), args(a) {};
  ~Parse_Term() { delete fn; delete args; };
};
typedef std::deque<Parse_Term *> Parse_TermList;

#endif /* __PARSER_STUFF_H__ */
