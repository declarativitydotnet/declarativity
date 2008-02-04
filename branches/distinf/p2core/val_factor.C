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
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include "oper.h"

#include "val_null.h"
#include "val_factor.h"
#include "val_str.h"

#include <prl/global.hpp>

#include "prl/detail/shortcuts_def.hpp"

class OperFactor : public opr::OperCompare<Val_Factor> {

};

const opr::Oper* Val_Factor::oper_ = new OperFactor();

prl::universe Val_Factor::u;

Val_Factor::named_var_map Val_Factor::named_var;

const Val_Factor::canonical_gaussian& Val_Factor::cast(ValuePtr v)
{
  switch(v->typeCode()) {

  case Value::FACTOR:
    return static_cast<Val_Factor*>(v.get())->factor;

  default:
    throw Value::TypeError(v->typeCode(),
                           v->typeName(),
                           Value::FACTOR,
                           "factor");
  }
}

std::string Val_Factor::toString() const
{
  std::stringstream ss;
  ss << factor;
  return ss.str();
}

std::string Val_Factor::toConfString() const
{
  std::cerr << "Val_Factor::toConfString" << std::endl;
  return toString();
}

int Val_Factor::compareTo(ValuePtr other) const
{
  if(other->typeCode() < Value::FACTOR)
    return -1;
  else if(other->typeCode() > Value::FACTOR)
    return 1;
  else {
    bool result = (factor != Val_Factor::cast(other));
    std::cerr << "Invoked compareTo: " << result << std::endl;
    return result;
  }
}

void Val_Factor::xdr_marshal_subtype(XDR * x)
{
  // std::cerr << "Serializing " << factor << std::endl;

  // Serialize the factor into a string text archive
  std::stringstream ss;
  boost::archive::text_oarchive oa(ss);
  oa << const_cast<const Val_Factor*>(this)->factor;
  // std::cerr << "Serialized: " << ss.str() << std::endl;

  // Now marshal the string
  Val_Str(ss.str()).xdr_marshal_subtype(x);
}

ValuePtr Val_Factor::xdr_unmarshal(XDR * x)
{
  // Create the archive and deserialize its content
  std::stringstream ss(Val_Str::xdr_unmarshal(x)->toString());
  boost::archive::text_iarchive ia(ss);
  canonical_gaussian factor;
  ia >> factor;

  // Match the factor's variables against the current list
  prl::var_map subst_map;
  foreach(variable_h v, factor.arguments()) {
    subst_map[v] = registerVectorVariable(v->name(), v->as_vector().size());
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

ValuePtr Val_Factor::mk()
{
  return ValuePtr(new Val_Factor());
}

ValuePtr Val_Factor::mk(const canonical_gaussian& factor)
{
  return ValuePtr(new Val_Factor(factor));
}

ValuePtr Val_Factor::mk(ListPtr args, MatrixPtr mat, VectorPtr vec)
{
  matrix_type lambda(mat->size1(), mat->size2());
  vector_type eta(vec->size());
  assert(lambda.size1()==lambda.size2());
  boost::transform(mat->data(), lambda.data().begin(), ValuePtr2double());
  boost::transform(vec->data(), eta.data().begin(), ValuePtr2double());
  return mk(canonical_gaussian(lookupVars(args), lambda, eta));
}

unsigned int Val_Factor::size() const
{
  return factor.arguments().size();
}

ValuePtr Val_Factor::arguments() const
{
  //typedef std::list< ValuePtr > ValPtrList;
  ValPtrList list;
  foreach(variable_h v, factor.arguments()) {
    list.push_back(Val_Str::mk(v->name()));
  }
  return Val_List::mk(ListPtr(new List(list)));
}

ValuePtr Val_Factor::mean() const
{
  VectorPtr vector_ptr(new ValPtrVector(factor.size()));
  moment_gaussian mg(factor);
  boost::transform(mg.mean().data(), vector_ptr->data().begin(),
                   double2ValuePtr());
  return Val_Vector::mk(vector_ptr);
}

ValuePtr Val_Factor::covariance() const
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

prl::variable_h Val_Factor::registerVectorVariable(std::string name, std::size_t size)
{
  using namespace std;
  variable_h v;
  if (named_var.contains(name)) {
    v = named_var[name];
    assert(v->as_vector().size() == size);
  } else {
    variable_h v = u.new_vector_variable(name, size);
    named_var[name] = v;
  }
  // cerr << "Registered variable " << name << endl;
  // cerr << named_var << endl;
  return v;
}

ValuePtr Val_Factor::multiply(const canonical_gaussian & f1, const canonical_gaussian & f2)
{
  return mk(f1*f2);
}

ValuePtr Val_Factor::marginal(const canonical_gaussian & f1, ListPtr retain)
{
  return mk(f1.marginal(domain_type(lookupVars(retain))));
}

std::vector<prl::variable_h> Val_Factor::lookupVars(ListPtr list_ptr) {
  std::vector<variable_h> vars;
  vars.reserve(list_ptr->size());
  foreach(ValuePtr val_ptr, std::make_pair(list_ptr->begin(), list_ptr->end())) {
    vars.push_back(named_var.get(val_ptr->toString()));
  }
  return vars;
}
