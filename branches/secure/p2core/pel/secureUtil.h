/*
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 * DESCRIPTION: Utility classes for compile process
 *
 */
#ifndef __SECUREUTIL_H__
#define __SECUREUTIL_H__

#include <ostream>
#include <set>
#include "list.h"
#include "set.h"
#include "fdbuf.h"
#include "compileUtil.h"

namespace compile {

  
  namespace secure {
    class Primitive {
      
    public:
      enum combinationAxis{SPKR=0, RCVR, VERIFIER};
      
      Primitive(SetPtr _p, SetPtr _r, uint32_t _k, SetPtr _v):p(_p), r(_r), v(_v){
	k = _k;
      }

      Primitive* combine(const Primitive *p2, int axis) const;
      
      bool smaller(const Primitive *p2) const;
      
      int compareTo(const Primitive *p2) const;
      
    private:
      SetPtr p;
      SetPtr r;
      SetPtr v;
      uint32_t k;
    };
    
    struct ltPrimitive
    {
      bool operator()(const Primitive* s1, const Primitive* s2) const
      {
	return s1->compareTo(s2) < 0 ;
      }
    };
    
    typedef std::set<Primitive*, ltPrimitive> PrimitiveSet;
    
    class SecurityAlgorithms{
    public:
      const static int hashSize;
      const static int keyPos;
      const static int primitivePos;

      enum encryptionType{RSA=0, AES};
      enum keyType{RSAPriv=0, RSAPub, AESSecret};
       
      static ValuePtr generate(ValuePtr msg, uint32_t encType, ValuePtr key);
      static bool verify(ValuePtr msg, ListPtr proof, Primitive *p);

      static ValuePtr readFromFile(std::string filename);
    
      static void writeToFile(std::string filename, ValuePtr f);

    private:
      //helper function
      
      static bool stronger(Primitive *p, PrimitiveSet &pSet);      
      static ValuePtr signAES(ValuePtr msg, ValuePtr key);
      static ValuePtr signRSA(ValuePtr msg, ValuePtr key);
      static bool verifyAES(ValuePtr msg, ValuePtr key, ValuePtr proof);
      static bool verifyRSA(ValuePtr msg, ValuePtr key, ValuePtr proof);
      
    };
    
  };
  
};

#endif
