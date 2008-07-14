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
#include "val_kmeans.h"
#include "val_int64.h"


class OperKmeans : public opr::OperCompare<Val_Kmeans> {
};

const opr::Oper* Val_Kmeans::oper_ = new OperKmeans();


int64_t Val_Kmeans::cast(ValuePtr v)
{
  switch (v->typeCode()) {
  case Value::KMEANS:
    return (static_cast<Val_Kmeans *>(v.get()))->kmeans;
  default:
    throw Value::TypeError(v->typeCode(), v->typeName(),
                           Value::KMEANS, "kmeans");
  }
}


// Marshal/Unmarshal essentially copied from Val_Tuple
void Val_Kmeans::xdr_marshal_subtype( XDR *x )
{
  //to write
}

ValuePtr Val_Kmeans::xdr_unmarshal( XDR *x )
{
  int64_t kmeans;
  xdr_int64_t(x, &kmeans);
  return mk(kmeans);
}

string Val_Kmeans::toString() const
{
  ostringstream s;
  s << kmeans;
  return s.str();
}

string Val_Kmeans::toConfString() const
{
  toString(); 
}

// Matrix comparison. Follow rules from Tuple
int Val_Kmeans::compareTo(ValuePtr other) const
{
  return 1;
}
