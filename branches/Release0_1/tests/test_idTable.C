
// @Author: Vik Singh
// @Date: 1/20/2004

/** IDTABLE TESTS
 *  Run the executable with an optional
 *  integer arg representing the number of
 *  Id's to use in the test cases. */

#include "idTable.h"
#include "random.h"

#define DEFAULT_ID_SIZE 10

// Returns a randomly generated Id key
// Retrieves random numbers from /dev/urandom
uint32_t key[5];
uint32_t * randomKey () {
  for (int i = 0; i < 5; i += 1)
    key[i] = random::uint32t_urand ();
  return key;
}

// Simply creates a bunch of Id's in a new
// IdTable memoizing structure 
void generateIds (u_int count) {
  IdTable * table = New IdTable ();
  for (u_int i = 0; i < count; i += 1)
    table->create (randomKey ());
  delete table;
}

// Checks if the toBitSet function of Id works
// properly
void
bitSetTests(u_int count)
{
  IdTable * table = New IdTable ();
  for (u_int i = 0;
       i < count;
       i++) {
    ref<Id> idPtr = table->create(randomKey());
    std::bitset<160> idBitSet = idPtr->toBitSet();
    for (int j = 0;
         j < 5;
         j++) {
      std::bitset<32> idWord = std::bitset<32>(idPtr->getWord(j));
      for (int k = 0;
           k < 32;
           k++) {
        if (idWord[k] == idBitSet[(k + (32 * j))]) {
        } else {
          warn << "BITSET TEST FAILED";
          delete table;
          return;
        }
      } // end INNER
    }
  } // end TOP
  
  delete table;
}

// Checks if the toHexString function of Id
// works properly
void
hexTests(u_int count)
{
  IdTable * table = New IdTable();
  str hexWord;
  for (u_int i = 0;
       i < count;
       i++) {
    ref<Id> idPtr = table->create(randomKey());
    str idString = idPtr->toHexString();
    for (int j = 0;
         j < 5;
         j++) {
      uint32_t theWord = idPtr->getWord(j);
      hexWord = str(strbuf() << hexdump((void*) &theWord, sizeof(uint32_t))).cstr();
      //      hexWord = toHexWord(idPtr->getWord(j));
      if (hexWord.cmp(substr(idString, (8 * j), 8))) 
        continue;
      else {
        warn << "HEXSTRING TEST FAILED FOR STR " << idString <<
          " and on WORD " << hexWord;
        delete table;
        return;
      }
    }
  }
  delete table;
}

// With a few cases, checks if IdTable actually memoizes and returns the
// ref<Id> of an already stored Id when asked to memoize the same bit
// pattern more than once
void doesItMemoize() {
  IdTable * table = New IdTable ();
  uint32_t key1[5];
  for (int i = 0; i < 5; i += 1) {
    key1[i] = 2 * i;
  }
  uint32_t key2[5];
  for (int i = 0; i < 5; i += 1) {
    key2[i] = key1[i];
  }
  ref<Id> idPtr1 = table->create(key1);
  ref<Id> idPtr2 = table->create(key2);
  if (!(idPtr1 == idPtr2 && table->size() == 1))
    warn << "FAILED TO MEMOIZE TWO SAME ID's\n";
  
  delete table;
}

// Checks the performance of memoizing many Id's
void memoizeLotsOfIds (u_int count) {
}

// Checks if Id's get removed after several
// insertions - 30 triggers a new GC cycle
void doesItGc () {
}

// Checks the performance of GCing
void lotsOfIdsToGc (u_int count) {
}

std::bitset<160> toBitWord (u_int32_t num) {
  return std::bitset<160> (num);
  /* std::bitset<160> result;
    for (int i = 0; i < 32 && num >= 1; i += 1) {
      if (num % 2)
        result.set (i);
      num /= 2;
    }
  return result; */
}

str toHexWord(u_int32_t num) {
  static str HEX_DIGITS = "0123456789ABCDEF";
  char result[] = {'0','0','0','0','0','0','0','0','\0'};
  int remainder;
  for (int i = 0; i < 8 && num >= 1; i += 1) {
    if ((remainder = num % 16))
      result[7 - i] = HEX_DIGITS[remainder];
    num /= 16;
  }
  return str(result);
}

int main (int argc, char * arg[]) {
  u_int idCount = 0;

  // CHECK IF VALID PROGRAM ARG
  if (argc > 1) {
    if ((idCount = atoi (arg[1])));
    else {
      fatal << "Invalid ARG: " << arg[1] <<
	". Please supply an INT value\n";
      exit(-1);
    }
  }

  // USER DID NOT SUPPLY ARG, SO USE DEFAULT
  if (idCount)
    idCount = DEFAULT_ID_SIZE;

  // @ID TESTS
  generateIds (idCount);
  bitSetTests (idCount);
  hexTests (idCount);

  // @IDTABLE TESTS
  doesItMemoize ();
  memoizeLotsOfIds (idCount);
  doesItGc ();
  lotsOfIdsToGc (idCount);

  return 0;
}

