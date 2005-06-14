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
 * DESCRIPTION: P2's concrete type system.  The base type for values.
 *
 */

#ifndef __OPER_IMPL_H__
#define __OPER_IMPL_H__

#include "value.h"

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
  const static Oper* oper_table_[Value::TYPES][Value::TYPES];

  class Exception {
  public:
    Exception(str d) : desc_(d) {};

    operator str() { return desc_; };

    private:
      str desc_;
  };

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

template <class T> class OperCompare : public Oper { 
public: 
  virtual bool _eq (const ValueRef& v1, const ValueRef& v2) const {
    return v1->compareTo(v2) == 0;
  };
  virtual bool _neq (const ValueRef& v1, const ValueRef& v2) const {
    return v1->compareTo(v2) != 0;
  };
  virtual bool _gt (const ValueRef& v1, const ValueRef& v2) const {
    return v1->compareTo(v2) > 0;
  };
  virtual bool _gte (const ValueRef& v1, const ValueRef& v2) const {
    return v1->compareTo(v2) >= 0;
  };
  virtual bool _lt (const ValueRef& v1, const ValueRef& v2) const {
    return v1->compareTo(v2) < 0;
  };
  virtual bool _lte (const ValueRef& v1, const ValueRef& v2) const {
    return v1->compareTo(v2) <= 0;
  };

  virtual bool _inOO(const ValueRef& v, const ValueRef& f,
                     const ValueRef& t) const {
    return (((v->compareTo(f) >  0) && (v->compareTo(t) <  0)) ||
            ((t->compareTo(f) <= 0) && (v->compareTo(f) >  0)) ||
            ((v->compareTo(t) <  0) && (t->compareTo(f) <= 0)));
  }

  virtual bool _inOC(const ValueRef& v, const ValueRef& f,
                     const ValueRef& t) const {
    return (((v->compareTo(f) >  0) && (v->compareTo(t) <= 0)) ||
            ((t->compareTo(f) <= 0) && (v->compareTo(f) >  0)) ||
            ((v->compareTo(t) <= 0) && (t->compareTo(f) <= 0)));
  }

  virtual bool _inCO(const ValueRef& v, const ValueRef& f,
                     const ValueRef& t) const {
    return (((v->compareTo(f) >= 0) && (v->compareTo(t) <  0)) ||
            ((t->compareTo(f) <= 0) && (v->compareTo(f) >= 0)) ||
            ((v->compareTo(t) <  0) && (t->compareTo(f) <= 0)));
  }

  virtual bool _inCC(const ValueRef& v, const ValueRef& f,
                     const ValueRef& t) const {
    return (((v->compareTo(f) >= 0) && (v->compareTo(t) <= 0)) ||
            ((t->compareTo(f) <= 0) && (v->compareTo(f) >= 0)) ||
            ((v->compareTo(t) <= 0) && (t->compareTo(f) <= 0)));
  }
};

template <class T, class B> class OperImpl : public OperCompare<T> { 
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
    B b1 = T::cast(v1);
    B b2 = T::cast(v2);
    return T::mk(b1 << b2);
  };
  virtual ValuePtr _rshift (const ValueRef& v1, const ValueRef& v2) const {
    B b1 = T::cast(v1);
    B b2 = T::cast(v2);
    return T::mk(b1 >> b2);
  };
  virtual ValuePtr _mod (const ValueRef& v1, const ValueRef& v2) const {
    B b1 = T::cast(v1);
    B b2 = T::cast(v2);
    return T::mk(b1 % b2);
  };
#endif

  virtual ValuePtr _plus (const ValueRef& v1, const ValueRef& v2) const {
    B b1 = T::cast(v1);
    B b2 = T::cast(v2);
    return T::mk(b1 + b2);
  };
  virtual ValuePtr _minus (const ValueRef& v1, const ValueRef& v2) const {
    B b1 = T::cast(v1);
    B b2 = T::cast(v2);
    return T::mk(b1 - b2);
  };
  virtual ValuePtr _times (const ValueRef& v1, const ValueRef& v2) const {
    B b1 = T::cast(v1);
    B b2 = T::cast(v2);
    return T::mk(b1 * b2);
  };
  virtual ValuePtr _divide (const ValueRef& v1, const ValueRef& v2) const {
    B b1 = T::cast(v1);
    B b2 = T::cast(v2);
    return T::mk(b1 / b2);
  };

};

#endif /* __OPER_H_ */
