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
#include "val_gaussian_mixture.h"
#include "val_int64.h"


class OperGaussianMixture : public opr::OperCompare<Val_Gaussian_Mixture> {
};

const opr::Oper* Val_Gaussian_Mixture::oper_ = new OperGaussianMixture();

const Val_Gaussian_Mixture::mixture_type& 
Val_Gaussian_Mixture::cast(ValuePtr v)
{
  switch(v->typeCode()) {

  case Value::GAUSSIAN_MIXTURE:
    return static_cast<Val_Gaussian_Mixture*>(v.get())->mixture;

  default:
    throw Value::TypeError(v->typeCode(),
                           v->typeName(),
                           Value::GAUSSIAN_MIXTURE,
                           "gaussian_mixture");
  }
}


// Marshal/Unmarshal essentially copied from Val_Tuple
void Val_Gaussian_Mixture::xdr_marshal_subtype( XDR *x )
{
  //to write
}

ValuePtr Val_Gaussian_Mixture::xdr_unmarshal( XDR *x )
{
  int64_t kmeans;
  xdr_int64_t(x, &kmeans);
  return mk(kmeans);
}

ValuePtr Val_Gaussian_Mixture::mk()
{
  return ValuePtr(new Val_Gaussian_Mixture());
}

ValuePtr Val_Gaussian_Mixture::mk(const mixture_type& factor)
{
  return ValuePtr(new Val_Gaussian_Mixture(factor));
}

string Val_Gaussian_Mixture::toString() const
{
  ostringstream s;
  s << "To do";
  return s.str();
}

string Val_Gaussian_Mixture::toConfString() const
{
  toString(); 
}

// Matrix comparison. Follow rules from Tuple
int Val_Gaussian_Mixture::compareTo(ValuePtr other) const
{
  return 1;
}
