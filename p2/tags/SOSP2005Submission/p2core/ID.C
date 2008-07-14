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
 */

#include "ID.h"


ID::ID()
{
  for (unsigned i = 0;
       i < WORDS;
       i++) {
    words[i] = 0;
  }
}

ID::ID(uint32_t newWords[ID::WORDS])
{
  for (unsigned i = 0;
       i < WORDS;
       i++) {
    words[i] = newWords[i];
  }
}

ID::ID(uint32_t word)
{
  for (unsigned i = 0;
       i < WORDS;
       i++) {
    words[i] = 0;
  }
  words[WORDS - 1] = word;
}

ID::ID(uint64_t doubleword)
{
  for (unsigned i = 0;
       i < WORDS;
       i++) {
    words[i] = 0;
  }
  words[WORDS - 1] = doubleword | 0xFFFFFFFF;
  words[WORDS - 2] = (doubleword >> 32) | 0xFFFFFFFF;
}

/** Turns to hexadecimal */
str
ID::toString() const
{
  strbuf result;
  for (unsigned i = 0;
       i < WORDS;
       i++) {
    result.fmt("%08x", words[i]);
  }
  return result;
}

int
ID::compareTo(IDRef other) const
{
  for (unsigned i = 0;
       i < WORDS;
       i++) {
    if (words[i] < other->words[i]) {
      return -1;
    }
    if (words[i] > other->words[i]) {
      return 1;
    }
  }
  return 0;
}

bool
ID::betweenOO(IDRef from, IDRef to) const
{
  return (((compareTo(from) > 0) && (compareTo(to) < 0)) ||
          ((to->compareTo(from) <= 0) && (compareTo(from) > 0)) ||
          ((compareTo(to) < 0) && (to->compareTo(from) <= 0)));
}

bool
ID::betweenOC(IDRef from, IDRef to) const
{
  return (((compareTo(from) > 0) && (compareTo(to) <= 0)) ||
          ((to->compareTo(from) <= 0) && (compareTo(from) > 0)) ||
          ((compareTo(to) <= 0) && (to->compareTo(from) <= 0)));
}

bool
ID::betweenCO(IDRef from, IDRef to) const
{
  return (((compareTo(from) >= 0) && (compareTo(to) < 0)) ||
          ((to->compareTo(from) <= 0) && (compareTo(from) >= 0)) ||
          ((compareTo(to) < 0) && (to->compareTo(from) <= 0)));
}

bool
ID::betweenCC(IDRef from, IDRef to) const
{
  return (((compareTo(from) >= 0) && (compareTo(to) <= 0)) ||
          ((to->compareTo(from) <= 0) && (compareTo(from) >= 0)) ||
          ((compareTo(to) <= 0) && (to->compareTo(from) <= 0)));
}

IDRef
ID::distance(IDRef to) const
{
  IDRef newID = ID::mk();
  uint32_t carry = 0;
  for (int i = (int) WORDS - 1;
       i >= 0;
       i--) {
    if (to->words[i] >=
        (words[i] + carry)) {
      newID->words[i] =
        to->words[i] - words[i] - carry;
      if (carry == 1) {
        if ((words[i] + carry) == 0) {
          carry = 1;
        } else {
          carry = 0;
        }
      }
    } else {
      newID->words[i] =
        to->words[i] - words[i] - carry;
      carry = 1;
    }
  }
  return newID;
}

IDRef
ID::shift(uint32_t shift) const
{
  if (shift == 0) {
    return ID::mk((uint32_t*) words);
  }
  if (shift >= WORDS * 32) {
    return ID::ZERO;
  }

  IDRef newID = ID::mk();

  // Perform long shifts (i.e., by bytes, not by bits)
  if (shift > 32) {
    // By how many entire bytes (at most)?
    uint32_t longShift = shift >> 5;
    for (uint i = 0;
         i < WORDS - longShift;
         i++) {
      newID->words[i] = words[i + longShift];
    }
    shift = shift & 0x1f;
  } else {
    for (uint i = 0;
         i < WORDS;
         i++) {
      newID->words[i] = words[i];
    }
  }
  
  // Now we only have short shifts
  uint32_t carry = 0;
  for (int i = (int) WORDS-1;
       i >= 0;
       i--) {
    uint64_t temp = newID->words[i];
    temp = temp << shift;
    temp = temp | carry;
    carry = temp >> 32;
    newID->words[i] = (temp & 0xFFFFFFFF);
  }

  return newID;
}

IDRef
ID::add(IDRef other) const
{
  IDRef newID = ID::mk();
  uint32_t carry = 0;
  for (int i = (int) WORDS - 1;
       i >= 0;
       i--) {
    uint64_t temp = words[i];
    temp += other->words[i];
    temp += carry;
    if (temp > UINT_MAX) {
      newID->words[i] = temp & 0xffffffff;
      carry = 1;
    } else {
      newID->words[i] = temp;
      carry = 0;
    }
  }
  return newID;
}

void
ID::xdr_marshal(XDR *x)
{
  for (uint i = 0;
       i < WORDS;
       i++) {
    xdr_int32_t(x, &(words[i]));
  }
}

IDRef
ID::xdr_unmarshal(XDR *x)
{
  IDRef newID = New refcounted< ID >();
  for (uint i = 0;
       i < WORDS;
       i++) {
    xdr_int32_t(x, &(newID->words[i]));
  }
  return newID;
}

IDRef
ID::ZERO(ID::mk((uint32_t) 0));

IDRef
ID::ONE(ID::mk((uint32_t) 1));

