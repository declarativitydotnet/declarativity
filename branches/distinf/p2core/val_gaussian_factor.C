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
#include "val_gaussian_factor.h"
#include "val_str.h"

#include <prl/global.hpp>

#include "prl/detail/shortcuts_def.hpp"

class OperGaussianFactor : public opr::OperCompare<Val_Gaussian_Factor> {

};

const opr::Oper* Val_Gaussian_Factor::oper_ = new OperGaussianFactor();

const Val_Gaussian_Factor::canonical_gaussian& 
Val_Gaussian_Factor::cast(ValuePtr v)
{
  switch(v->typeCode()) {

  case Value::GAUSSIAN_FACTOR:
    return static_cast<Val_Gaussian_Factor*>(v.get())->factor;

  default:
    throw Value::TypeError(v->typeCode(),
                           v->typeName(),
                           Value::GAUSSIAN_FACTOR,
                           "gaussian_factor");
  }
}

std::string Val_Gaussian_Factor::toString() const
{
  std::stringstream ss;
  ss << factor;
  return ss.str();
}

int Val_Gaussian_Factor::compareTo(ValuePtr other) const
{
  if(other->typeCode() < Value::GAUSSIAN_FACTOR)
    return -1;
  else if(other->typeCode() > Value::GAUSSIAN_FACTOR)
    return 1;
  else {
    bool result = (factor != Val_Gaussian_Factor::cast(other));
    // std::cerr << "Invoked compareTo: " << result << std::endl;
    return result;
  }
}

void Val_Gaussian_Factor::xdr_marshal_subtype(XDR * x)
{
  // std::cerr << "Serializing " << factor << std::endl;

  // Serialize the factor into a string text archive
  std::stringstream ss;
  boost::archive::text_oarchive oa(ss);
  oa << const_cast<const Val_Gaussian_Factor*>(this)->factor;
  // std::cerr << "Serialized: " << ss.str() << std::endl;

  // Now marshal the string
  Val_Str(ss.str()).xdr_marshal_subtype(x);
}

ValuePtr Val_Gaussian_Factor::xdr_unmarshal(XDR * x)
{
  // Create the archive and deserialize its content
  std::stringstream ss(Val_Str::xdr_unmarshal(x)->toString());
  boost::archive::text_iarchive ia(ss);
  canonical_gaussian factor;
  ia >> factor;

  // Match the factor's variables against the current list
  prl::var_map subst_map;
  foreach(variable_h v, factor.arguments()) {
    subst_map[v] = registerVectorVariable(v->name(), v->size());
  }
  factor.subst_args(subst_map);

  // Delete the variable objects created during deserialization
//   foreach(prl::var_map::value_type& p, subst_map) {
//     if (p.first != p.second) // why can they be the same???
//       delete p.first;
//   }
  // std::cerr << "Deserialized " << factor << std::endl;
  return mk(factor);
}

ValuePtr Val_Gaussian_Factor::mk()
{
  return ValuePtr(new Val_Gaussian_Factor());
}

ValuePtr Val_Gaussian_Factor::mk(const canonical_gaussian& factor)
{
  return ValuePtr(new Val_Gaussian_Factor(factor));
}

ValuePtr Val_Gaussian_Factor::mk(ListPtr args, MatrixPtr mat, VectorPtr vec)
{
  matrix_type lambda(mat->size1(), mat->size2());
  vector_type eta(vec->size());
  assert(lambda.size1()==lambda.size2());
  boost::transform(mat->data(), lambda.data().begin(), ValuePtr2double());
  boost::transform(vec->data(), eta.data().begin(), ValuePtr2double());
  return mk(canonical_gaussian(lookupVars(args), lambda, eta));
}

prl::domain_t Val_Gaussian_Factor::arguments() const
{
  return factor.arguments();
}

ValuePtr Val_Gaussian_Factor::mean() const
{
  VectorPtr vector_ptr(new ValPtrVector(factor.size()));
  moment_gaussian mg(factor);
  boost::transform(mg.mean().data(), vector_ptr->data().begin(),
                   double2ValuePtr());
  return Val_Vector::mk(vector_ptr);
}

ValuePtr Val_Gaussian_Factor::covariance() const
{
  //typedef ublas::matrix< ValuePtr > ValPtrMatrix;
  //typedef boost::shared_ptr< ValPtrMatrix > MatrixPtr;
  MatrixPtr matrix_ptr(new ValPtrMatrix(factor.size(), factor.size()));
  moment_gaussian mg(factor);
  // Note that moment_gaussian stores its data in column-major format,
  // whereas ValPtrMatrix stores it in row-major format. Luckily,
  // the covariance matrix is symmetric, so this doesn't matter.
  boost::transform(mg.covariance().data(), matrix_ptr->data().begin(),
                   double2ValuePtr());
  return Val_Matrix::mk(matrix_ptr);
}

ValuePtr Val_Gaussian_Factor::multiply(ValuePtr other) const
{
  return mk(factor * Val_Gaussian_Factor::cast(other));
}

ValuePtr Val_Gaussian_Factor::marginal(ListPtr retain) const
{
  return mk(factor.marginal(domain_type(lookupVars(retain))));
}

ValuePtr Val_Gaussian_Factor::normalize() const
{
  return mk(factor); // Gaussians don't need to be normalized
}
