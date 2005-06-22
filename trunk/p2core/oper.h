/*
 * @(#)$Id$
 *
 * Copyright (c) 2004 Intel Corporation. All rights reserved.
 *
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 * DESCRIPTION: P2's operator system for basic concrete types.
 *
 */

#ifndef __OPER_IMPL_H__
#define __OPER_IMPL_H__

#include "value.h"

/**
 * MACROS for throwing operator not supported exceptions. 
 * An operator is not supported if the types of the operands
 * do not override the operator. 
 */
#define NOSUP1(o, t1) \
throw New Exception("Oper("<<str(o)<<") not supported for types "<<str(t1))

#define NOSUP2(o, t1, t2) \
throw New Exception("Oper("<<str(o)<<") not supported for types " \
                     <<str(t1)<<", "<<str(t2))

#define NOSUP3(o, t1, t2, t3) \
throw New Exception("Oper("<<str(o)<<") not supported for types " \
                     <<str(t1)<<", "<<str(t2)<<", "<<str(t3))

class Oper {
public:
  /**
   * Operator Type Table
   * This is an NxN matrix of operator functions. Most operators
   * take operate on two operands. The types of those operators are
   * used to index this table, which will hold the operator function
   * of the base type of the operands. The initialization of this table
   * occurs in oper.C, but this be changed to support a different type 
   * conversion lattice.
   * See also value.h for defining new P2 concrete types and how such
   * new types affect the operator table definition.
   */
  const static Oper** oper_table_[Value::TYPES][Value::TYPES];

  /**
   * Thrown when operand types do not override an operator function.
   */ 
  class Exception {
  public:
    Exception(str d) : desc_(d) {};

    operator str() { return desc_; };

    private:
      str desc_;
  };

  /**
   * OPERATOR FUNCTIONS
   * An operator function is where the functionality of the operator
   * exists. The default operator functions listed below will throw
   * an exception, meaning that the operator is not supported for that
   * type. Each P2 concreate type will define a subclass of Oper and
   * override those operator functions that have definitions for the
   * respective type.
   */
  virtual ValuePtr _bnot (const ValueRef& v) const
    { NOSUP1("~", v->typeName()); return false; };
  virtual ValuePtr _band (const ValueRef& v1, const ValueRef& v2) const
    { NOSUP2("&", v1->typeName(), v2->typeName()); return false; };
  virtual ValuePtr _bor (const ValueRef& v1, const ValueRef& v2)const
    { NOSUP2("|", v1->typeName(), v2->typeName()); return false; };
  virtual ValuePtr _bxor (const ValueRef& v1, const ValueRef& v2)const
    { NOSUP2("^", v1->typeName(), v2->typeName()); return false; };
  virtual ValuePtr _lshift (const ValueRef& v1, const ValueRef& v2) const
    { NOSUP2("<<", v1->typeName(), v2->typeName()); return NULL; };
  virtual ValuePtr _rshift (const ValueRef& v1, const ValueRef& v2) const
    { NOSUP2(">>", v1->typeName(), v2->typeName()); return NULL; };
  virtual ValuePtr _plus (const ValueRef& v1, const ValueRef& v2) const
    { NOSUP2("+", v1->typeName(), v2->typeName()); return NULL; };
  virtual ValuePtr _minus (const ValueRef& v1, const ValueRef& v2) const
    { NOSUP2("-", v1->typeName(), v2->typeName()); return NULL; };
  virtual ValuePtr _times (const ValueRef& v1, const ValueRef& v2) const
    { NOSUP2("*", v1->typeName(), v2->typeName()); return NULL; };
  virtual ValuePtr _divide (const ValueRef& v1, const ValueRef& v2) const
    { NOSUP2("/", v1->typeName(), v2->typeName()); return NULL; };
  virtual ValuePtr _mod (const ValueRef& v1, const ValueRef& v2) const
    { NOSUP2("\%", v1->typeName(), v2->typeName()); return NULL; };

  virtual bool _eq (const ValueRef& v1, const ValueRef& v2) const
    { NOSUP2("==", v1->typeName(), v2->typeName()); return false; };
  virtual bool _neq (const ValueRef& v1, const ValueRef& v2) const
    { NOSUP2("!=", v1->typeName(), v2->typeName()); return false; };
  virtual bool _gt (const ValueRef& v1, const ValueRef& v2) const
    { NOSUP2(">", v1->typeName(), v2->typeName()); return false; };
  virtual bool _gte (const ValueRef& v1, const ValueRef& v2) const
    { NOSUP2(">=", v1->typeName(), v2->typeName()); return false; };
  virtual bool _lt (const ValueRef& v1, const ValueRef& v2) const
    { NOSUP2("<", v1->typeName(), v2->typeName()); return false; };
  virtual bool _lte (const ValueRef& v1, const ValueRef& v2) const
    { NOSUP2("<=", v1->typeName(), v2->typeName()); return false; };

