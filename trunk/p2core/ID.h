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
 * DESCRIPTION: An opaque ID.  Operations on it with scalars happen
 * modulo its size.
 *
 */

#ifndef __ID_H__
#define __ID_H__

#include <rpc/xdr.h>
#include <string>
#include <sstream>
#include <boost/shared_ptr.hpp>
#include "inlines.h"

using std::ostringstream;

class ID;
typedef boost::shared_ptr< ID > IDPtr;

class ID {
public:
  /** My number of 4-byte words in this ID.  This could eventually
      become a template parameter, but not for now. */
  static const unsigned WORDS = 5;

  /** ONE */
  static IDPtr ONE;

  /** ZERO */
  static IDPtr ZERO;
  
private:
  /** My representation of an ID.   */
  uint32_t words[WORDS];
  
public:  
  ID();

  
  ID(uint32_t[WORDS]);
  
  
  ID(uint32_t);
  
  
  ID(uint64_t);
  
  
  ID(std::string);


  string
  toString() const;

  
  string
  toConfString() const;

  /** Strict equality */
  REMOVABLE_INLINE bool
  equals(IDPtr other) const { return compareTo(other) == 0; }


  /** Am I less than, equal or greater than the other value?  -1 means
      less, 0 means equal, +1 means greater.  This is done strictly on
      the numerical space, i.e., not on the ring. */
  int
  compareTo(IDPtr) const;


  /** Am I in (from, to)? */
  bool
  betweenOO(IDPtr, IDPtr) const;


  /** Am I in (from, to]? */
  bool
  betweenOC(IDPtr, IDPtr) const;


  /** Am I in [from, to)? */
  bool
  betweenCO(IDPtr, IDPtr) const;


  /** Am I in [from, to]? */
  bool
  betweenCC(IDPtr, IDPtr) const;


  /** The minimum directional distance from me to the given ID on the
      ring. */
  IDPtr
  distance(IDPtr) const;


  /** Add two IDs together arithmetically */
  IDPtr
  add(IDPtr) const;


  /** Bitwise AND two IDs together */
  IDPtr
  bitwiseAND(IDPtr) const;


  /** Bitwise OR two IDs together */
  IDPtr
  bitwiseOR(IDPtr) const;


  /** Bitwise XOR two IDs together */
  IDPtr
  bitwiseXOR(IDPtr) const;


  /** Bitwise NOT of an ID */
  IDPtr
  bitwiseNOT() const;


  /** Left shift an ID by a number of bit positions */
  IDPtr
  lshift(uint32_t) const;


  /** Right shift an ID by a number of bit positions */
  IDPtr
  rshift(uint32_t) const;


  /** Marshal into an XDR, one word at a time */
  void
  xdr_marshal(XDR *x);


  /** Create an ID from an XDR buffer, one word at a time */
  static IDPtr
  xdr_unmarshal(XDR *x);


  /** Create an ID */
  static IDPtr mk()                  { IDPtr p(new ID());  return p; }
  static IDPtr mk(uint32_t w[WORDS]) { IDPtr p(new ID(w)); return p; }
  static IDPtr mk(uint32_t u)        { IDPtr p(new ID(u)); return p; }
  static IDPtr mk(uint64_t u)        { IDPtr p(new ID(u)); return p; }
  static IDPtr mk(std::string s)     { IDPtr p(new ID(s)); return p; }
};

/** The stream operator */
std::ostream&
operator <<(std::ostream& os, const ID& object);

#endif /* __ID_H_ */
