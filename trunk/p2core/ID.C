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
  for (int i = 0;
       i < WORDS;
       i++) {
    words[i] = 0;
  }
}

ID::ID(uint32_t newWords[ID::WORDS])
{
  for (int i = 0;
       i < WORDS;
       i++) {
    words[i] = newWords[i];
  }
}

ID::ID(uint32_t word)
{
  for (int i = 0;
       i < WORDS;
       i++) {
    words[i] = 0;
  }
  words[WORDS - 1] = word;
}

ID::ID(uint64_t doubleword)
{
  for (int i = 0;
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
  for (int i = 0;
       i < WORDS;
       i++) {
    result << hexdump(&(words[i]), sizeof(uint32_t));
  }
  return result;
}

int
ID::compareTo(IDRef other) const
{
  for (int i = 0;
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

IDRef
ID::distance(IDRef to) const
{
  IDRef newID = ID::mk();
  uint32_t carry = 0;
  for (int i = WORDS - 1;
       i >= 0;
       i--) {
    if (to->words[i] >=
        (words[i] + carry)) {
      newID->words[i] =
        to->words[i] - words[i] - carry;
      if ((words[i] + carry) == 0) {
        carry = 1;
      } else {
        carry = 0;
      }
    } else {
      newID->words[i] =
        UINT_MAX - words[i] - carry + to->words[i];
      carry = 1;
    }
  }
  return newID;
}

IDRef
ID::shift(uint32_t shift) const
{
  IDRef newID = ID::mk();
  uint32_t carry = 0;
  for (int i = 0;
       i < WORDS;
       i++) {
    uint64_t temp = words[i];
    temp = temp << shift;
    temp = temp | carry;
    newID->words[i] = (temp & 0xFFFFFFFF);
    carry = temp >> 32;
  }

  return newID;
}

IDRef
ID::add(IDRef other) const
{
  IDRef newID = ID::mk();
  uint32_t carry = 0;
  for (int i = WORDS - 1;
       i >= 0;
       i--) {
    uint64_t temp = words[i] + other->words[i] + carry;
    if (temp > UINT_MAX) {
      newID->words[i] = temp - UINT_MAX;
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
  for (int i = 0;
       i < WORDS;
       i++) {
    xdr_int32_t(x, &(words[i]));
  }
}

IDRef
ID::xdr_unmarshal(XDR *x)
{
  IDRef newID = New refcounted< ID >();
  for (int i = 0;
       i < WORDS;
       i++) {
    xdr_int32_t(x, &(newID->words[i]));
  }
  return newID;
}