  virtual bool _inOO (const ValueRef& v1, const ValueRef& v2, 
                          const ValueRef& v3) const { 
    NOSUP3("()", v1->typeName(), v2->typeName(), v3->typeName()); 
    return NULL; 
  };
  virtual bool _inOC (const ValueRef& v1, const ValueRef& v2, 
                          const ValueRef& v3) const { 
    NOSUP3("(]", v1->typeName(), v2->typeName(), v3->typeName()); 
    return NULL; 
  };
  virtual bool _inCO (const ValueRef& v1, const ValueRef& v2, 
                          const ValueRef& v3) const { 
    NOSUP3("[)", v1->typeName(), v2->typeName(), v3->typeName()); 
    return NULL; 
  };
  virtual bool _inCC (const ValueRef& v1, const ValueRef& v2, 
                          const ValueRef& v3) const { 
    NOSUP3("[]", v1->typeName(), v2->typeName(), v3->typeName()); 
    return NULL; 
  };
};
 
/**
 * C++ Operator functions
 * The job of these functions is to simply lookup the base
 * type of the type operands and call the operator function 
 * (defined by the subclass of Oper) defined by the base type. 
 */
ValueRef operator<<(const ValueRef& v1, const ValueRef& v2);
ValueRef operator>>(const ValueRef& v1, const ValueRef& v2); 
ValueRef operator+ (const ValueRef& v1, const ValueRef& v2); 
ValueRef operator- (const ValueRef& v1, const ValueRef& v2); 
ValueRef operator* (const ValueRef& v1, const ValueRef& v2); 
ValueRef operator/ (const ValueRef& v1, const ValueRef& v2); 
ValueRef operator% (const ValueRef& v1, const ValueRef& v2); 
ValueRef operator~ (const ValueRef& v);
ValueRef operator& (const ValueRef& v1, const ValueRef& v2); 
ValueRef operator| (const ValueRef& v1, const ValueRef& v2); 
ValueRef operator^ (const ValueRef& v1, const ValueRef& v2); 

bool     operator==(const ValueRef& v1, const ValueRef& v2); 
bool     operator!=(const ValueRef& v1, const ValueRef& v2); 
bool     operator< (const ValueRef& v1, const ValueRef& v2); 
bool     operator<=(const ValueRef& v1, const ValueRef& v2); 
bool     operator> (const ValueRef& v1, const ValueRef& v2); 
bool     operator>=(const ValueRef& v1, const ValueRef& v2); 

bool     inOO(const ValueRef& v1, const ValueRef& v2, const ValueRef& v3);
bool     inOC(const ValueRef& v1, const ValueRef& v2, const ValueRef& v3);
bool     inCO(const ValueRef& v1, const ValueRef& v2, const ValueRef& v3);
bool     inCC(const ValueRef& v1, const ValueRef& v2, const ValueRef& v3);

/**
 * Basic Operator Function Template for comparison based operators.
 * Most of the concrete P2 types define a compareTo method. The
 * OperCompare template will override the comparison operators (==, !=, etc.)
 * of Oper by implementing the respective operator using the compareTo logic.
 */
