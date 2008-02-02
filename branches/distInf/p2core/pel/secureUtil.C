#include<iostream>
#include "secureUtil.h"
#include "val_opaque.h"
#include "val_set.h"
#include "val_str.h"
#include "val_int64.h"
#include "value.h"
//#include "sfslite.h"
#include "plumber.h"
#include "systemTable.h"

namespace compile {

  namespace secure {
    bool debug = false;
    const int SecurityAlgorithms::hashSize = 20;
    //modify these to account for actual tuple
    const int SecurityAlgorithms::keyPos = 3;
    const int SecurityAlgorithms::primitivePos = 2;

    FdbufPtr readFromFile(std::string filename){

      Fdbuf *f = new Fdbuf();
      FdbufPtr fdbufPtr(f);
      fdbufPtr->pushString("This is supposed to be an RSA Key");
      return fdbufPtr;
    }
    
    void writeToFile(std::string filename, FdbufPtr f){
      assert(0);
    }


    Primitive* Primitive::combine(const Primitive* p2, int axis) const{
      SetPtr _p, _r, _v;
      uint32_t _k;
      
      SetPtr spkrUnion = p->setunion(p2->p);
      SetPtr spkrIntersection = p->intersect(p2->p);

      switch(axis)
	{
	case Primitive::SPKR:
	  _p = spkrUnion;
	  _r = r->intersect(p2->r);
	  _v = v->intersect(p2->v);
	  _k = std::max((unsigned)std::max(k, p2->k), k + p2->k - spkrIntersection->size());
	  break;
	case Primitive::RCVR:
	  _p = spkrIntersection;
	  _r = r->setunion(p2->r);
	  _v = v->intersect(p2->v);
	  _k = std::max((unsigned)0, k + p2->k - spkrUnion->size());
	  break;
	case Primitive::VERIFIER:
	  _p = spkrIntersection;
	  _r = r->intersect(p2->r);
	  _v = v->setunion(p2->v);
	  _k = std::max((unsigned)0, (signed)k + (signed)p2->k - spkrUnion->size());
	  break;
	default:           
	  throw compile::Exception("Invalid combination axis in Primitive::combine!");

	}
      return new Primitive(_p, _r, _k, _v);
    }

    bool Primitive::smaller(const Primitive *p2) const{
      return (p->subset(p2->p) && r->subset(p2->r) && 
	      v->subset(p2->v) && (p2->k <= k) && p->subset(p2->p) && 
	      (((signed)p->size()-(signed)p2->p->size()) <= ((signed)k - (signed)(p2->k))));
    }

    int Primitive::compareTo(const Primitive *p2) const{
      int comparison= 0;
      if((comparison = p->compareTo(p2->p)) != 0){
	return comparison;
      }
      else if((comparison = r->compareTo(p2->r)) != 0){
	return comparison;
      }
      else if((comparison = ((signed)k - (signed)(p2->k))) != 0){
	return comparison;
      }
      else if((comparison = v->compareTo(p2->v)) != 0){
	return comparison;
      }
      else{
	return 0;
      }
	  
    }
    
    ValuePtr SecurityAlgorithms::generate(ValuePtr msg, uint32_t encType, ValuePtr key){
      switch(encType){
      case SecurityAlgorithms::RSA:
	return signRSA(msg, key);
	break;
      case SecurityAlgorithms::AES:
	return signAES(msg, key);
	break;
      default:
	throw compile::Exception("Invalid encryption type in SecurityAlgorithms::generate!");
	break;
	  
      };
    }

