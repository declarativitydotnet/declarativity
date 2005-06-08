/*
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

#ifndef __PARSER_UTIL_H__
#define __PARSER_UTIL_H__

#include <deque>
#include <iostream>
#include <map>
#include "value.h"
#include "val_str.h"

class OL_Lexer;

class Parse_Expr {
public:
  static Parse_Expr* Now;

  Parse_Expr(ValuePtr val=NULL) : v(val), position_(-1) {};
  Parse_Expr(Parse_Expr *val) : v(val->v), position_(-1) {};

  virtual ~Parse_Expr() {};

  virtual bool operator==(const Parse_Expr &e) 
    { return v == e.v; };

  virtual str toString() = 0;

  virtual void position(int p) { position_ = p; };
  virtual int position() { return position_; }

  ValuePtr v;
  int      position_;
};
typedef std::deque<Parse_Expr *> Parse_ExprList;

// Boxing up a ValueRef see we can pass it through the Bison parser
// union unscathed. 
class Parse_Val : public Parse_Expr { 
public:
  Parse_Val(ValuePtr val) : Parse_Expr(val), neg_(false) {};
  virtual void neg(bool n) { neg_ = n; };
  virtual bool neg() { return neg_; };
  virtual str toString() { return ((neg_ ? "-" : "") << v->toString()); };

  virtual operator int();

private:
  bool neg_;
};

class Parse_Var : public Parse_Expr { 
public:
  Parse_Var(ValuePtr var) : Parse_Expr(var) {};
  Parse_Var(const strbuf& var) : Parse_Expr(Val_Str::mk(var))  {};

  virtual str toString() { return v->toString(); };
};

class Parse_Agg : public Parse_Expr {
public:
  enum Operator {MAX, MIN, COUNT};
  static Parse_Expr* DONT_CARE;

  Parse_Agg(Parse_Expr *v, Operator o) 
    : Parse_Expr(v), oper(o) {};

  virtual bool operator==(const Parse_Expr &e);

  virtual str toString();

  Operator oper;
};

class Parse_Bool : public Parse_Expr {
public:
  enum Operator {NOT, AND, OR, EQ, NEQ, GT, LT, LTE, GTE, RANGE, NOP};

  Parse_Bool(Operator o, Parse_Expr *l, Parse_Expr *r=NULL); 
  virtual ~Parse_Bool() { delete lhs; if (rhs) delete rhs; };
  virtual bool operator==(const Parse_Expr &e); 

  virtual str toString();

  Operator   oper;
  Parse_Expr *lhs;
  Parse_Expr *rhs;
};

class Parse_Range : public Parse_Expr {
public:
  enum Interval{RANGEOO, RANGEOC, RANGECO, RANGECC};

  Parse_Range(Interval i, Parse_Expr *l, Parse_Expr *r)
    : type(i), lhs(l), rhs(r) { };

  virtual bool operator==(const Parse_Expr &e);

  virtual str toString();

  Interval   type;
  Parse_Expr *lhs;
  Parse_Expr *rhs;
};

class Parse_Math : public Parse_Expr {
public:
  enum Operator {LSHIFT, RSHIFT, PLUS, MINUS, TIMES, DIVIDE, MODULUS, EXP, NOP};
  Parse_Math(Operator o, Parse_Expr *l, Parse_Expr *r=NULL, bool i = false) 
    : oper(o), id(i), lhs(l), rhs(r) {
      // TODO: if (oper != NOP && rhs == NULL) ERROR!
  };
  ~Parse_Math() { delete lhs; if (rhs) delete rhs; };

  virtual bool operator==(const Parse_Expr &e);

  virtual str toString();

  virtual operator int();

  Operator   oper;
  bool       id;
  Parse_Expr *lhs;
  Parse_Expr *rhs;
};

class Parse_Function : public Parse_Expr {
public:
  Parse_Function(Parse_Expr *n, Parse_ExprList *a) 
    : Parse_Expr(n), args_(a) { };

  ~Parse_Function() { delete args_; };

  virtual str toString();

  void arg(Parse_Expr *arg) { args_->push_back(arg); };
  Parse_Expr* arg(int i) { return args_->at(i); };
  int args() { return args_->size(); };


  Parse_ExprList *args_;
};

class Parse_Term {
public:
  virtual ~Parse_Term() {};

  virtual str toString() = 0;
  void position(int p) { position_ = p; };
  int position() { return position_; };

  int position_;
};
typedef std::deque<Parse_Term *> Parse_TermList;

class Parse_FunctorName {
public:
  Parse_FunctorName(Parse_Expr *n, Parse_Expr *l=NULL);

  str toString();

  str name;
  str loc;
};

class Parse_Functor : public Parse_Term {
public:
  Parse_Functor(Parse_FunctorName *f, Parse_ExprList *a) 
    : fn(f), args_(a) {};
  virtual ~Parse_Functor() {delete fn; delete args_; };

  virtual str toString();

  int aggregate();

  int find(Parse_Expr *arg);
  int find(str argname);

  void arg(Parse_Expr *arg) { args_->push_back(arg); };
  Parse_Expr* arg(int i) { return args_->at(i); };
  int args() { return args_->size(); };

  void replace(int p, Parse_Expr *e);

  Parse_FunctorName *fn;
  Parse_ExprList    *args_;
};

class Parse_Assign : public Parse_Term {
public:
  Parse_Assign(Parse_Expr *v, Parse_Expr *a) 
    : var(NULL), assign(a) {
    var = dynamic_cast<Parse_Var*>(v);
  };
  virtual ~Parse_Assign() { delete var; delete assign; };

  virtual str toString();

  Parse_Var  *var;
  Parse_Expr *assign;
};

class Parse_Select : public Parse_Term {
public:
  Parse_Select(Parse_Expr *s) {
    select = dynamic_cast<Parse_Bool*>(s);
  };
  virtual ~Parse_Select() { delete select; }

  virtual str toString();

  Parse_Bool *select;
};

class Parse_RangeFunction : public Parse_Term {
public:
  Parse_RangeFunction(Parse_Expr *v, Parse_Expr *s, Parse_Expr *e) 
    : var(v), start(s), end(e) { };
  virtual ~Parse_RangeFunction() { delete var; delete start; delete end; }

  virtual str toString();
  
  Parse_Expr *var;
  Parse_Expr *start;
  Parse_Expr *end;
};


#endif /* __PARSER_UTIL_H__ */
