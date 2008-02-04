#include<set>
#include <openssl/sha.h>
#include "secureUtil1.h"
#include "secureUtil.h"
#include "val_int64.h"
#include "val_tuple.h"
#include "val_null.h"
#include "val_opaque.h"
#include "val_str.h"
#include "val_list.h"
#include "list.h"
#include "val_set.h"
#include "val_id.h"
#include "tuple.h"
#include "plumber.h"
#include "fdbuf.h"
#include "xdrbuf.h"
#include "systemTable.h"

namespace compile {

  namespace secure {
    const string ROOTSUFFIX = "NewProcess";
    const string NEWSAYSSUFFIX = "NewSays";
    const string SAYSSUFFIX = "Says";
    const string LOCSPECTABLE = "locSpecTable";
    const string LINKEXPANDERTABLE = "linkExpanderTable";
    const uint32_t TUPLEVERPOS = 2;
    const uint32_t LOCSPECLOCATIONPOS = 3; // pos of location field in loc spec tuple
    const uint32_t LOCSPECVERPOS = 4; // pos of version field in loc spec tuple
    const uint32_t LOCSPECLOCSPECPOS = 2; // pos of loc spec field in loc spec tuple
    const uint32_t LINKEXPANDERLOCSPECPOS = 2; // pos of loc spec field in link expander tuple
    const uint32_t CURVERSION = 0;
    static string refSuffix[] = {"", "", NEWSAYSSUFFIX, NEWSAYSSUFFIX, ""}; // suffix for the tuple name based on the ref type
    static uint32_t idCounter = 0;
    static bool debug = true;
    void
    dump(CommonTable::ManagerPtr catalog)
    {
      CommonTablePtr functorTbl = catalog->table("parentNewProcess");
      CommonTablePtr parentTbl = catalog->table("parent");
      CommonTablePtr parentSaysTbl = catalog->table("parentSays");
      CommonTablePtr assignTbl  = catalog->table("child");
      CommonTablePtr locSpecTbl  = catalog->table(LOCSPECTABLE);
      CommonTablePtr linkExpanderTbl  = catalog->table(LINKEXPANDERTABLE);
      CommonTablePtr tableTbl  = catalog->table(TABLE);

      CommonTable::Iterator iter;
      // first display all functors
      TELL_OUTPUT << "\n PARENTSNewProcess \n";
      for (iter = functorTbl->scan();!iter->done(); ) {
	TuplePtr functor = iter->next();
	TELL_OUTPUT << functor->toString()<<"\n";
      }

      TELL_OUTPUT << "\n ParentSaysTbl \n";
      if(parentSaysTbl){
	for (iter = parentSaysTbl->scan();!iter->done(); ) {
	  TuplePtr functor = iter->next();
	  TELL_OUTPUT << functor->toString()<<"\n";
	}
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

      TELL_OUTPUT << "\n LINKEXPANDER \n";
      for (iter = linkExpanderTbl->scan();!iter->done(); ) {
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
      if(debug)std::cout<<"generate loc spec called \n";
      CommonTable::ManagerPtr catalog = Plumber::catalog();
      TuplePtr locSpec = Tuple::mk();
      locSpec->append(Val_Str::mk(LOCSPECTUPLE));
      locSpec->append(catalog->nodeid());
      //locSpec->append(Val_Str::mk("Hi there!"));
      locSpec->append(Val_Int64::mk(idCounter++));
      locSpec->append(Val_Int64::mk(strong?1:0)); // strong or weak
      locSpec->append(Val_Null::mk()); //for the self-certifying hash
      locSpec->freeze();
      return Val_Tuple::mk(locSpec);
    }

    ValuePtr generateVersion(bool strong){
      if(debug)std::cout<<"generate version called \n";
      CommonTable::ManagerPtr catalog = Plumber::catalog();
      TuplePtr version = Tuple::mk();
      version->append(Val_Str::mk(VERSIONTUPLE));
      version->append(catalog->nodeid());
      //version->append(Val_Str::mk("Hi there!"));
      version->append(Val_Int64::mk(idCounter++));
      version->append(Val_Int64::mk(strong?1:0)); // strong or weak
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
      if(debug)std::cout<<"is locspec called with arg " << v->toString() << ": returned " << rVal << "\n";
      return rVal;

    }

    bool isVersion(ValuePtr v){
      bool rVal = false;
      if(v->typeCode() == Value::TUPLE){
	TuplePtr tuple = Val_Tuple::cast(v);
	rVal = (Val_Str::cast((*tuple)[TNAME]) == VERSIONTUPLE);
      }
      return rVal;
    }

    ValuePtr loadFile(ValuePtr fileName){
      return SecurityAlgorithms::readFromFile(Val_Str::cast(fileName));
    }

    // lets implement the insecure version first
    ValuePtr processGen(ValuePtr tableName, ValuePtr key){
      // traverse the graph and return the serialized graph
      // find out all the linked tuples and recursively find their linked tuples and add it to the list

      // assert that the table name is materialized and has key 2
      if(debug)std::cout<<"processGen called with arg" << tableName->toString() << " and time stamp " << key->toString()<< " \n";
      CommonTable::ManagerPtr catalog = Plumber::catalog();
      if(debug) dump(catalog);
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
      ValuePtr hint = (*parentProcessTuple)[HINTPOS + 1]; // +1 because of timestamp
      uint32_t myRefType = (isSaysHint(hint)?ROOTSAYS:ROOT);
      serialize(catalog, parentProcessTuple, Val_Str::cast(tableName), buf, myRefType, true);
      if(debug)std::cout<<"serialized buffer"<<"\n"<<buf->toString()<<"\n";
      return Val_List::mk(buf);
      
    }

    ValuePtr processExtract(ValuePtr tableName, ValuePtr buf){
      if(debug)std::cout<<"processExtract called with arg" << tableName->toString() << " and buffer " << buf->toString()<< " \n";
      CommonTable::ManagerPtr catalog = Plumber::catalog();
      ValuePtr nodeId = catalog->nodeid();
      ListPtr serialBuf = Val_List::cast(buf);
      ValuePtr null = Val_Null::mk();

      CommonTablePtr refTbl = catalog->table(REF);
      CommonTable::Iterator refIter;
      uint32_t refPosPos = catalog->attribute(REF, "LOCSPECFIELD");
      uint32_t refTypePos = catalog->attribute(REF, "REFTYPE");
      
      for(ValPtrList::const_iterator iter= serialBuf->begin(); iter != serialBuf->end(); iter++){
	TuplePtr tuple = (Val_Tuple::cast(*iter))->clone();
	bool install = true; // unless something invalid is found, tuple WILL be installed
	tuple->set(NODE_ID, nodeId);
	string tablename = Val_Str::cast((*tuple)[TNAME]);
	if(tablename == LOCSPECTABLE){
	  tuple->set(LOCSPECLOCATIONPOS, nodeId);
	}
	else if (tablename == LINKEXPANDERTABLE){
	  TuplePtr locSpec = Val_Tuple::cast((*tuple)[LINKEXPANDERLOCSPECPOS]);
	  ValuePtr hash = sha1((*tuple)[LINKEXPANDERLOCSPECPOS + 1]);
	  ValuePtr embeddedHash = (*locSpec)[HASHPOS];
	  if(embeddedHash->compareTo(hash) != 0){
	    install = false;
	    if(debug)std::cout<<"calculated hash"<<hash->toString()<<"\n embedded hash"<<(*locSpec)[HASHPOS]->toString();
	  }
	}
	else { // handle version tuples
	  // check the hash of the normal table if the version is strong
	  uint32_t offset = 1; // for version field
	  //	      create a set and also assert that all the strong fields have non-null hash
	  ValuePtr version = (*tuple)[TUPLEVERPOS]; // the version field
	  TuplePtr verTp;
	  bool strong = false; // by default assume that this tuple is not supposed to be strongly linked
	  std::set<uint32_t> linkSet;
	  TuplePtr dummyTpl= Tuple::mk();
	  string parentname = Val_Str::cast((*tuple)[TNAME]);
	  dummyTpl->append((*tuple)[TNAME]);

	  if(isVersion(version)){ // if its a non-current version field
	    verTp = Val_Tuple::cast(version);
	    strong = (Val_Int64::cast((*verTp)[STRONGLINK]) == 1);
	  }

	  // can handle other cases here
 	  size_t saysPos = parentname.rfind(SAYSSUFFIX, parentname.size());
	  string basename = parentname;
 	  if(saysPos != string::npos){
	    offset += 5; // for P, R, K, V and Proof
	    basename = parentname.substr(0, saysPos);
	    dummyTpl->set(TNAME, Val_Str::mk(basename));
 	  }
	  
	  // need to use a modified tuple if the tuple is suffixed (i.e SAYSSUFFIX)

	  for(refIter = refTbl->lookup(CommonTable::theKey(CommonTable::KEY0), 
				       CommonTable::theKey(CommonTable::KEY4), 
				       dummyTpl); 
	      !refIter->done() && install;){
	    
	    TuplePtr refTpl = refIter->next();		
	    uint32_t refType = Val_Int64::cast((*refTpl)[refTypePos]);
	    uint32_t refPos = Val_Int64::cast((*refTpl)[refPosPos]);
	    linkSet.insert(refPos);
	    
	    if(!isLocSpec((*tuple)[refPos + offset])){
	      if(debug)std::cout<<"Invalid tuple received: location specifier field "<<refPos<<" corrupted "<<(*tuple)[refPos + offset]->toString();
	      install = false; 
	      continue;
	    }
	    else if(strong){
	      // check if the hash of this tuple matches its version[has]
	      // find out the list of strong and weak references for this tuple
	      if(refType == STRONGLINK || refType == STRONGSAYS){
		TuplePtr locSpecTuple = Val_Tuple::cast((*tuple)[refPos + offset]);
		if((*locSpecTuple)[HASHPOS] == null){
		  if(debug)std::cout<<"Invalid tuple received: strong location specifier field "<<(refPos + offset)<<" is missing hash "<<(*tuple)[refPos+offset]->toString();
		  install = false;
		  continue;
		}
	      }
	    }
	  }

	  if(strong && install){
	      // need to check that the hash matches the tuple data
	    // add the hash the cert
	    // set the retVal
	    std::set<uint32_t> skipSet;	
	    skipSet.insert(1); // for loc
	    skipSet.insert(2); // for version
	    ValuePtr serializedTuple = serializeTuple1(tuple, parentname, linkSet, skipSet, offset);
	    ValuePtr retVal = sha1(serializedTuple);
	    if(retVal->compareTo((*verTp)[HASHPOS]) != 0){
	      install = false;
	      continue;
	    }
	    
	  }

	  if((saysPos != string::npos) && install){
	    // check the validity of says params
	    std::set<uint32_t> skipSet;	
	    skipSet.insert(1); // for loc
	    skipSet.insert(2); // for version
	    skipSet.insert(3); // for P
	    skipSet.insert(4); // for R
	    skipSet.insert(5); // for K
	    skipSet.insert(6); // for V
	    skipSet.insert(7); // for Proof
	    //	    uint32_t linkSetOffset = 6;// 1 for P, 1 for R, 1 for K, 1 for V, 1 for Proof, 1 for version
	    ValuePtr serializedTuple = serializeTuple1(tuple, basename, linkSet, skipSet, offset);

	    ValuePtr proof_2 = (*tuple)[PROOFPOS + 1];
	    ValuePtr P_3 = (*tuple)[PPOS + 1];
	    ValuePtr R_4 =  (*tuple)[RPOS + 1];
	    uint32_t K_5 = Val_Int64::cast((*tuple)[KPOS + 1]);
	    ValuePtr V_6 =  (*tuple)[VPOS + 1];
	    ListPtr proofList;
	    SetPtr P, R, V;
	    if(proof_2->typeCode() != Value::LIST){
	      proofList = List::mk();
	      proofList->append(proof_2);
	    }
	    else{
	      proofList = Val_List::cast(proof_2);
	    }
	    
	    if(P_3->typeCode() != Value::SET){
	      P = Set::mk();
	      P->insert(P_3);
	    }
	    else{
	      P = Val_Set::cast(P_3);
	    }
	    if(R_4->typeCode() != Value::SET){
	      R = Set::mk();
	      R->insert(R_4);
	    }
	    else{
	      R = Val_Set::cast(R_4);
	    }
	    
	    if(V_6->typeCode() != Value::SET){
	      V = Set::mk();
	      V->insert(V_6);
	    }
	    else{
	      V = Val_Set::cast(V_6);
	    }
	    
	    compile::secure::Primitive primitive(P, R, K_5, V);
	    
	    install = compile::secure::SecurityAlgorithms::verify(serializedTuple, proofList, &primitive);
	  }
	  
	}
	tuple->freeze();
	CommonTablePtr table = catalog->table(tablename);
	if(install){
	  if(!table->insert(tuple))
	    if(debug)std::cout<<"Failed to insert "<< tuple->toString();
	}
	else{
	  if(debug)std::cout<<"Failed to validate tuple"<<tuple->toString();
	}
      }
      if(debug) dump(catalog);
      return tableName;
      // traverse the graph and return the serialized graph
    }

    // serializes a tuple excluding the loc spec field
//     ValuePtr serializeTuple(TuplePtr tuple, std::set<uint32_t> linkSet){
//       Fdbuf* fin = new Fdbuf(0);
//       XDR xdr;
//       xdrfdbuf_create(&xdr, fin, false, XDR_ENCODE);
//       ValuePtr va;
//       uint32_t size = tuple->size();
//       for(uint32_t count = 0; count  < size; count++){
// 	if(linkSet.find(count) == linkSet.end()){ // process link fields differently
// 	  ValuePtr field = (*tuple)[count];
// 	  if(field->typeCode() == Value::TUPLE){
// 	    TuplePtr fieldTuple  = Val_Tuple::cast(field);
// 	    string tablename = Val_Str::cast((*fieldTuple)[TNAME]);
// 	    assert(tablename == LOCSPECTUPLE);
// 	    (*fieldTuple)[HASHPOS]->xdr_marshal(&xdr);
// 	  }
// 	}
// 	else if(count != 1 && count != 2){ // skip location field and the version field
// 	  (*tuple)[count]->xdr_marshal(&xdr);
// 	}
//       }
//       return Val_Opaque::mk(FdbufPtr(fin));
//     }

    // serializes a tuple excluding the loc spec field
    ValuePtr serializeTuple1(TuplePtr tuple, string name, std::set<uint32_t> linkSet, std::set<uint32_t> skipFields, uint32_t linkSetOffset){
      ostringstream o;
      /*      std::cout<<"\n Printing linkSet"<<" ---- ";
      for(std::set<uint32_t>::iterator i = linkSet.begin(); i != linkSet.end(); i++){
	std::cout<<(*i)<<" ";
      }
      std::cout<<"\n done Printing linkSet"<<" ---- ";*/
      uint32_t size = tuple->size();
      o<<name;
      for(uint32_t count = 1; count  < size; count++){
	if(linkSet.find(count - linkSetOffset) != linkSet.end()){ // process link fields differently
	  ValuePtr field = (*tuple)[count];
	  if(field->typeCode() == Value::TUPLE){
	    TuplePtr fieldTuple  = Val_Tuple::cast(field);
	    string tablename = Val_Str::cast((*fieldTuple)[TNAME]);
	    assert(tablename == LOCSPECTUPLE);
	    o<<(*fieldTuple)[HASHPOS]->toString();
	  }
	}
	else if(skipFields.find(count) == skipFields.end()){ // skip location field and the version field
	  o<<(*tuple)[count]->toString();
	}
      }
      return Val_Str::mk(o.str());
    }

    ValuePtr sha1(ValuePtr vp){
      // create hash of the serialized hashSet and insert it into the cert part of the locspec
      // assert that the cert part of locspec is null prior to this
      std::string svalue = vp->toString();
      unsigned char digest[SHA_DIGEST_LENGTH];	// 20 byte array
      SHA1(reinterpret_cast<const unsigned char*>(svalue.c_str()), 
	   svalue.size(), &digest[0]);
	
      IDPtr hashID = ID::mk(reinterpret_cast<uint32_t*>(digest));
      return Val_ID::mk(hashID);
    }

    ValuePtr getCert(ValuePtr v){
      if(v->typeCode() == Value::TUPLE){
	TuplePtr tuple = Val_Tuple::cast(v);
	string tname = Val_Str::cast((*tuple)[TNAME]);
	if(tname == LOCSPECTUPLE || tname == VERSIONTUPLE){
	  return (*tuple)[HASHPOS];
	}
      }
      return Val_Null::mk();
    }

    ValuePtr makeLocSpecStrong(SetPtr locSpecSet, SetPtr hashSet){
      ValuePtr merkleHash = sha1(Val_Set::mk(hashSet));
      
      for(ValPtrSet::const_iterator iter = locSpecSet->begin(); iter != locSpecSet->end(); iter++){
	TuplePtr locSpecTuple = Val_Tuple::cast(*iter);
	TuplePtr locSpecField = (Val_Tuple::cast((*locSpecTuple)[2]))->clone();
	locSpecField->set(HASHPOS, merkleHash);
	locSpecField->set(STRONGPOS, Val_Int64::mk(1));
	locSpecField->freeze();
	locSpecTuple->set(LOCSPECLOCSPECPOS, Val_Tuple::mk(locSpecField));
	locSpecTuple->freeze();
      }
      return merkleHash;
    }

    bool isSaysHint(ValuePtr v){
      if(v->typeCode() != Value::LIST){
	return false;
      }
      else {
	ListPtr l = Val_List::cast(v);
	ValuePtr front = l->front();
	int typeCode = front->typeCode();
	if(typeCode != Value::INT64){
	  return false;
	}
	else{
	  uint32_t typeCode = Val_Int64::cast(front);
	  return (typeCode == CREATESAYS || typeCode == COPYSAYS);
	}
      }
    }
    
    // serialize the graph rooted at parent tuple with table name = tablename
    // store the result in buf
    // also create a secure version of this parent tuple if the createSecureVersion flag is set
    // princem: currently the old tuple can be safely deleted
    // handle parentNewProcess tuple, parent tuple, and parentNewSays tuple
    // decision based on the parent tuple: parentname tuple for ref calculations
    // myRefType: type of link connecting this tuple to the main root
    // certify: is the link already certified: this will happen for example when a strong link is copied
    ValuePtr serialize(CommonTable::ManagerPtr catalog, TuplePtr parent, string parentname, ListPtr buf, uint32_t myRefType, bool certify){
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
      if(debug)std::cout<<"serializing " << tuplename <<" tuple \n";
      uint32_t refPosPos = catalog->attribute(REF, "LOCSPECFIELD");
      uint32_t refTypePos = catalog->attribute(REF, "REFTYPE");
      uint32_t refToPos = catalog->attribute(REF, "TO");
      std::set<uint32_t> linkSet;
      bool makeSays = true;
      //set offset based on my ref type: this will help in extracting fields from parent tuple      
      uint32_t offset = 0;
      switch(myRefType){
      case WEAKLINK: 
      case STRONGLINK: offset = 1; // because of version
	assert(parentname == tuplename);
	makeSays = false;
	break;
      case WEAKSAYS:
      case STRONGSAYS: offset = 1; // because of version
	if(tuplename == parentname + NEWSAYSSUFFIX){
	  offset += 3; // for loc, opaque and hint
	  makeSays = true;
	}
	else{
	  makeSays = false;
	  offset += 6; // for loc, p, r, k, v and proof
	  assert(tuplename== parentname + SAYSSUFFIX);
	}
	break;
      case ROOT:
      case ROOTSAYS: offset = 4; // because of new fields + time stamp fields
	assert(tuplename == (parentname + ROOTSUFFIX));
	makeSays = ((myRefType ==  ROOTSAYS)?true:false);
	break;
      default: assert(0);
      }
      
      CommonTablePtr refTbl = catalog->table(REF);
      CommonTable::Iterator refIter;
      TuplePtr dummyTpl = Tuple::mk(parentname);
      // now iterate over all links
      for(refIter = refTbl->lookup(CommonTable::theKey(CommonTable::KEY0), CommonTable::theKey(CommonTable::KEY4), dummyTpl); !refIter->done();){
      
	TuplePtr refTpl = refIter->next();
	
	uint32_t refType = Val_Int64::cast((*refTpl)[refTypePos]);
	string childTableName = Val_Str::cast((*refTpl)[refToPos]);
	uint32_t refPosVal = Val_Int64::cast((*refTpl)[refPosPos]);
	CommonTable::Key key;
	key.push_back(refPosVal + offset);
	linkSet.insert(refPosVal);
	CommonTablePtr locSpecTbl = catalog->table(LOCSPECTABLE);
	CommonTable::Iterator childIter;
	CommonTable::Iterator locSpecIter;
	
	SetPtr locSpecSet(new Set());
	SetPtr hashSet(new Set());
	TuplePtr parentLocSpec = Val_Tuple::cast((*parent)[refPosVal + offset]);
	bool needCertification = ((*parentLocSpec)[HASHPOS] == Val_Null::mk()) && certify;
	CommonTablePtr childTbl[2];//max of two types of links
	string materializedChild = childTableName + refSuffix[refType]; // the name of the tuple materilized corresponding to the childTableName
	childTbl[0] = catalog->table(materializedChild);
	uint32_t maxCount = 1;
	if(refType == STRONGSAYS || refType == WEAKSAYS){
	  childTbl[maxCount++] = catalog->table(childTableName + SAYSSUFFIX);
	}

	// for each link, find all locSpecs
	for (locSpecIter = locSpecTbl->lookup(key, CommonTable::theKey(CommonTable::KEY2), parent);
	     !locSpecIter->done(); ) {
	  TuplePtr locSpec = locSpecIter->next()->clone();
	  //	  locSpecTbl->remove(locSpec);
	  //find out the child corresponding to each locspec
	  for(uint32_t typeCount = 0; typeCount < maxCount; typeCount++){
	  
	    childIter = childTbl[typeCount]->lookup(CommonTable::theKey(CommonTable::KEY4), CommonTable::theKey(CommonTable::KEY2), locSpec);
	    if(!childIter->done()) {
	      TuplePtr child = childIter->next();
	      //    childTbl->remove(child);
	      if(refType == STRONGLINK || refType == STRONGSAYS){
		if(certify){
		  ValuePtr proof = serialize(catalog, child, childTableName, buf, refType, needCertification);
		  hashSet->insert(proof); // the version tuple is already having this proof: 
		  // However, we need to include this proof in the version field in the locSpec tuple
		  TuplePtr version = (Val_Tuple::cast((*locSpec)[4]))->clone();
		  version->set(HASHPOS, proof);
		  version->set(STRONGPOS, Val_Int64::mk(1));
		  version->freeze();
		  locSpec->set(4, Val_Tuple::mk(version));
		}
		locSpecSet->insert(Val_Tuple::mk(locSpec));
		// create a link expander and export it 
	      }
	      else{
		serialize(catalog, child, childTableName, buf, refType, certify);
		locSpec->freeze();
		buf->append(Val_Tuple::mk(locSpec));
	      }
	    }
	    
	  }
	  if((refType == STRONGLINK || refType == STRONGSAYS)){
	    TuplePtr linkExpander;
	    if(certify){
	      TuplePtr parentLocSpecTuple = parentLocSpec->clone();
	    
	      ValuePtr merkleHash = makeLocSpecStrong(locSpecSet, hashSet);
	      parentLocSpecTuple->set(HASHPOS, merkleHash);
	      parentLocSpecTuple->set(STRONGPOS, Val_Int64::mk(1));
	      parentLocSpecTuple->freeze();
	      parent->set(refPosVal + offset, Val_Tuple::mk(parentLocSpecTuple));
	   
	      TuplePtr parentLocSpecTupleClone = parentLocSpecTuple->clone();
	      parentLocSpecTupleClone->freeze();
	      linkExpander = Tuple::mk(LINKEXPANDERTABLE);
	      linkExpander->append(Val_Tuple::mk(parentLocSpecTupleClone));
	      linkExpander->append(Val_Set::mk(hashSet));
	      linkExpander->freeze();
	    }
	    else{
	      CommonTablePtr linkExpanderTbl = catalog->table(LINKEXPANDERTABLE);
	      CommonTable::Iterator iter = linkExpanderTbl->lookup(key, CommonTable::theKey(CommonTable::KEY2), parent);
	      assert(!iter->done());
	      linkExpander = iter->next()->clone();
	      assert(iter->done());
	    }

	    buf->append(Val_Tuple::mk(linkExpander));

	    for(ValPtrSet::const_iterator iter = locSpecSet->begin(); iter != locSpecSet->end(); iter++){
	      buf->append(*iter);
	    }
  	  
	  }
	
	}	
      }

      TuplePtr parentCopy;
      if(makeSays || myRefType == ROOT){
	// all tuples here are versioned
	uint32_t tupleSize = parent->size();
	string parentNewName;
	if(myRefType == ROOTSAYS || myRefType == STRONGSAYS || myRefType == WEAKSAYS){
	  parentNewName = parentname + SAYSSUFFIX;
	}
	else if(myRefType == ROOT){
	  parentNewName = parentname;
	}
	parentCopy = Tuple::mk(parentNewName);
	if(myRefType == ROOT || myRefType == ROOTSAYS){
	  parentCopy->append(Val_Int64::mk(CURVERSION));
	  if(myRefType == ROOT){
	    // +2: +1 for location as it is also automatically created, and +1 because the relevant fields start from 1 not 0
	    for(uint32_t i = offset + 2; i < tupleSize; i++) 
	      {
		parentCopy->append((*parent)[i]);
	      }
	  }
	}
	else{
	  parentCopy->append((*parent)[TUPLEVERPOS]);
	}

	if(makeSays){
	  assert(myRefType == WEAKSAYS || myRefType == STRONGSAYS || myRefType == ROOTSAYS);
	  // extract the appropriate parameters from the hintField
	  ValuePtr hintField = (*parent)[HINTPOS+1];
	  assert(hintField != Val_Null::mk());
	  ListPtr hintList = Val_List::cast(hintField);
	  // the structure of hintList: [Type<COPYSAYS/CREATESAYS>, EncType<RSA/AES>(only relevant if prev field is CREATE), Key/Proof, P, R, K, V]
	  uint32_t saysType = Val_Int64::cast(hintList->front());
	  hintList->pop_front();
	  uint32_t encType = ((hintList->front() != Val_Null::mk())?Val_Int64::cast(hintList->front()):0);
	  hintList->pop_front();
	  ValuePtr keyProof = hintList->front();
	  hintList->pop_front();
	  ValuePtr P = hintList->front();
	  hintList->pop_front();
	  ValuePtr R = hintList->front();
	  hintList->pop_front();
	  ValuePtr K = hintList->front();
	  hintList->pop_front();
	  ValuePtr V = hintList->front();
	  hintList->pop_front();
	  parentCopy->append(P);
	  parentCopy->append(R);
	  parentCopy->append(K);
	  parentCopy->append(V);

	  uint32_t linkSetOffset = 4;// 1 for loc, 1 for version/timestamp, 1 for opaque, 1 for hint
	  if(saysType == CREATESAYS){
	    std::set<uint32_t> skipSet;	
	    skipSet.insert(1); // for loc
	    skipSet.insert(2); // for timestamp/version
	    skipSet.insert(3); // for opaque
	    skipSet.insert(4); // for hint
	    skipSet.insert(5); // for destlocspec
	    ValuePtr serializedTuple = serializeTuple1(parent, parentname, linkSet, skipSet, linkSetOffset);
	    ValuePtr proof = compile::secure::SecurityAlgorithms::generate(serializedTuple, encType, keyProof);
	    parentCopy->append(proof);
	  }
	  else{
	    parentCopy->append(keyProof);
	  }
	  // 1 because loc is already included and 1 because fields start from 1
	  for(uint32_t i = linkSetOffset + 2; i < tupleSize; i++) 
	    {
	      parentCopy->append((*parent)[i]);
	    }
	  
	}
      }
      else{ // the default case when its a simple weak or strong link or nothing needs to be done for says links
	parentCopy = parent->clone();
      }
      
      ValuePtr retVal;
      if(certify && (myRefType == STRONGSAYS || myRefType == STRONGLINK)){
	// add the hash the cert
	// set the retVal
	std::set<uint32_t> skipSet;	
	skipSet.insert(1); // for loc
	skipSet.insert(2); // for version
	uint32_t linkSetOffset = 1;// for version
	if(myRefType == STRONGSAYS){
	  linkSetOffset += 5; // P, R, K, V, Proof
	}
	ValuePtr serializedTuple = serializeTuple1(parentCopy, parentname, linkSet, skipSet, linkSetOffset);
	retVal = sha1(serializedTuple);
	TuplePtr version = (Val_Tuple::cast((*parentCopy)[2]))->clone();
	version->set(HASHPOS, retVal);
	version->set(STRONGPOS, Val_Int64::mk(1));
	version->freeze();
	parentCopy->set(TUPLEVERPOS, Val_Tuple::mk(version));
      }
      else{
	retVal = Val_Null::mk();
      }
      parentCopy->freeze();
      buf->append(Val_Tuple::mk(parentCopy));
      return retVal;
    }
    

} // END SECURE

} // END COMPILE
