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

#include "oper.h"

#include "val_null.h"
#include "val_factor.h"
#include "val_matrix.h"
#include "val_vector.h"
#include "val_str.h"

#include <iostream>

#include <prl/global.hpp>
#include <prl/factor/constant_factor.hpp>
#include <prl/factor/table_factor.hpp>
#include <prl/factor/gaussian_factors.hpp>
#include <prl/factor/mixture.hpp>
#include <prl/math/bindings/lapack.hpp>

#include <pstade/oven/algorithm.hpp>

#include "prl/detail/shortcuts_def.hpp"


/////////////////////////////////////////////////////////
// Types
////////////////////////////////////////////////////////

//! The matrix representation
typedef prl::math::bindings::lapack::double_matrix matrix_type;

//! The vector representation
typedef prl::math::bindings::lapack::double_vector vector_type;

//! A constant factor
typedef prl::constant_factor<> constant_factor;

//! A table factor
typedef prl::table_factor<> table_factor;

//! The type of factors stored in this value
typedef prl::canonical_gaussian<matrix_type, vector_type> canonical_gaussian;

//! The moment Gaussian factor type (used for mean, covariance)
typedef prl::moment_gaussian<matrix_type, vector_type> moment_gaussian;


/////////////////////////////////////////////////////////
// Arithmetic operations
////////////////////////////////////////////////////////

class OperFactor : public opr::OperCompare<Val_Factor> {
  virtual ValuePtr _plus (const ValuePtr& v1, const ValuePtr& v2) const {
    return Val_Factor::mk(Val_Factor::cast(v1) + Val_Factor::cast(v2));
  };
  virtual ValuePtr _minus (const ValuePtr& v1, const ValuePtr& v2) const {
    return Val_Factor::mk(Val_Factor::cast(v1) - Val_Factor::cast(v2));
  };
  virtual ValuePtr _neg (const ValuePtr& v1) const {
    return Val_Factor::mk(-Val_Factor::cast(v1));
  };
  virtual ValuePtr _times (const ValuePtr& v1, const ValuePtr& v2) const {
    return Val_Factor::mk(Val_Factor::cast(v1) * Val_Factor::cast(v2));
  };
  virtual ValuePtr _divide (const ValuePtr& v1, const ValuePtr& v2) const {
    return Val_Factor::mk(Val_Factor::cast(v1) / Val_Factor::cast(v2));
  };
};

const opr::Oper* Val_Factor::oper_ = new OperFactor();

/////////////////////////////////////////////////////////
// Variable registration
////////////////////////////////////////////////////////

// The universe of all variables
prl::universe Val_Factor::u;

// The variable that corresponds to each name
Val_Factor::named_var_map Val_Factor::named_var;

// Mutex for variable registration
boost::mutex Val_Factor::mutex;

prl::variable_h
Val_Factor::registerVariable(const std::string& name, std::size_t size,
                             prl::variable::variable_type type) {
  switch(type) {
  case prl::variable::FINITE:
    return registerFiniteVariable(name, size);
  case prl::variable::VECTOR:
    return registerVectorVariable(name, size);
  default:
    assert(false);
  }
}

prl::variable_h 
Val_Factor::registerVectorVariable(const std::string& name, std::size_t size)
{
  boost::mutex::scoped_lock scoped_lock(mutex);
  using namespace std;
  variable_h v;
  if (named_var.contains(name)) {
    v = named_var[name];
    assert(v->as_vector().size() == size);
  } else {
    v = u.new_vector_variable(name, size);
    named_var[name] = v;
    TELL_WARN << "Registered variable " << v << endl;
    // cerr << "Registered new variable " << v << endl;
  }
  // cerr << "New list of variables: " << named_var << endl;
  return v;
}

prl::variable_h
Val_Factor::registerFiniteVariable(const std::string& name, std::size_t size)
{
  boost::mutex::scoped_lock scoped_lock(mutex);
  using namespace std;
  variable_h v;
  if (named_var.contains(name)) {
    v = named_var[name];
    assert(v->size() == size);
  } else {
    v = u.new_finite_variable(name, size);
    named_var[name] = v;
    TELL_WARN << "Registered variable " << v << endl;
    // cerr << "Registered new variable " << v << endl;
  }
  // cerr << "New list of variables: " << named_var << endl;
  return v;
}

std::vector<prl::variable_h>
Val_Factor::lookupVars(ListPtr list_ptr, bool ignore_missing) {
  boost::mutex::scoped_lock scoped_lock(mutex);
  // using namespace std;
  std::vector<variable_h> vars;
  vars.reserve(list_ptr->size());
  // cout << named_var << endl;
  foreach(ValuePtr val_ptr, std::make_pair(list_ptr->begin(),list_ptr->end())) {
    std::string name = val_ptr->toString();
    if (named_var.contains(name)) 
      vars.push_back(named_var.get(val_ptr->toString()));
    else if (!ignore_missing) {
      std::cerr << "Cannot find variable named " << name << std::endl;
      assert(false);
    }
  }
  return vars;
}

/////////////////////////////////////////////////////////
// Constructors, cast, serialization
////////////////////////////////////////////////////////

polymorphic_factor Val_Table_Factor::cast(ValuePtr v)
{
  switch(v->typeCode()) {

  case Value::FACTOR:
    return static_cast<Val_Table_Factor*>(v.get())->factor;

  case Value::DOUBLE
    return polymorphic_factor(Val_Double::cast(v));

  default:
    throw Value::TypeError(v->typeCode(),
                           v->typeName(),
                           Value::FACTOR,
                           "factor");
  }
}

