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
#include "val_list.h"
#include "val_vector.h"

class OperList : public opr::OperCompare<Val_List> {
};

const opr::Oper* Val_List::oper_ = new OperList();

ListPtr Val_List::cast(ValuePtr v)
{
   
  switch(v->typeCode()) {
  case Value::LIST:
    {
      return (static_cast<Val_List *>(v.get()))->L;         
    }
  case Value::VECTOR:
    {
      VectorPtr vp = Val_Vector::cast(v);
      ValPtrList list(vp->begin(), vp->end());
      return ListPtr(new List(list));
    }
  case Value::NULLV:
    {
      return List::mk();
    }
  default:
    {
      throw Value::TypeError(v->typeCode(),
                             v->typeName(),
                             Value::LIST,
                             "list");      
    }
  }
}


void Val_List::xdr_marshal_subtype( XDR *x )
{
  L->xdr_marshal(x);
}

ValuePtr Val_List::xdr_unmarshal( XDR *x )
{
  ListPtr lp = List::xdr_unmarshal(x);
  return mk(lp);
}

string Val_List::toConfString() const
{
  ostringstream sb;
   
  sb << "(";
   
  ValPtrList::const_iterator iter = L->begin();
  ValPtrList::const_iterator end = L->end();
  ValPtrList::const_iterator almost_end = L->end();
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


// List comparison. For now, all members of the Val_List class are 
// equal, since this needs to be declared but doesn't really apply to 
// this particular type --ACR
int Val_List::compareTo(ValuePtr other) const
{
  if(other->typeCode() < Value::LIST) {
    return -1;
  } else if(other->typeCode() > Value::LIST) {
    return 1;
  } else {
    return L->compareTo(cast(other));
  }
}
