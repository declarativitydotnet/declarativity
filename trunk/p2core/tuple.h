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
 * DESCRIPTION:
 *
 */

#ifndef __TUPLE_H__
#define __TUPLE_H__

#include <vector>

#include <assert.h>
#include <async.h>
#include <arpc.h>
#include "inlines.h"

#define CAST(TC,TN,TE) \
	REMOVABLE_INLINE TC as_##TN() { \
          if (TE != t) { throw TupleField::TypeError(); } \
	return TN; \
	}

class TupleField;
typedef ref<TupleField> TupleFieldRef;
typedef ptr<TupleField> TupleFieldPtr;

class TupleField {

public:  
  // Concrete types
  enum Type { NULLV=0, INT32, UINT32, INT64, UINT64, STRING, DOUBLE, INVALID };
  
  struct TypeError {} ;

protected:
  Type t;
  union {
    int32_t	i32;
    uint32_t	ui32;
    int64_t	i64;
    uint64_t	ui64;
    double	d;
  };
  str		s;

public:

  TupleField() : t(NULLV) {};
  TupleField( int32_t i ) : t(INT32), i32(i) {};
  TupleField( uint32_t i ) : t(UINT32), ui32(i) {};
  TupleField( int64_t i ) : t(INT64), i64(i) {};
  TupleField( uint64_t i ) : t(UINT64), ui64(i) {};
  TupleField( str st ) : t(STRING), s(st) {};
  TupleField( double f ) : t(DOUBLE), d(f) {};
  
  REMOVABLE_INLINE Type get_type() const { return t; };

  str toString() const;
  str toTypeString() const ;
  static const char *typeName(Type t); 
  
  //
  // Casts: these are strict type narrowing.  What you probably want
  // is a type conversion function (see below)
  //
  CAST(int32_t,  i32, INT32);
  CAST(u_int32_t,ui32,UINT32);
  CAST(int64_t,  i64, INT64);
  CAST(u_int64_t,ui64,UINT64);
  CAST(str,      s,   STRING);
  CAST(double,   d,   DOUBLE );

  //
  // Type conversion: used for arithmetic 
  //
  bool convert_unsigned(uint64_t &val);
  bool convert_signed(int64_t &val);

  void xdr_marshal( XDR *x );
  static TupleFieldRef xdr_unmarshal( XDR *x );
  
};
#undef CAST  

class Tuple;
typedef ref<Tuple> TupleRef;
typedef ptr<Tuple> TuplePtr;

class Tuple {

private:
  std::vector<TupleFieldRef> fields;
  bool		frozen;

public:

  Tuple() : fields(), frozen(false) {};
  static TupleRef mk() { return New refcounted<Tuple>(); };

  void append(TupleFieldRef tf) { assert(!frozen); fields.push_back(tf); };
  void freeze() { frozen = true; };

  size_t size() const { return fields.size(); };

  TupleFieldRef operator[] (ptrdiff_t i) { return fields[i]; };
  const TupleFieldRef operator[] (ptrdiff_t i) const { return fields[i]; };

  void xdr_marshal( XDR *uio );
  static TupleRef xdr_unmarshal( XDR *uio );

  str toString() const;
};

#endif /* __TUPLE_H_ */
