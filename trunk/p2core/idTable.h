
// @Author: Vik Singh
// @Date: 1/19/2004

#ifndef __IDTABLE_H__
#define __IDTABLE_H__

#include <string>
#include <bitset>
#include <utility>
#include <algorithm>

#include <async.h>
#include <assert.h>
#include <ihash.h>

class Id;
class IdTable;

/** --------------------------------------------------------------
 *  @CLASS: IdHash
 *  @DESCRIPTION: Wraps up a ref counted pointer to @Id, along with
 *                its ihash_entry and hashcode. @IdHash gets
 *                stored in the ihash
 *  -------------------------------------------------------------- */
struct IdHash {
  inline void idRemove ();
  ihash_entry <IdHash> id_hashLink;
  size_t hashCode;

  IdTable * table;

  ref<Id> id;
  IdHash (ref<Id> idPtr, IdTable * table);
};

/** --------------------------------------------------------------
 *  @CLASS: Id
 *  @DESCRIPTION: Represents 160-bit key value, provides functions
 *                to retrieve hex/bit representations and words
 *  @NOTE: @IdTable is a FRIEND since it uses the private member
 *         function refcount_getcnt () for garbage collection
 *  -------------------------------------------------------------- */
class Id : private virtual refcount {
  friend class IdTable;
  private:
    static ref<Id> mkIdRef (const u_int32_t *);
    static ref<Id> mkIdRef (const std::string&);
    
    static std::bitset<160> toBitWord (u_int32_t num);
    static std::string toHexWord (u_int32_t num);

    IdHash * hashWrapper;

    // key represented as 5 @u_int32_t words
    u_int32_t key[5];

  public:
    // constructor should be private but that doesn't play
    // nicely with @refcounted
    Id () {}
    inline size_t getHash () const;
    std::bitset<160> toBitSet () const;
    std::string toHexString () const;
    inline u_int32_t getWord (const int) const;

    // remove the wrapper @IdHash from the ihash and delete my ptr
    // check if @finalize needs to be private
    void finalize () { hashWrapper->idRemove (); delete this; }

    // static Id * unmarshal (const XDR *);
};

/** --------------------------------------------------------------
 *  @CLASS: IdTable
 *  @DESCRIPTION: Stores and provides functions to memoize @Id
 *                objects, ensuring only one memory address
 *                pertains to each @Id bit pattern
 *  -------------------------------------------------------------- */
class IdTable {
  public:
    IdTable ()   { threshold = numOfInserts = 0; }
    ~IdTable ()	 { clear ();     }

    inline void initialize_gc (const size_t);
    
    // @create: factory functions to create and memoize @Id's.
    // if the requested @Id is already stored, the function
    // returns the ref<Id> to the existing @Id
    ref<Id> create (const std::string&);

    ref<Id> create (const u_int32_t *);

    // create a random @Id using /dev/urandom
    ref<Id> create ();

    // static Id * create (const XDR *);

    inline size_t size () const;

    inline void remove (IdHash *);
    inline void clear ();

  private:
    ihash< size_t, IdHash, &IdHash::hashCode, &IdHash::id_hashLink >
      memTable;
    size_t threshold;
    size_t numOfInserts;

    void gc ();

    // @storeId performs memoization
    ref<Id> storeId (ref<Id>);

    void add (IdHash *);
};

#endif /** !__IDTABLE_H__ */

