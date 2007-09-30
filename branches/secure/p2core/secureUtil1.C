#include "secureUtil1.h"
#include "secureUtil.h"
#include "systemTable.h"
#include "val_uint32.h"
#include "val_tuple.h"
#include "val_null.h"
#include "val_str.h"
#include "val_list.h"
#include "tuple.h"
#include "plumber.h"

namespace compile {

  namespace secure {
    const string ROOTSUFFIX = "NewProcess";
    const string NEWSAYSSUFFIX = "NewSays";
    const string SAYSSUFFIX = "Says";
    const string LOCSPECTABLE = "locSpecTable";
    const uint32_t WEAK = 0;
    const uint32_t STRONG = 1;
    const uint32_t WEAKSAYS = 2;
    const uint32_t STRONGSAYS = 3;
    const uint32_t ROOT = 4;    
    const uint32_t LOCSPECLOCATIONFIELD = 3;
    const uint32_t CURVERSION = 0;
    static string refSuffix[] = {"", "", NEWSAYSSUFFIX, NEWSAYSSUFFIX, ""};
    static uint32_t idCounter = 0;
    void
    dump(CommonTable::ManagerPtr catalog)
    {
      CommonTablePtr functorTbl = catalog->table("parentNewProcess");
      CommonTablePtr parentTbl = catalog->table("parent");
      CommonTablePtr assignTbl  = catalog->table("child");
      CommonTablePtr locSpecTbl  = catalog->table(LOCSPECTABLE);
      CommonTablePtr tableTbl  = catalog->table(TABLE);

      CommonTable::Iterator iter;
      // first display all functors
      TELL_OUTPUT << "\n PARENTSNewProcess \n";
      for (iter = functorTbl->scan();!iter->done(); ) {
	TuplePtr functor = iter->next();
	TELL_OUTPUT << functor->toString()<<"\n";
      }

      TELL_OUTPUT << "\n PARENTS \n";
      for (iter = parentTbl->scan();!iter->done(); ) {
	TuplePtr functor = iter->next();
	TELL_OUTPUT << functor->toString()<<"\n";
      }

      TELL_OUTPUT << "\n CHILD \n";
      for (iter = assignTbl->scan();!iter->done(); ) {
	TuplePtr term = iter->next();
	TELL_OUTPUT << term->toString()<<"\n";
      }

      TELL_OUTPUT << "\n LOCSPEC \n";
      for (iter = locSpecTbl->scan();!iter->done(); ) {
	TuplePtr term = iter->next();
	TELL_OUTPUT << term->toString()<<"\n";
      }

      TELL_OUTPUT << "\n TABLE \n";
      for (iter = tableTbl->scan();!iter->done(); ) {
	TuplePtr term = iter->next();
	TELL_OUTPUT << term->toString()<<"\n";
      }
  
    }   

    ValuePtr generateLocSpec(bool strong){
      std::cout<<"generate loc spec called \n";
      CommonTable::ManagerPtr catalog = Plumber::catalog();
      TuplePtr locSpec = Tuple::mk();
      locSpec->append(Val_Str::mk(LOCSPECTUPLE));
      locSpec->append(catalog->nodeid());
      //locSpec->append(Val_Str::mk("Hi there!"));
      locSpec->append(Val_UInt32::mk(idCounter++));
      locSpec->append(Val_UInt32::mk(strong?1:0)); // strong or weak
      locSpec->append(Val_Null::mk()); //for the self-certifying hash
      locSpec->freeze();
      return Val_Tuple::mk(locSpec);
    }

    ValuePtr generateVersion(bool strong){
      std::cout<<"generate version called \n";
      CommonTable::ManagerPtr catalog = Plumber::catalog();
      TuplePtr version = Tuple::mk();
      version->append(Val_Str::mk(VERSIONTUPLE));
      version->append(catalog->nodeid());
      //version->append(Val_Str::mk("Hi there!"));
      version->append(Val_UInt32::mk(idCounter++));
      version->append(Val_UInt32::mk(strong?1:0)); // strong or weak
      version->append(Val_Null::mk()); //for the self-certifying hash
      version->freeze();
      return Val_Tuple::mk(version);
    }

