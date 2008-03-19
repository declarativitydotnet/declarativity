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
#include "val_vector.h"
#include "val_int64.h"

class OperVector : public opr::OperCompare<Val_Vector> {
};

const opr::Oper* Val_Vector::oper_ = new OperVector();

VectorPtr Val_Vector::cast(ValuePtr v)
{
   
  switch(v->typeCode()) {
  case Value::VECTOR:
    {
      return (static_cast<Val_Vector *>(v.get()))->V;         
    default:
      {
        throw Value::TypeError(v->typeCode(),
                               v->typeName(),
                               Value::VECTOR,
                               "vector");      
      }
    }
  }
}


// Marshal/Unmarshal essentially copied from Val_Tuple
void Val_Vector::xdr_marshal_subtype( XDR *x )
{
  ValPtrVector::const_iterator it;
  
  uint32_t sz = V->size();
  u_int32_t i = (u_int32_t)sz;
  xdr_uint32_t(x, &i);
  // Marshal the entries
  for (it = V->begin();  it != V->end(); ++it)
    { (*it)->xdr_marshal(x); }
}

ValuePtr Val_Vector::xdr_unmarshal( XDR *x )
{
  u_int32_t ui;
  xdr_uint32_t(x, &ui);
  uint32_t sz = ui;

  VectorPtr vp(new boost::numeric::ublas::vector<ValuePtr>(sz));
  // Unmarshal the entries
  for (uint32_t i = 0;
       i < sz;
       i++){
    (*vp)(i) = Value::xdr_unmarshal(x);
  }
  return mk(vp);
}

string Val_Vector::toString() const
{
  ostringstream sb;
   
  sb << "[";
   
  ValPtrVector::const_iterator iter = V->begin();
  ValPtrVector::const_iterator end = V->end();
  ValPtrVector::const_iterator almost_end = V->end();
  almost_end--;
   
  while(iter != end) {
    sb << (*iter)->toString();
      
    if(iter != almost_end) {
      sb << ", ";
    }
    iter++;
  }
   
  sb << "]";
   
  return sb.str();

}

string Val_Vector::toConfString() const
{
  ostringstream sb;
   
  sb << "<";
   
  ValPtrVector::const_iterator iter = V->begin();
  ValPtrVector::const_iterator end = V->end();
  ValPtrVector::const_iterator almost_end = V->end();
  almost_end--;
   
  while(iter != end) {
    sb << (*iter)->toConfString();
      
    if(iter != almost_end) {
      sb << ", ";
    }
    iter++;
  }
   
  sb << ">";
   
  return sb.str();

}


// Vector comparison. Follow rules from Tuple
int Val_Vector::compareTo(ValuePtr other) const
{
  if(other->typeCode() < Value::VECTOR) {
    return -1;
  } 
  else if(other->typeCode() > Value::VECTOR) {
    return 1;
  } 
  else {
    Val_Vector ov = cast(other);
    if (size() == ov.V->size()) {
      // same size
      ValPtrVector::const_iterator myiter, oiter;
      for (myiter = V->begin(), oiter = ov.V->begin();
           myiter != V->end(); myiter++, oiter++) {
        int result = (*myiter)->compareTo(*oiter);
        if (result != 0) {
          // Found a field position for which we are different.  Return
          // the difference.
          return result;
        }
      }
      // All fields are equal
      return(0);
    }
    else if (size() < ov.V->size()) {
      return -1;
    } 
    else {
      return 1;
    }
  }
}