template <class T> class OperCompare : public Oper { 
public: 
  virtual bool _eq (const ValueRef& v1, const ValueRef& v2) const {
    ValueRef c1 = T::mk(T::cast(v1));
    ValueRef c2 = T::mk(T::cast(v2));
    return c2->compareTo(c1) == 0;
  };
  virtual bool _neq (const ValueRef& v1, const ValueRef& v2) const {
    ValueRef c1 = T::mk(T::cast(v1));
    ValueRef c2 = T::mk(T::cast(v2));
    return c2->compareTo(c1) != 0;
  };
  virtual bool _gt (const ValueRef& v1, const ValueRef& v2) const {
    ValueRef c1 = T::mk(T::cast(v1));
    ValueRef c2 = T::mk(T::cast(v2));
    return c2->compareTo(c1) > 0;
  };
  virtual bool _gte (const ValueRef& v1, const ValueRef& v2) const {
    ValueRef c1 = T::mk(T::cast(v1));
    ValueRef c2 = T::mk(T::cast(v2));
    return c2->compareTo(c1) >= 0;
  };
  virtual bool _lt (const ValueRef& v1, const ValueRef& v2) const {
    ValueRef c1 = T::mk(T::cast(v1));
    ValueRef c2 = T::mk(T::cast(v2));
    return c2->compareTo(c1) < 0;
  };
  virtual bool _lte (const ValueRef& v1, const ValueRef& v2) const {
    ValueRef c1 = T::mk(T::cast(v1));
    ValueRef c2 = T::mk(T::cast(v2));
    return c2->compareTo(c1) <= 0;
  };

  virtual bool _inOO(const ValueRef& vc, const ValueRef& fc,
                     const ValueRef& tc) const {
    ValueRef v = T::mk(T::cast(vc));
    ValueRef f = T::mk(T::cast(fc));
    ValueRef t = T::mk(T::cast(tc));
    return (((v->compareTo(f) >  0) && (v->compareTo(t) <  0)) ||
            ((t->compareTo(f) <= 0) && (v->compareTo(f) >  0)) ||
            ((v->compareTo(t) <  0) && (t->compareTo(f) <= 0)));
  }

  virtual bool _inOC(const ValueRef& vc, const ValueRef& fc,
                     const ValueRef& tc) const {
    ValueRef v = T::mk(T::cast(vc));
    ValueRef f = T::mk(T::cast(fc));
    ValueRef t = T::mk(T::cast(tc));
    return (((v->compareTo(f) >  0) && (v->compareTo(t) <= 0)) ||
            ((t->compareTo(f) <= 0) && (v->compareTo(f) >  0)) ||
            ((v->compareTo(t) <= 0) && (t->compareTo(f) <= 0)));
  }

  virtual bool _inCO(const ValueRef& vc, const ValueRef& fc,
                     const ValueRef& tc) const {
    ValueRef v = T::mk(T::cast(vc));
    ValueRef f = T::mk(T::cast(fc));
    ValueRef t = T::mk(T::cast(tc));
    return (((v->compareTo(f) >= 0) && (v->compareTo(t) <  0)) ||
            ((t->compareTo(f) <= 0) && (v->compareTo(f) >= 0)) ||
            ((v->compareTo(t) <  0) && (t->compareTo(f) <= 0)));
  }

  virtual bool _inCC(const ValueRef& vc, const ValueRef& fc,
                     const ValueRef& tc) const {
    ValueRef v = T::mk(T::cast(vc));
    ValueRef f = T::mk(T::cast(fc));
    ValueRef t = T::mk(T::cast(tc));
    return (((v->compareTo(f) >= 0) && (v->compareTo(t) <= 0)) ||
            ((t->compareTo(f) <= 0) && (v->compareTo(f) >= 0)) ||
            ((v->compareTo(t) <= 0) && (t->compareTo(f) <= 0)));
  }
};

/**
 * Basic Operator Function Template.
 * This template provides basic functionality for ALL operator functions
 * defined in Oper. Only the most basic concrete types will be able to
 * make use of this template (e.g., Int32, UInt32, Int64, UInt64, Double).
 */
template <class T> class OperImpl : public OperCompare<T> { 
public: 
#ifndef DOUBLE_HACK
  virtual ValuePtr _bnot (const ValueRef& v) const {
    return T::mk(~(T::cast(v)));
  };
  virtual ValuePtr _band (const ValueRef& v1, const ValueRef& v2) const {
    return T::mk(T::cast(v1) & T::cast(v2));
  };
  virtual ValuePtr _bor (const ValueRef& v1, const ValueRef& v2) const {
    return T::mk(T::cast(v1) | T::cast(v2));
  };
  virtual ValuePtr _bxor (const ValueRef& v1, const ValueRef& v2) const {
    return T::mk(T::cast(v1) ^ T::cast(v2));
  };
  virtual ValuePtr _lshift (const ValueRef& v1, const ValueRef& v2) const {
    return T::mk(T::cast(v1) << T::cast(v2));
  };
  virtual ValuePtr _rshift (const ValueRef& v1, const ValueRef& v2) const {
    return T::mk(T::cast(v1) >> T::cast(v2));
  };
  virtual ValuePtr _mod (const ValueRef& v1, const ValueRef& v2) const {
    return T::mk(T::cast(v1) % T::cast(v2));
  };
#endif

  virtual ValuePtr _plus (const ValueRef& v1, const ValueRef& v2) const {
    return T::mk(T::cast(v1) + T::cast(v2));
  };
  virtual ValuePtr _minus (const ValueRef& v1, const ValueRef& v2) const {
    return T::mk(T::cast(v1) - T::cast(v2));
  };
  virtual ValuePtr _times (const ValueRef& v1, const ValueRef& v2) const {
    return T::mk(T::cast(v1) * T::cast(v2));
  };
  virtual ValuePtr _divide (const ValueRef& v1, const ValueRef& v2) const {
    return T::mk(T::cast(v1) / T::cast(v2));
  };

};

#endif /* __OPER_H_ */
