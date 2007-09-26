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
      CommonTable::ManagerPtr catalog = Plumber::catalog();
      TuplePtr locSpec = Tuple::mk(LOCSPECTUPLE);
      locSpec->append(catalog->nodeid());
      locSpec->append(Val_UInt32::mk(idCounter++));
      locSpec->append(Val_UInt32::mk(strong?1:0)); // strong or weak
      locSpec->append(Val_Null::mk()); //for the self-certifying hash
      locSpec->freeze();
      return Val_Tuple::mk(locSpec);
    }

    ValuePtr generateVersion(bool strong){
      CommonTable::ManagerPtr catalog = Plumber::catalog();
      TuplePtr version = Tuple::mk(VERSIONTUPLE);
      version->append(catalog->nodeid());
      version->append(Val_UInt32::mk(idCounter++));
      version->append(Val_UInt32::mk(strong?1:0)); // strong or weak
      version->append(Val_Null::mk()); //for the self-certifying hash
      version->freeze();
      return Val_Tuple::mk(version);
    }

    bool isLocSpec(ValuePtr v){
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

      // assert that the table name is materialized and has key 0,1
      return tableName;
      
    }

    ValuePtr processExtract(ValuePtr tableName, ValuePtr buf){
      return tableName;
      // traverse the graph and return the serialized graph
    }

    void serialize(string tablename, ValuePtr key, ListPtr buf){
      
    }


  } // END SECURE

} // END COMPILE