    bool isLocSpec(ValuePtr v){
      bool rVal = false;
      if(v->typeCode() == Value::TUPLE){
	TuplePtr tuple = Val_Tuple::cast(v);
	rVal = (Val_Str::cast((*tuple)[TNAME]) == LOCSPECTUPLE);
      }
      std::cout<<"is locspec called with arg " << v->toString() << ": returned " << rVal << "\n";
      return rVal;

    }

    // lets implement the insecure version first
    ValuePtr processGen(ValuePtr tableName, ValuePtr key){
      // traverse the graph and return the serialized graph
      // find out all the linked tuples and recursively find their linked tuples and add it to the list

      // assert that the table name is materialized and has key 2
      std::cout<<"processGen called with arg" << tableName->toString() << " and time stamp " << key->toString()<< " \n";
      CommonTable::ManagerPtr catalog = Plumber::catalog();
      dump(catalog);
       CommonTablePtr parentTbl = catalog->table(Val_Str::cast(tableName) + ROOTSUFFIX);      
      TuplePtr dummyTpl = Tuple::mk(Val_Str::cast(tableName) + ROOTSUFFIX);
      dummyTpl->append(key);
      // get reference to the tuple being talked about
      CommonTable::Iterator parentProcessIter = parentTbl->lookup(parentTbl->primaryKey(), dummyTpl);
      assert(!parentProcessIter->done());
      TuplePtr parentProcessTuple = parentProcessIter->next();
      // process root tuple here and call the serialize method for each linked tuple
      // root needs spl processing i.e. dependent on the other parameters of processNewTuple
      // first field
      ListPtr buf = List::mk();
      serialize(catalog, parentProcessTuple, Val_Str::cast(tableName), buf, ROOT);
      std::cout<<"serialized buffer"<<"\n"<<buf->toString()<<"\n";
      return Val_List::mk(buf);
      
    }

