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
#include "value.h"
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


ID::ID(std::string hexString)
{
  static const std::string zeros(WORDS * 4, '0');
  
  // Read it in in chunks of 4-byte words from right to left. If it's
  // two long, discard any extraneous prefix to leave WORDS-worth of
  // suffix characters
  unsigned position = hexString.length();
  if (position > (WORDS * 4)) {
    // This string is too long. Chop it down
    hexString = hexString.substr(position - (WORDS * 4), WORDS * 4);
    position = WORDS * 4;
  }

  // Now we have at most WORDS*4 characters but perhaps fewer.
  if (position < WORDS * 4) {
    // I don't have enough characters. Prepend 0's
    hexString = zeros.substr(0, WORDS * 4 - position) + hexString;
    position = WORDS * 4;
  }

  // Now we have exactly WORDS*4 characters.
  int wordIndex = WORDS - 1;
  for (;
       wordIndex >= 0;
       position -= 4,
         wordIndex--) {
    // Pick the last 4-char suffix
    std::string suffix = hexString.substr(position - 4, 4);
    char* endPointer;
    words[wordIndex] = (uint32_t) strtoull(suffix.c_str(),
                                           &endPointer,
                                           16);
    if (*endPointer != '\0') {
      // Decoding wasn't happy.  As much of a suffix as possible and pad
      // the rest
      while (*endPointer != '\0') {
        words[wordIndex] = (uint32_t) strtoull(endPointer + 1,
                                               &endPointer,
                                               16);
      }

      // Take what we have and stop here. Fill in
      // the remainder with 0s
      wordIndex--;
      while (wordIndex >= 0) {
        words[wordIndex] = 0;
        wordIndex--;
      }
    }
  }
  
  // Now I've filled in my words array and I can move on with life.
}


/** Turns to hexadecimal */
string
ID::toString() const
{ 
  char buf[30];
  string result;
  for (unsigned i = 0;
       i < WORDS;
       i++) {
    sprintf(buf, "%04x", words[i]);
    result += buf;
  }
  return result;
}


string ID::toConfString() const
{ 
  ostringstream result;
  char buf[30];
  for (unsigned i = 0; i < WORDS; i++) {
    sprintf(buf, "%04x", words[i]);
    result << buf;
  }
  return result.str();
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

std::ostream&
operator <<(std::ostream& os, const ID& object)
{
  os << object.toString();
  return os;
}

