#include "secureUtil1.h"
#include "secureUtil.h"
#include "systemTable.h"
#include "val_uint32.h"
#include "val_tuple.h"
#include "val_null.h"
#include "val_str.h"
#include "tuple.h"
#include "plumber.h"

namespace compile {

  namespace secure {

    static uint32_t idCounter = 0;

    ValuePtr generateLocSpec(bool strong){
      std::cout<<"generate loc spec called \n";
      CommonTable::ManagerPtr catalog = Plumber::catalog();
      TuplePtr locSpec = Tuple::mk(LOCSPECTUPLE);
      //      locSpec->append(catalog->nodeid());
      locSpec->append(Val_Str::mk("Hi there!"));
      locSpec->append(Val_UInt32::mk(idCounter++));
      locSpec->append(Val_UInt32::mk(strong?1:0)); // strong or weak
      locSpec->append(Val_Null::mk()); //for the self-certifying hash
      locSpec->freeze();
      return Val_Tuple::mk(locSpec);
    }

    ValuePtr generateVersion(bool strong){
      std::cout<<"generate version called \n";
      CommonTable::ManagerPtr catalog = Plumber::catalog();
      TuplePtr version = Tuple::mk(VERSIONTUPLE);
      //      version->append(catalog->nodeid());
      version->append(Val_Str::mk("Hi there!"));
      version->append(Val_UInt32::mk(idCounter++));
      version->append(Val_UInt32::mk(strong?1:0)); // strong or weak
      version->append(Val_Null::mk()); //for the self-certifying hash
      version->freeze();
      return Val_Tuple::mk(version);
    }

    bool isLocSpec(ValuePtr v){
      std::cout<<"is locspec called with arg " + v->toString() + "\n";
      if(v->typeCode() != Value::TUPLE){
	return false;
      }
      else{
	TuplePtr tuple = Val_Tuple::cast(v);
	return(Val_Str::cast((*tuple)[TNAME]) == LOCSPECTUPLE);
      }
    }

    // lets implement the insecure version first
    ValuePtr processGen(ValuePtr tableName, ValuePtr key){
      // traverse the graph and return the serialized graph
      // find out all the linked tuples and recursively find their linked tuples and add it to the list

      // assert that the table name is materialized and has key 1
      std::cout<<"processGen called with arg" << tableName->toString() << " and time stamp " << key->toString()<< " \n";
      return tableName;
      
    }

    ValuePtr processExtract(ValuePtr tableName, ValuePtr buf){
      std::cout<<"processGen called with arg" << tableName->toString() << " and buffer " << buf->toString()<< " \n";
      return tableName;
      // traverse the graph and return the serialized graph
    }
    //
    //
    // CHECK IF THE ALL THE POS ARE CORRECTLY ASSIGNED
    ///
    //
    //

    // serialize the graph rooted at parent tuple with table name = tablename
    // store the result in buf
    // also create a secure version of this parent tuple if the createSecureVersion flag is set
    // princem: currently the old tuple can be safely deleted
    void serialize(CommonTable::ManagerPtr catalog, TuplePtr parent, ListPtr buf, bool createSecureVersion){
	// do a lookup on the ref table to find out all the referenced tables
	//     -> for all such tables childTable, iterate over all the joinable tuples (through locSpec table)
	//           -> for all such tuples child, call serialize(catalog, childTableName, child, buf, link.type)
	//           -> if link.type == strong
	//                -> calculate the appended hash
	//           -> create locspec tuple for this child tuple
	//           -> use the appended hash to calculate the link hash
	// finally, calculate the the local hash that includes strong link.hash and all non-loc spec fields
	// return local hash

//       uint32_t refPosPos = catalog->attribute(REF, "LOCSPECFIELD");
//       uint32_t refTypePos = catalog->attribute(REF, "REFTYPE");
//       uint32_t refToPos = catalog->attribute(REF, "TO");
     
//       assert((*parent)[catalog->attribute(FUNCTOR, "TID")] != Val_Null::mk());
//       CommonTablePtr refTbl = catalog->table(REF);
//       CommonTable::Iterator refIter;
//       CommonTable::Key key;
//       key->push_back(refPosVal);
//       CommonTablePtr childTbl = catalog->table(childTableName);
//       CommonTablePtr locSpecTbl = catalog->table(LOCSPECTABLE);
//       CommonTable::Iterator childIter;
//       CommonTable::Iterator locSpecIter;
      
//       for (locSpecIter = locSpecTbl->lookup(CommonTable::theKey(CommonTable::KEY0), CommonTable::theKey(CommonTable::KEY4), parent);
// 	   !refIter->done(); ) {
// 	TuplePtr ref = refIter->next();
// 	uint32_t refPosVal = Val_UInt32::cast((*ref)[refPosPos]);
// 	TuplePtr refLocSpecTuple = Val_Tuple::cast((*parent)[refPosVal]);
// 	uint32_t refType = Val_UInt32::cast((*ref)[refTypePos]);
// 	string childTableName = Val_Str::cast((*ref)[refToPos]);

// 	Fdbuf totalBuf;
// 	childIter = childTbl->lookup(key, CommonTable::theKey(CommonTable::KEY2), parent);
// 	  for (; !childIter->done(); ) {
// 	    TuplePtr child = child->next();
// 	childIter = childTbl->lookup(key, CommonTable::theKey(CommonTable::KEY2), parent);
// 	  for (; !childIter->done(); ) {
// 	    TuplePtr child = child->next();
// 	    if(refType == STRONG){
// 	      ValuePtr proof = serialize(catalog, child, buf, (refType == STRONG));
// 	      fdbuf->append(proof);
// 	      createStrongLocSpec
// 	    }
// 	  }
// 	  if(refType == STRONG){
	    
// 	  }

// 	}
// 	    if(refType == STRONG){
// 	      ValuePtr proof = serialize(catalog, child, buf, (refType == STRONG));
// 	      fdbuf->append(proof);
// 	      createStrongLocSpec
// 	    }
// 	  }
// 	  if(refType == STRONG){
	    
// 	  }

// 	}
//       }
    }


  } // END SECURE

} // END COMPILE