    bool SecurityAlgorithms::verify(ValuePtr msg, ListPtr proof, Primitive *p){
      PrimitiveSet pSet;
      CommonTable::ManagerPtr catalog = Plumber::catalog();

      CommonTablePtr encHintTbl = catalog->table("encHint");  

      CommonTablePtr verKeyTbl = catalog->table("verKey");
      CommonTable::Iterator encHintIter;

      for (encHintIter = encHintTbl->scan(); !encHintIter->done(); ) {
	TuplePtr encHint = encHintIter->next();
	CommonTable::Iterator verKeyIter;
	verKeyIter = verKeyTbl->lookup(CommonTable::theKey(CommonTable::KEY6), 
				       CommonTable::theKey(CommonTable::KEY2), 
				       encHint);
	for (; !verKeyIter->done(); ) {
	  ValPtrList::const_iterator iter = proof->begin();
	  for(; iter != proof->end(); iter++){
	    bool res = false;
	    TuplePtr keyTuple = verKeyIter->next();
	    int encType = Val_Int64::cast((*keyTuple)[SecurityAlgorithms::keyPos]);
	    ValuePtr key = (*keyTuple)[SecurityAlgorithms::keyPos+1];
	    switch(encType){
	    case SecurityAlgorithms::RSA:
	      res = verifyRSA(msg, key, (*iter));
	      break;
	    case SecurityAlgorithms::AES:
	      res = verifyRSA(msg, key, (*iter));
	      break;
	    }
	    if(res){
	      SetPtr P, R, V;
	      ValuePtr P_3 = (*encHint)[SecurityAlgorithms::primitivePos];
	      if(P_3->typeCode() != Value::SET){
		P = Set::mk();
		P->insert(P_3);
	      }
	      else{
		P = Val_Set::cast(P_3);
	      }
	      ValuePtr R_4 = (*encHint)[SecurityAlgorithms::primitivePos+1]; 
	      uint32_t k = Val_Int64::cast((*encHint)[SecurityAlgorithms::primitivePos+2]);
	      ValuePtr V_6 = (*encHint)[SecurityAlgorithms::primitivePos+3];

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
	      
	      Primitive *provedP = new Primitive(P, R, k, V);
	      if(provedP->smaller(p)){
		return true;
	      }
	      pSet.insert(provedP);
	    }
	  }

	}
	  
      }

	
      bool newfacts = true;
      while(newfacts){
	newfacts = false;
	PrimitiveSet::iterator itOuter = pSet.begin();
	for(; itOuter != pSet.end(); itOuter++){
	  PrimitiveSet::iterator itInner = pSet.begin();
	  for(; itInner != pSet.end(); itInner++){
	    if(*itOuter != *itInner){
	      for(int axis = 0; axis < 3; axis++){
		Primitive *combinedP = (*itOuter)->combine(*itInner, axis);
		if(combinedP->smaller(p)){
		  return true;
		}
		if(!stronger(combinedP, pSet)){
		  delete combinedP;
		}
		else{
		  newfacts = true;
		  pSet.insert(combinedP);
		}
	      }
	    }
	  }
	}
      }
      return false;
    }
    
    bool SecurityAlgorithms::stronger(Primitive *p, PrimitiveSet &pSet){
      PrimitiveSet::iterator itOuter = pSet.begin();
      for(; itOuter != pSet.end(); itOuter++){
	if(p->smaller(*itOuter)){
	  delete *itOuter;
	  pSet.erase(itOuter);
	}
	else if((*itOuter)->smaller(p)){
	  return false;
	}
      }
      return true;
    }
    
    ValuePtr SecurityAlgorithms::signAES(ValuePtr msgPtr, ValuePtr keyPtr){

      return signRSA(msgPtr, keyPtr);
      //check if the msg and key are of Val_Opaque type
      //      FdbufPtr msg = Val_Opaque::cast(msgPtr);
      //      FdbufPtr key = Val_Opaque::cast(keyPtr);
      //      return Val_Opaque::mk(Sfslite::secureSignAES(msg, key));
    }
      
    ValuePtr SecurityAlgorithms::signRSA(ValuePtr msgPtr, ValuePtr keyPtr){
      FdbufPtr msg = Val_Opaque::cast(msgPtr);
      FdbufPtr key = Val_Opaque::cast(keyPtr);
      std::string signedMsg = msg->str() + key->str();
      if(debug)std::cout<<"signRSA called: with inputs Msg:"<< msgPtr->toString() << " Key: " << keyPtr->toString() << "returning: "<< signedMsg <<"\n";

      return Val_Str::mk(signedMsg);
      //      return Val_Opaque::mk(Sfslite::secureSignRSA(msg, key));
    }
    
    bool SecurityAlgorithms::verifyAES(ValuePtr msgPtr, ValuePtr keyPtr, ValuePtr proofPtr){	
      //      FdbufPtr msg = Val_Opaque::cast(msgPtr);
      //      FdbufPtr key = Val_Opaque::cast(keyPtr);
      //      FdbufPtr proof = Val_Opaque::cast(proofPtr);
      //      return Sfslite::secureVerifyAES(msg, key, proof);
      return verifyRSA(msgPtr, keyPtr, proofPtr);
    }
      
    bool SecurityAlgorithms::verifyRSA(ValuePtr msgPtr, ValuePtr keyPtr, ValuePtr proofPtr){
      FdbufPtr msg = Val_Opaque::cast(msgPtr);
      //      FdbufPtr proof(new Fdbuf(Val_Opaque::cast(proofPtr)));
      FdbufPtr key = Val_Opaque::cast(keyPtr);

      std::string signedMsg = msg->str() + key->str();
      if(debug)std::cout<<"verifyRSA called: with inputs Msg:"<< msgPtr->toString() << " Proof: " << proofPtr->toString() << "returning: "<< signedMsg <<"\n";
      return (Val_Str::mk(signedMsg)->compareTo(proofPtr) == 0);
      //      return Sfslite::secureVerifyRSA(msg, key, proof);
    }

    ValuePtr SecurityAlgorithms::readFromFile(std::string filename){
      //    return Val_Opaque::mk(Sfslite::readFromFile(filename));
      return Val_Opaque::mk(compile::secure::readFromFile(filename));
    }
    
    void SecurityAlgorithms::writeToFile(std::string filename, ValuePtr f){
      //      Sfslite::writeToFile(filename, Val_Opaque::cast(f));
      compile::secure::writeToFile(filename, Val_Opaque::cast(f));
    }



  } // END NAMESTRACKER

} // END COMPILE
