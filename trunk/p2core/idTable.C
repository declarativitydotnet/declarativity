
#include "idTable.h"

// @Author: Vik Singh
// @Date: 12/30/2004

//// @CLASS Id

Id::Id () { }

Id::Id (const u_int32_t * num) {
  for (int i = 0; i < 5; i += 1)
    key[i] = *(num + i);
}

Id::Id (const std::string& idString) {
}

std::bitset<160> Id::toBitWord (u_int32_t num) {
  return std::bitset<160> (num);
  /* std::bitset<160> result;
    for (int i = 0; i < 32 && num >= 1; i += 1) {
      if (num % 2)
        result.set (i);
      num /= 2;
    }
  return result; */
}

std::string Id::toHexWord (u_int32_t num) {
  const std::string HEX_DIGITS = "0123456789ABCDEF";
  char result[] = {'0','0','0','0','0','0','0','0','\0'};
  int remainder;
  for (int i = 0; i < 8 && num >= 1; i += 1) {
    if ((remainder = num % 16))
      result[7 - i] = HEX_DIGITS.at (remainder);
    num /= 16;
  }
  return std::string (result);
}

std::bitset<160> Id::toBitSet () const {
  std::bitset<160> result;
  for (int i = 0; i < 5; i += 1)
    result ^= toBitWord (key[i]) <<
      ((4 - i) * 32);
  return result;
}

std::string Id::toHexString () const {
  std::string currentWord;
  char result[] =
  {'0','0','0','0','0','0','0','0','0','0',
   '0','0','0','0','0','0','0','0','0','0',
   '0','0','0','0','0','0','0','0','0','0',
   '0','0','0','0','0','0','0','0','0','0', '\0'};
  for (int i = 0; i < 5; i += 1) {
    currentWord = toHexWord (key[i]);
      for (int j = i * 8; j < (i * 8) + 8; j += 1)
	result[j] = currentWord.at (j % 8);
  }
  return std::string (result);
}

size_t Id::hashCode () const {
  size_t result = 0;
  for (int i = 0; i < 5; i += 1)
    result ^= key[i];
  return result;
}

u_int32_t Id::getWord (const int index) const {
  assert (index >= 0 && index < 5);
  return key[index];
}

//// @CLASS IdTable

void IdTable::initialize_gc (const size_t num, const size_t thresh) {
  maxSize = num;
  threshold = thresh;
}

// const Id * create (const XDR * xdr);

ref<Id> IdTable::create (const std::string& idString) {
  ref<Id> result = New refcounted<Id> (); // use New ref<Id> (idString);
  return storeId (result);
}

ref<Id> IdTable::create (const u_int32_t * random) {
  ref<Id> result = New refcounted<Id> (random); // use New ref<Id> (random);
  return storeId (result);
}

ref<Id> IdTable::create () {
  ref<Id> result = New refcounted<Id> (); // auto gen with dev/urandom
  return storeId (result);
}

size_t IdTable::size () const {
  return memTable.size ();
}

void IdTable::remove (ref<Id> idPtr) {
  memTable.erase (idPtr);
}

void IdTable::clear () {
  memTable.clear ();
}

ref<Id> IdTable::storeId (ref<Id> idKey) {
  std::map< ref<Id>, int, IdComparator >::iterator
    idFound = memTable.find (idKey);
  if (idFound != memTable.end ()) {
    delete idKey;
    return idFound->first;
  }
  else {
    add (idKey);
    return idKey;
  }
}

void IdTable::add (ref<Id> idKey) {
  memTable.insert (std::pair< ref<Id>, int > (idKey, 0));
  if (maxSize == -1) return;
  else if (memTable.size () > maxSize)
    gc ();
}

void IdTable::gc () {
  assert (!memTable.empty ());
  const size_t HALF = size () / 2;
  if (threshold > HALF || threshold == 0)
    threshold = (size_t) (HALF * .20);

  /*
  struct IdRefCompare {
    bool operator () (std::pair< ref<Id>, int > &  x,
	              std::pair< ref<Id>, int > & y) const {
      return (x.first->refcount_getcnt () <
	      y.first->refcount_getcnt ());
    }
  };

  std::sort (memTable.begin (), memTable.end (), IdRefCompare);

  std::map< ref<Id>, int, IdComparator >::iterator item =
    memTable.end ();

  for (int i = 0; i < threshold; i += 1, i += 1, item -= 1)
    remove (item);
  */

}

