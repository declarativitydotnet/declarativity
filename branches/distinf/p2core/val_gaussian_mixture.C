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

// The archive classes must come before any export definitions.
#include <sstream>

#include "oper.h"
#include "val_null.h"
#include "val_str.h"
#include "val_gaussian_mixture.h"
#include "val_int64.h"

#include "prl/detail/shortcuts_def.hpp"


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
  // Serialize the mixture factor into a string text archive
  stringstream ss;
  boost::archive::text_oarchive oa(ss);
  oa << const_cast<const Val_Gaussian_Mixture*>(this)->mixture;
  // Now marshal the string
  Val_Str(ss.str()).xdr_marshal_subtype(x);
}

ValuePtr Val_Gaussian_Mixture::xdr_unmarshal( XDR *x )
{
  // Create the archive and deserialize its content
  std::stringstream ss(Val_Str::xdr_unmarshal(x)->toString());
  boost::archive::text_iarchive ia(ss);
  mixture_type mixture;
  ia >> mixture;
  return mk(mixture);
}

ValuePtr Val_Gaussian_Mixture::mk()
{
  return ValuePtr(new Val_Gaussian_Mixture());
}

ValuePtr Val_Gaussian_Mixture::mk(const mixture_type& m)
{
  return ValuePtr(new Val_Gaussian_Mixture(m));
}

string Val_Gaussian_Mixture::toString() const
{
  ostringstream s;
  s << mixture;
  return s.str();
}

string Val_Gaussian_Mixture::toConfString() const
{
  toString(); 
}

// Matrix comparison. Follow rules from Tuple
int Val_Gaussian_Mixture::compareTo(ValuePtr other) const
{
  // ****** To do
  return 1;
}

ValuePtr Val_Gaussian_Mixture::mk(string filename, int64_t dim, int64_t var, double regul) {
  size_t k = 2;
  boost::mt19937 rng;
  var_vector v = u.new_vector_variables(var, dim); // var variables of dim dimensions
  array_data<> data = load_plain< array_data<> >(filename, v);
  em_engine engine(&data, k);
  mixture_type m = engine.initialize(rng, regul);
  return mk(m);
}

ValuePtr Val_Gaussian_Mixture::emupdate(ValuePtr other) {
  mixture_type * m = Val_Gaussian_Mixture::cast(other);
  double log_lik = engine.expectation(m);
  m = engine.maximization(regul);
  return mk(m);
}


