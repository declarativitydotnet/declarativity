/*
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 */

#include "val_null.h"
#include "val_set.h"

class OperSet : public opr::OperCompare<Val_Set> {

  virtual ValuePtr _divide (const ValuePtr& v1, const ValuePtr& v2) const {
    return Val_Set::mk(Val_Set::cast(v1)->difference(Val_Set::cast(v2)));
  };
  virtual ValuePtr _bor (const ValuePtr& v1, const ValuePtr& v2) const {
    return Val_Set::mk(Val_Set::cast(v1)->setunion(Val_Set::cast(v2)));
  };
  virtual ValuePtr _bxor (const ValuePtr& v1, const ValuePtr& v2) const {
    return Val_Set::mk(Val_Set::cast(v1)->symmetricDiff(Val_Set::cast(v2)));
  };
  virtual ValuePtr _band (const ValuePtr& v1, const ValuePtr& v2) const {
    return Val_Set::mk(Val_Set::cast(v1)->intersect(Val_Set::cast(v2)));
  };
};

const opr::Oper* Val_Set::oper_ = new OperSet();

SetPtr Val_Set::cast(ValuePtr v)
{
   
  switch(v->typeCode()) {
  case Value::SET:
    {
      return (static_cast<Val_Set *>(v.get()))->L;         
    }
  case Value::NULLV:
    {
      return Set::mk();
    }
  default:
    {
      throw Value::TypeError(v->typeCode(),
                             v->typeName(),
                             Value::SET,
                             "set");      
    }
  }
}


void Val_Set::xdr_marshal_subtype( XDR *x )
{
  L->xdr_marshal(x);
}

ValuePtr Val_Set::xdr_unmarshal( XDR *x )
{
  SetPtr lp = Set::xdr_unmarshal(x);
  return mk(lp);
}

string Val_Set::toConfString() const
{
  ostringstream sb;
   
  sb << "(";
   
  ValPtrSet::const_iterator iter = L->begin();
  ValPtrSet::const_iterator end = L->end();
  ValPtrSet::const_iterator almost_end = L->end();
  almost_end--;
   
  while(iter != end) {
    sb << (*iter)->toConfString();
      
    if(iter != almost_end) {
      sb << ", ";
    }
    iter++;
  }
   
  sb << ")";
   
  return sb.str();

}


// Set comparison. 
int Val_Set::compareTo(ValuePtr other) const
{
  if(other->typeCode() < Value::SET) {
    return -1;
  } else if(other->typeCode() > Value::SET) {
    return 1;
  } else {
    return L->compareTo(cast(other));
  }
}
