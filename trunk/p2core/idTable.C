
#include "idTable.h"

// @Author: Vik Singh
// @Date: 12/30/2004

// @CLASS: Id

Id::Id (IdTable * table) { this->table = table; }

Id::Id (const uint32_t * num, IdTable * table) {
  for (int i = 0; i < 5; i += 1)
    key[i] = *(num + i);
  this->table = table;
}

Id::Id (const std::string& idString, IdTable * table) {
  this->table = table;
}

void Id::finalize () {
  table->remove (this);
}

std::bitset<160> Id::toBitWord (uint32_t num) {
  return std::bitset<160> (num);
  /* std::bitset<160> result;
    for (int i = 0; i < 32 && num >= 1; i += 1) {
      if (num % 2)
        result.set (i);
      num /= 2;
    }
  return result; */
}

std::string Id::toHexWord (uint32_t num) {
  const std::string HEX_DIGITS = "0123456789ABCDEF";
  char result[] = {'0','0','0','0','0','0','0','0','\0'};
  int remainder;
  for (int i = 0; i < 8 && num >= 1; i += 1) {
    if (remainder = num % 16)
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

uint32_t Id::getWord (const int index) const {
  assert (index >= 0 && index < 5);
  return key[index];
}

// CLASS: IdTable

IdTable::IdTable () { maxSize = 0; }

void IdTable::initialize_gc (const size_t num) {
  maxSize = num;
}

bool IdTable::equalByVal (const Id * x, const Id * y) {
  for (int i = 0; i < 5; i += 1) {
    if (x->key[i] == y->key[i]) continue;
    else return false;
  }
  return true;
}

// const Id * create (const XDR * xdr);

const Id * IdTable::create (const std::string& idString) {
  Id * result = new Id (this); // use New ref<Id> (idString);
  return storeId (result);
}

const Id * IdTable::create (const uint32_t * random) {
  Id * result = new Id (random, this); // use New ref<Id> (random);
  return storeId (result);
}

const Id * IdTable::create () {
  Id * result = new Id (this); // auto gen with dev/urandom
  return storeId (result);
}

size_t IdTable::size () const {
  return idSet.size ();
}

void IdTable::remove (const Id * idPtr) {
  idSet.erase (idPtr);
}

void IdTable::clear () {
  idSet.clear ();
}

const Id * IdTable::storeId (const Id * idKey) {
  hash_set<const Id *, IdHashCode, IdComparator>::iterator
    idFound = idSet.find (idKey);
  if (idFound != idSet.end ()) {
    delete idKey;
    return *idFound;
  }
  else {
    add (idKey);
    return idKey;
  }
}

void IdTable::add (const Id * idKey) {
  idSet.insert (idKey);
  if (maxSize == 0) return;
  else if (idSet.size () > maxSize)
    gc ();
}

// ref<Id> 's are deleted automatically from the set when
// refcount goes to zero - look at Id::finalize ()
void IdTable::gc () {
  /**
   * Get the size of @idSet
   * divide size by 2
   * first half = old
   * second half = recent
   * fudge factor = desired size, keep collecting until ff
   * reached
   * within this set order @Id's by # of references
   * within this set order @Id's by time of last use
   *
   */
  hash_set<const Id *, IdHashCode, IdComparator>::iterator items =
    idSet.begin ();
  while (items != idSet.end ()) {
  }
}