    ValuePtr processExtract(ValuePtr tableName, ValuePtr buf){
      std::cout<<"processExtract called with arg" << tableName->toString() << " and buffer " << buf->toString()<< " \n";
      CommonTable::ManagerPtr catalog = Plumber::catalog();
      ValuePtr nodeId = catalog->nodeid();
      ListPtr serialBuf = Val_List::cast(buf);
      for(ValPtrList::const_iterator iter= serialBuf->begin(); iter != serialBuf->end(); iter++){
	TuplePtr tuple = (Val_Tuple::cast(*iter))->clone();
	tuple->set(NODE_ID, nodeId);
	string tablename = Val_Str::cast((*tuple)[TNAME]);
	if(tablename == LOCSPECTABLE){
	  tuple->set(LOCSPECLOCATIONFIELD, nodeId);
	}
	tuple->freeze();
	CommonTablePtr table = catalog->table(tablename);
	if(!table->insert(tuple)){
	  std::cout<<"Failed to insert "<< tuple->toString();
	}
      }
      dump(catalog);
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
    // handle parentNewProcess tuple, parent tuple, and parentNewSays tuple
    // decision based on the parent tuple: parentname tuple for ref calculations
    void serialize(CommonTable::ManagerPtr catalog, TuplePtr parent, string parentname, ListPtr buf, uint32_t myRefType){
	// do a lookup on the ref table to find out all the referenced tables
	//     -> for all such tables childTable, iterate over all the joinable tuples (through locSpec table)
	//           -> for all such tuples child, call serialize(catalog, childTableName, child, buf, link.type)
	//           -> if link.type == strong
	//                -> calculate the appended hash
	//           -> create locspec tuple for this child tuple
	//           -> use the appended hash to calculate the link hash
	// finally, calculate the the local hash that includes strong link.hash and all non-loc spec fields
	// return local hash

      string tuplename = Val_Str::cast((*parent)[0]);
      std::cout<<"serializing " << tuplename <<" tuple \n";
      uint32_t refPosPos = catalog->attribute(REF, "LOCSPECFIELD");
      uint32_t refTypePos = catalog->attribute(REF, "REFTYPE");
      uint32_t refToPos = catalog->attribute(REF, "TO");
      
      uint32_t offset = 0;
      switch(myRefType){
      case WEAK: 
      case STRONG: offset = 1; // because of version
	assert(parentname == tuplename);
	break;
      case WEAKSAYS:
      case STRONGSAYS: offset = 3; // because of new fields
	assert(tuplename == (parentname + NEWSAYSSUFFIX));
	break;
      case ROOT: offset = 4; // because of new fields + time stamp fields
	assert(tuplename == (parentname + ROOTSUFFIX));
	break;
      default: assert(0);
      }
      
      CommonTablePtr refTbl = catalog->table(REF);
      CommonTable::Iterator refIter;
      TuplePtr dummyTpl = Tuple::mk(parentname);
      for(refIter = refTbl->lookup(CommonTable::theKey(CommonTable::KEY0), CommonTable::theKey(CommonTable::KEY4), dummyTpl); !refIter->done();){
      
	TuplePtr refTpl = refIter->next();
	
	uint32_t refType = Val_UInt32::cast((*refTpl)[refTypePos]);
	string childTableName = Val_Str::cast((*refTpl)[refToPos]);
	uint32_t refPosVal = Val_UInt32::cast((*refTpl)[refPosPos]);
	CommonTable::Key key;
	key.push_back(refPosVal + offset);
	string materializedChild = childTableName + refSuffix[refType];
	CommonTablePtr childTbl = catalog->table(materializedChild);
	CommonTablePtr locSpecTbl = catalog->table(LOCSPECTABLE);
	CommonTable::Iterator childIter;
	CommonTable::Iterator locSpecIter;
	
	for (locSpecIter = locSpecTbl->lookup(key, CommonTable::theKey(CommonTable::KEY2), parent);
	     !locSpecIter->done(); ) {
	  TuplePtr locSpec = locSpecIter->next()->clone();
	  locSpec->freeze();
	  TuplePtr refLocSpecTuple = Val_Tuple::cast((*parent)[refPosVal]);
	  
	  SetPtr hashSet = Set::mk();
	  childIter = childTbl->lookup(CommonTable::theKey(CommonTable::KEY4), CommonTable::theKey(CommonTable::KEY2), locSpec);
	  if(!childIter->done()) {
	    TuplePtr child = childIter->next();
	    if(refType == STRONG || refType == STRONGSAYS){
	      assert(0);
	      //	      ValuePtr proof = serialize(catalog, child, childTableName, buf, refType);
	      //	      hashSet->insert(proof);
	      // createStrongLocSpec
	    }
	    else{
	      serialize(catalog, child, childTableName, buf, refType);
	    }
	  }
	  if(refType == STRONG || refType == STRONGSAYS){
	    // do more work
	  }
	  buf->append(Val_Tuple::mk(locSpec));
	    
	}
      }

      if(myRefType == STRONGSAYS || myRefType == WEAKSAYS || myRefType == ROOT){
	uint32_t tupleSize = parent->size();
	string parentNewName;
	if(myRefType == STRONGSAYS || myRefType == WEAKSAYS){
	  parentNewName = parentname + SAYSSUFFIX;
	}
	else if(myRefType == ROOT){
	  parentNewName = parentname;
	}
	TuplePtr parentCopy = Tuple::mk(parentNewName);
	if(myRefType == ROOT){
	  parentCopy->append(Val_UInt32::mk(CURVERSION));
	}
	for(uint32_t i = offset + 1 + 1; i < tupleSize; i++) // for location as it is also automatically created, also note that relevant fields start from 1
	  {
	    parentCopy->append((*parent)[i]);
	  }
	parentCopy->freeze();
	std::cout<<"created a tuple "<< parentCopy->toString() << "to replace tuple"<< parent->toString()<<"\n";
	buf->append(Val_Tuple::mk(parentCopy));
      }
      else{
	TuplePtr parentCopy = parent->clone();
	parentCopy->freeze();
	buf->append(Val_Tuple::mk(parentCopy));
      }
	
    }
    

} // END SECURE

} // END COMPILE