std::string Val_Table_Factor::toString() const
{
  return factor;
}

std::string Val_Factor::toConfString() const
{
  return factor;
}


int Val_Table_Factor::compareTo(ValuePtr other) const
{
  if(other->typeCode() < Value::FACTOR)
    return -1;
  else if(other->typeCode() > Value::FACTOR)
    return 1;
  else {
    factor_type otherf = Val_Factor::cast(other);
//     if (factor < otherf) 
//       return -1;
//     else 
    return factor != otherf;
    // std::cerr << "Invoked compareTo: " << result << std::endl;
  }
}

void Val_Table_Factor::xdr_marshal_subtype(XDR * x)
{
  // std::cerr << "Serializing " << factor << std::endl;

  // Serialize the factor into a string text archive
  std::stringstream ss;
  boost::archive::text_oarchive oa(ss);
  oa << factor;
  // std::cerr << "Serialized: " << ss.str() << std::endl;

  // Now marshal the string
  Val_Str(ss.str()).xdr_marshal_subtype(x);
}

ValuePtr Val_Table_Factor::xdr_unmarshal(XDR * x)
{
  using namespace std;

  // Create the archive and deserialize its content
  std::stringstream ss(Val_Str::xdr_unmarshal(x)->toString());
  boost::archive::text_iarchive ia(ss);
  polymorphic_factor factor;
  ia >> factor;

  // Match the factor's variables against the current list
  prl::var_map subst_map;
  // cerr << "Unmarshaled " << factor << endl;
  foreach(variable_h v, factor.arguments()) {
    subst_map[v] = registerVariable(v->name(), v->size(), v->type());
    // cerr << "(v,subst_map[v])=" << v << "," << subst_map[v] << endl;
    assert(v != subst_map[v]);
  }
  factor.subst_args(subst_map);

  // Delete the variable objects created during deserialization
  foreach(prl::var_map::value_type& p, subst_map) delete p.first;
  
  // cerr << "Deserialized " << factor << endl;
  return mk(factor);
}


/////////////////////////////////////////////////////////
// Accessors and factor operations
////////////////////////////////////////////////////////

unsigned int Val_Factor::size() const {
  return factor.arguments().size();
}

prl::domain Val_Factor::arguments() const {
  return factor.arguments();
}

ValuePtr Val_Factor::arg_names() const {
  ValPtrList list;
  foreach(variable_h v, this->arguments()) {
    list.push_back(Val_Str::mk(v->name()));
  }
  return Val_List::mk(ListPtr(new List(list)));
}

ValuePtr marginal(ListPtr retain) const {
  return mk(factor.marginal(domain(lookupVars(retain, true))));
}
  
ValuePtr maximum(ListPtr retain) const {
  return mk(factor.maximum(domain(lookupVars(retain, true))));
}

ValuePtr minimum(ListPtr retain) const {
  return mk(factor.maximum(domain(lookupVars(retain, true))));
}

ValuePtr restrict(ListPtr variables, ListPtr values) const {
  assert(false); // not implemented yet
}

ValuePtr normalize() const {
  return mk(polymorphic_factor(factor).normalize());
}

ValuePtr values() const {
  using pstade::oven::transform;
  VectorPtr vector_ptr(new ValPtrVector(factor.size()));
  transform(factor.values(), vector_ptr->data().begin(), double2ValuePtr());
  return Val_Vector::mk(vector_ptr);
}

ValuePtr Val_Gaussian_Factor::mean() const
{
  using pstade::oven::transform;
  VectorPtr vector_ptr(new ValPtrVector(factor.size()));
  moment_gaussian mg(factor.as<moment_gaussian>());
  transform(mg.mean().data(), vector_ptr->data().begin(),
            double2ValuePtr());
  return Val_Vector::mk(vector_ptr);
}


ValuePtr Val_Gaussian_Factor::covariance() const
{
  using pstade::oven::transform;
  MatrixPtr matrix_ptr(new ValPtrMatrix(factor.size(), factor.size()));
  moment_gaussian mg(factor.as<moment_gaussian>());
  // Note that moment_gaussian stores its data in column-major format,
  // whereas ValPtrMatrix stores it in row-major format. Luckily,
  // the covariance matrix is symmetric, so this doesn't matter.
  transform(mg.covariance().data(), matrix_ptr->data().begin(),
            double2ValuePtr());
  return Val_Matrix::mk(matrix_ptr);
}

/////////////////////////////////////////////////////////
// Free functions
////////////////////////////////////////////////////////
prl::var_vector toVarVector(ValuePtr vars) {
  return Val_Factor::lookupVars(Val_List::cast(vars));
}

prl::domain toDomain(ValuePtr vars) {
  return prl::domain(Val_Factor::lookupVars(Val_List::cast(vars)));
}

prl::assignment toAssignment(ValuePtr vars, ValuePtr vals) {
  prl::var_vector variables(varVector(vars));
  ListPtr values(Val_List::cast(vals));
  assignment a;
  size_t pos = 0;
  assert(vars.size() == vals->size());
  for(size_t i = 0; i < vars.size(); i++) {
    prl::variable_h v = variables[i];
    switch(v.type()) {
    case prl::variable::FINITE:
      size_t val = Val_Int64::cast(values[i]);
      assert(val < v->size());
      a[v] = val;
      break;
    case prl::variable::VECTOR:
      double_vector vec = doubleVector(values[i]);
      assert(vec.size() == v->size());
      a[v] = vec;
      break;
    defaul:
      assert(false);
    }
  }
  return a;
}
