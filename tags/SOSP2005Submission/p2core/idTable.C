
#include "idTable.h"

// @Author: Vik Singh
// @Date: 1/19/2004


/** --------------------------------------------------------------
 *  @CLASS: Id -- 160 bit id representation with conversion op's
 *  -------------------------------------------------------------- */

ref<Id> Id::mkIdRef (const u_int32_t * num) {
  ref<Id> result = New refcounted<Id> ();
  for (int i = 0; i < 5; i += 1)
    result->key[i] = *(num + i);
  return result;
}

// NEED TO IMPLEMENT
ref<Id> Id::mkIdRef (str idString) {
  ref<Id> result = New refcounted<Id> ();
  return result;
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

str Id::toHexWord(u_int32_t num) {
  static const str HEX_DIGITS = "0123456789ABCDEF";
  char result[] = {'0','0','0','0','0','0','0','0','\0'};
  int remainder;
  for (int i = 0; i < 8 && num >= 1; i += 1) {
    if ((remainder = num % 16))
      result[7 - i] = HEX_DIGITS[remainder];
    num /= 16;
  }
  return str(result);
}

std::bitset<160> Id::toBitSet () const {
  std::bitset<160> result;
  for (int i = 0; i < 5; i += 1)
    result ^= toBitWord (key[i]) <<
      ((4 - i) * 32);
  return result;
}

str
Id::toHexString() const {
  strbuf result;
  for (int i = 0;
       i < 5;
       i++) {
    result << toHexWord(key[i]);
  }
  return str(result);
}

size_t
Id::getHash() const {
  size_t result = 0;
  for (int i = 0; i < 5; i += 1)
    result ^= key[i];
  return result;
}

u_int32_t Id::getWord (const u_int index) const {
  assert (index >= 0 && index < 5);
  return key[index];
}

/** --------------------------------------------------------------
 *  @STRUCT: IdHash - wraps up ref<Id> and hashing attributes
 *  -------------------------------------------------------------- */

IdHash::IdHash (ref<Id> idPtr, IdTable * table)
  : id (New refcounted<Id> ()) {
  id = idPtr;
  this->table = table;
  hashCode = idPtr->getHash ();
}

void IdHash::idRemove() {
  table->remove(this);
}

/** --------------------------------------------------------------
 *  @CLASS: IdTable - memoizes @IdHash or id representations
 *  -------------------------------------------------------------- */

void IdTable::initialize_gc (const size_t thresh = 30) {
  threshold = thresh;
}

// ref<Id> create (const XDR * xdr); look at tuple.C

ref<Id>
IdTable::create(const str idString) {
  ref<Id> result = Id::mkIdRef(idString);
  return storeId(result);
}

ref<Id>
IdTable::create(const u_int32_t * random) {
  ref<Id> result = Id::mkIdRef (random);
  return storeId (result);
}

ref<Id> IdTable::create () {
  u_int32_t random[] = { 0, 0, 0, 0, 0 };
  ref<Id> result = Id::mkIdRef (random);
  return storeId (result);
}

size_t IdTable::size () const {
  return memTable.size ();
}

// NEED TO IMPLEMENT
void IdTable::remove (IdHash * id) {
  memTable.remove (id);
}

void IdTable::clear () {
  memTable.clear ();
}

ref<Id> IdTable::storeId (ref<Id> idKey) {
  IdHash * result = memTable[idKey->getHash ()];
  if (result) {
    delete idKey;
    return result->id;
  }
  else {
    IdHash * idWrapper = New IdHash (idKey, this);
    idKey->hashWrapper = idWrapper;
    add (idWrapper);
    return idKey;
  }
}

void IdTable::add (IdHash * id) {
  memTable.insert (id);
  numOfInserts += 1;
  if (numOfInserts > threshold)
    gc ();
}

void IdTable::gc () {
  /*
  const size_t HALF = size () / 2;
  if (threshold > HALF || threshold == 0)
    threshold = (size_t) (HALF * .20);
  struct IdRefCompare {
    bool operator () (ref<Id>  x, ref<Id> y) const {
      return (x->refcount_getcnt () <
	      y->refcount_getcnt ());
    }
  };

  std::sort (memTable.begin (), memTable.end (), IdRefCompare);

  std::map< ref<Id>, int, IdComparator >::iterator item =
    memTable.begin ();

  for (int i = 0; i < threshold; i += 1, item += 1)
    remove (item->first);
  */
}

