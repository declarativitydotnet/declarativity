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

#include <assert.h>
#include <async.h>
#include <arpc.h>

#define CAST(TC,TN,TE) \
	inline TC as_##TN() { \
          if (TE != t) { throw TupleField::TypeError(); } \
	return TN; \
	}

class TupleField {

public:  
  // Concrete types
  enum Type { I32=0, UI32, I64, UI64, S, D, INVALID };
  
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

  TupleField() : t(INVALID) {};

public:
  
  TupleField( int32_t i ) : t(I32), i32(i) {};
  TupleField( uint32_t i ) : t(UI32), ui32(i) {};
  TupleField( int64_t i ) : t(I64), i64(i) {};
  TupleField( uint64_t i ) : t(UI64), ui64(i) {};
  TupleField( str st ) : t(S), s(st) {};
  TupleField( double f ) : t(D), d(f) {};
  
  Type get_type() const { return t; };

  CAST(int32_t,  i32, I32);
  CAST(u_int32_t,ui32,UI32);
  CAST(int64_t,  i64, I64);
  CAST(u_int64_t,ui64,UI64);
  CAST(str,      s,   S);
  CAST(double,   d,   D );
  
  void xdr_marshal( XDR *x );
  static TupleField *xdr_unmarshal( XDR *x );
  
};
#undef CAST  

class Tuple {

private:
  vec<TupleField> fields;
  bool		finalized;

public:

  Tuple() : fields() {};

  void append(TupleField &tf) { assert(!finalized); fields.push_back(tf); };
  void finalize() { finalized = true; };

  size_t size() const { return fields.size(); };

  TupleField &operator[] (ptrdiff_t i) { return fields[i]; };
  const TupleField &operator[] (ptrdiff_t i) const { return fields[i]; };

  void xdr_marshal( XDR *uio );
  static Tuple *xdr_unmarshal( XDR *uio );

};

#endif /* __TUPLE_H_ */
