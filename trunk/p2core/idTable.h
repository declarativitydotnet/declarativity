
// @Author: Vik Singh
// @Date: 12/30/2004

#ifndef __IDTABLE_H__
#define __IDTABLE_H__

#include <string>
#include <bitset>
#include <functional>
#include <hash_set.h>
#include <assert.h>
#include <stdint.h>


class IdTable;
/** --------------------------------------------------------------
 *  @CLASS: Id
 *  @DESCRIPTION: Represents 160-bit key value, provides functions
 *                to retrieve hex/bit representations and words
 *  @NOTE: Constructors are private since @IdTable is a FRIEND so
 *         all @Id's are created through the factory functions in
 *         @IdTable
 *  -------------------------------------------------------------- */
class Id /* : private virtual refcounted */ {
  friend class IdTable;
  private:
    Id (IdTable *);
    Id (const uint32_t * num, IdTable *);
    Id (const std::string&, IdTable *);

    static std::bitset<160> toBitWord (uint32_t num);
    static std::string toHexWord (uint32_t num);

    // key represented as 5 @uint32_t words
    uint32_t key[5];

  public:
    IdTable * table;
    std::bitset<160> toBitSet () const;
    std::string toHexString () const;
    inline size_t hashCode () const;
    inline uint32_t getWord (const int index) const;
    
    // static Id * unmarshall (const XDR *);
};

/** --------------------------------------------------------------
 *  @STRUCT: IdHashCode
 *  @DESCRIPTION: Represents a function object to compute the hash
 *                code for an @Id object; used in @IdTable
 *  -------------------------------------------------------------- */
struct IdHashCode : public unary_function<const Id *, size_t> {
  size_t operator () (const Id * idKey) const {
    return idKey->hashCode ();
  }
};

/** --------------------------------------------------------------
 *  @STRUCT: IdComparator
 *  @DESCRIPTION: Represents a function object to compare the
 *                equality of two @Id objects
 *  -------------------------------------------------------------- */
struct IdComparator : 
  public binary_function<const Id *, const Id *, bool> {
  bool operator () (const Id * x, const Id * y) const {
    for (int i = 0; i < 5; i += 1) {
      if (x->getWord(i) == y->getWord(i))
	continue;
      else return false;
    }
    return true;
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
    IdTable ();

    static bool equalByVal (const Id *, const Id *);

    inline void initialize_gc (const size_t);
    
    // @create: factory functions to create and memoize @Id's.
    
    // If the requested Id already stored, the function returns
    // the ref<Id> to the existing Id
    const Id * create (const std::string&);

    const Id * create (const uint32_t *);

    // Create a random @Id using /dev/urandom
    const Id * create ();

    // static Id * create (const XDR *);

    inline size_t size () const;
    inline void remove (const Id *);
    inline void clear ();

  private:

    hash_set<const Id *, IdHashCode, IdComparator> idSet;

    size_t maxSize;

    void gc ();
    const Id * storeId (const Id *);
    void add (const Id *);
};

#endif /** !__IDTABLE_H__ */

