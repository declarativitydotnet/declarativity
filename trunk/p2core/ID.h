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
 * DESCRIPTION: An opaque ID.  Operations on it with scalars happen
 * modulo its size.
 *
 */

#ifndef __ID_H__
#define __ID_H__

#include "inlines.h"
#include <async.h>
#include <arpc.h>

class ID;
typedef ref< ID > IDRef;
typedef ptr< ID > IDPtr;

class ID {
private:
  /** My number of 4-byte words in this ID.  This could eventually
      become a template parameter, but not for now. */
  static const int WORDS = 5;

  /** My representation of an ID.   */
  uint32_t words[WORDS];

public:  
  ID();

  ID(uint32_t[WORDS]);

  ID(uint32_t);

  ID(uint64_t);

  str toString() const;

  /** Strict equality */
  REMOVABLE_INLINE bool equals(IDRef other) const { return compareTo(other) == 0; }

  /** Am I less than, equal or greater than the other value?  -1 means
      less, 0 means equal, +1 means greater.  This is done strictly on
      the numerical space, i.e., not on the ring. */
  int compareTo(IDRef) const;

  /** The minimum directional distance from me to the given ID on the
      ring. */
  IDRef distance(IDRef) const;

  /** Add two IDs together arithmetically */
  IDRef add(IDRef) const;

  /** Shift an ID by a number of bit positions */
  IDRef shift(uint32_t) const;

  /** Marshal into an XDR, one word at a time */
  void xdr_marshal(XDR *x);

  /** Create an ID from an XDR buffer, one word at a time */
  static IDRef xdr_unmarshal(XDR *x);

  /** Create an ID */
  static IDRef mk() { return New refcounted< ID >(); }
  static IDRef mk(uint32_t u) { return New refcounted< ID >(u); }
  static IDRef mk(uint64_t u) { return New refcounted< ID >(u); }
};

#endif /* __ID_H_ */
