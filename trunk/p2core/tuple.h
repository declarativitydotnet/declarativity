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
#include "value.h"

class Tuple;
typedef ref<Tuple> TupleRef;
typedef ptr<Tuple> TuplePtr;

class Tuple {

private:
  std::vector<ValueRef> fields;
  bool		frozen;

public:

  Tuple() : fields(), frozen(false) {};
  static TupleRef mk() { return New refcounted<Tuple>(); };

  void append(ValueRef tf) { assert(!frozen); fields.push_back(tf); };
  void freeze() { frozen = true; };

  size_t size() const { return fields.size(); };

  ValueRef operator[] (ptrdiff_t i) { return fields[i]; };
  const ValueRef operator[] (ptrdiff_t i) const { return fields[i]; };

  void xdr_marshal( XDR *uio );
  static TupleRef xdr_unmarshal( XDR *uio );

  str toString() const;
};

#endif /* __TUPLE_H_ */
