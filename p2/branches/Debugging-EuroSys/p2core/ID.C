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
  words[WORDS - 1] = doubleword & 0xFFFFFFFF;
  words[WORDS - 2] = (doubleword >> 32) & 0xFFFFFFFF;
}

/** Turns to hexadecimal */
string ID::toString() const
{ 
  char buf[30];
  string result;
  for (unsigned i = 0;
       i < WORDS;
       i++) {
    sprintf(buf, "%08x", words[i]);
    result += buf;
  }
  return result;
}

int
ID::compareTo(IDPtr other) const
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
ID::betweenOO(IDPtr from, IDPtr to) const
{
  return (((compareTo(from) > 0) && (compareTo(to) < 0)) ||
          ((to->compareTo(from) <= 0) && (compareTo(from) > 0)) ||
          ((compareTo(to) < 0) && (to->compareTo(from) <= 0)));
}

bool
ID::betweenOC(IDPtr from, IDPtr to) const
{
  return (((compareTo(from) > 0) && (compareTo(to) <= 0)) ||
          ((to->compareTo(from) <= 0) && (compareTo(from) > 0)) ||
          ((compareTo(to) <= 0) && (to->compareTo(from) <= 0)));
}

bool
ID::betweenCO(IDPtr from, IDPtr to) const
{
  return (((compareTo(from) >= 0) && (compareTo(to) < 0)) ||
          ((to->compareTo(from) <= 0) && (compareTo(from) >= 0)) ||
          ((compareTo(to) < 0) && (to->compareTo(from) <= 0)));
}

bool
ID::betweenCC(IDPtr from, IDPtr to) const
{
  return (((compareTo(from) >= 0) && (compareTo(to) <= 0)) ||
          ((to->compareTo(from) <= 0) && (compareTo(from) >= 0)) ||
          ((compareTo(to) <= 0) && (to->compareTo(from) <= 0)));
}

IDPtr
ID::distance(IDPtr to) const
{
  IDPtr newID = ID::mk();
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

IDPtr
ID::shift(uint32_t shift) const
{
  if (shift == 0) {
    return ID::mk((uint32_t*) words);
  }
  if (shift >= WORDS * 32) {
    return ID::ZERO;
  }

  IDPtr newID = ID::mk();

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

IDPtr
ID::add(IDPtr other) const
{
  IDPtr newID = ID::mk();
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
    xdr_uint32_t(x, &(words[i]));
  }
}

IDPtr
ID::xdr_unmarshal(XDR *x)
{
  IDPtr newID(new ID());
  for (uint i = 0;
       i < WORDS;
       i++) {
    xdr_uint32_t(x, &(newID->words[i]));
  }
  return newID;
}

IDPtr
ID::ZERO(ID::mk((uint32_t) 0));

IDPtr
ID::ONE(ID::mk((uint32_t) 1));

