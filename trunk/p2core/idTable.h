
// @Author: Vik Singh
// @Date: 12/30/2004

#ifndef __IDTABLE_H__
#define __IDTABLE_H__

#include <string>
#include <bitset>
#include <map>
#include <utility>
#include <algorithm>
#include <async.h>
#include <assert.h>


class IdTable;
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
    static std::bitset<160> toBitWord (u_int32_t num);
    static std::string toHexWord (u_int32_t num);

    // key represented as 5 @u_int32_t words
    u_int32_t key[5];

  public:
    Id ();
    Id (const u_int32_t *);
    Id (const std::string&);

    std::bitset<160> toBitSet () const;
    std::string toHexString () const;
    inline size_t hashCode () const;
    inline u_int32_t getWord (const int) const;

    // static Id * unmarshal (const XDR *);
};

/** --------------------------------------------------------------
 *  @STRUCT: IdComparator
 *  @DESCRIPTION: Represents a function object to compare the
 *                equality of two @Id objects
 *  -------------------------------------------------------------- */
struct IdComparator {
  bool operator () (ref<Id> x, ref<Id> y) const {
    return (x == y);
  }
};

/** --------------------------------------------------------------
 *  @CLASS: IdTable
 *  @DESCRIPTION: Stores and provides functions to memoize @Id
 *                objects, ensuring only one memory address
 *                pertains to each @Id bit pattern
 *  -------------------------------------------------------------- */
class IdTable {
  public:
    IdTable ()   { maxSize = threshold = 0; }
    ~IdTable ()	 { clear ();     }

    inline void initialize_gc (const size_t, const size_t);
    
    // @create: factory functions to create and memoize @Id's.
    // if the requested @Id already stored, the function returns
    // the ref<Id> to the existing @Id
    ref<Id> create (const std::string&);

    ref<Id> create (const u_int32_t *);

    // create a random @Id using /dev/urandom
    ref<Id> create ();

    // static Id * create (const XDR *);

    inline size_t size () const;
    inline void remove (ref<Id>);
    inline void clear ();

  private:
    std::map< ref<Id>, int, IdComparator > memTable;
    size_t maxSize;
    size_t threshold;

    void gc ();
    ref<Id> storeId (ref<Id>);
    void add (ref<Id>);
};

#endif /** !__IDTABLE_H__ */

