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
#include "val_int64.h"

#include <iostream>

#include <prl/global.hpp>
#include <prl/factor/xml/constant_factor.hpp>
#include <prl/factor/xml/table_factor.hpp>
#include <prl/factor/xml/gaussian_factors.hpp>
#include <prl/factor/mixture.hpp>
#include <prl/factor/decomposable_fragment.hpp>
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

//! A decomposable fragment over Gaussians
typedef prl::decomposable_fragment<canonical_gaussian> fragment_gaussian;

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
    return Val_Factor::mk(constant_factor(0)-Val_Factor::cast(v1));
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
prl::named_universe Val_Factor::u;

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
  return u.new_vector_variable(name, size);
}

prl::variable_h
Val_Factor::registerFiniteVariable(const std::string& name, std::size_t size)
{
  return u.new_finite_variable(name, size);
}

std::vector<prl::variable_h>
Val_Factor::lookupVars(ListPtr list_ptr, bool ignore_missing) {
  // using namespace std;
  std::vector<variable_h> vars;
  vars.reserve(list_ptr->size());
  // cout << named_var << endl;
  foreach(ValuePtr val_ptr, std::make_pair(list_ptr->begin(),list_ptr->end())) {
    std::string name = val_ptr->toString();
    vars.push_back(u[name]);
  }
  return vars;
}

//! Returns the names for a set of variables as a list of strings
ValuePtr Val_Factor::names(const domain& args) {
  ValPtrList list;
  foreach(variable_h v, args)
    list.push_back(Val_Str::mk(v->name()));
  return Val_List::mk(ListPtr(new List(list)));
}

/////////////////////////////////////////////////////////
// Factor type registration
////////////////////////////////////////////////////////
struct factor_registration {

  factor_registration() {
    using namespace std;

    cerr << "Registering the factor types" << endl;
    typedef polymorphic_factor polymorphic;

    polymorphic::register_factor<constant_factor>();
    polymorphic::register_factor<table_factor>();
    polymorphic::register_factor<canonical_gaussian>();
    polymorphic::register_factor<moment_gaussian>();
    polymorphic::register_factor<fragment_gaussian>();
    
    polymorphic::register_binary<constant_factor, constant_factor>();
    polymorphic::register_binary<constant_factor, table_factor>();
    polymorphic::register_binary<constant_factor, moment_gaussian>();
    polymorphic::register_binary<constant_factor, canonical_gaussian>();
    polymorphic::register_binary<moment_gaussian, canonical_gaussian>();

    polymorphic::register_binary<fragment_gaussian, constant_factor>();
    polymorphic::register_binary<fragment_gaussian, canonical_gaussian>();
    polymorphic::register_binary<fragment_gaussian, fragment_gaussian>();
  }

} register_factors;

/////////////////////////////////////////////////////////
// Constructors, cast, serialization
////////////////////////////////////////////////////////

ValuePtr Val_Factor::mk()
{
  return ValuePtr(new Val_Factor());
}

ValuePtr Val_Factor::mk(const polymorphic_factor& factor)
{
  return ValuePtr(new Val_Factor(factor));
}

ValuePtr Val_Factor::mk(const prl::factor& factor)
{
  return ValuePtr(new Val_Factor(factor));
}

polymorphic_factor Val_Factor::cast(ValuePtr v)
{
  switch(v->typeCode()) {

  case Value::FACTOR:
    return static_cast<Val_Factor*>(v.get())->factor;

  case Value::DOUBLE:
    return polymorphic_factor(Val_Double::cast(v));

  case Value::INT64:
    return polymorphic_factor(Val_Double::cast(v));

  default:
    throw Value::TypeError(v->typeCode(),
                           v->typeName(),
                           Value::FACTOR,
                           "factor");
  }
}

std::string Val_Factor::toString() const
{
  return factor;
}

std::string Val_Factor::toConfString() const
{
  return factor;
}


int Val_Factor::compareTo(ValuePtr other) const
{
  if(other->typeCode() < Value::FACTOR)
    return -1;
  else if(other->typeCode() > Value::FACTOR)
    return 1;
  else {
    polymorphic_factor otherf = Val_Factor::cast(other);
//     if (factor < otherf) 
//       return -1;
//     else 
    return factor != otherf;
    // std::cerr << "Invoked compareTo: " << result << std::endl;
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
  prl::var_vector variables(toVarVector(vars));
  ListPtr values(Val_List::cast(vals));
  prl::assignment a;
  assert(vars->size() == vals->size());
  for(size_t i = 0; i < vars->size(); i++) {
    prl::variable_h v = variables[i];
    switch(v->type()) {
    case prl::variable::FINITE:
      {
        size_t val = Val_Int64::cast(values->at(i));
        assert(val < v->size());
        a[v] = val;
        break;
      }
    case prl::variable::VECTOR:
      {
        vector_type vec = Val_Vector::cast_double(values->at(i));
        assert(vec.size() == v->size());
        a[v] = vec;
        break;
      }
    default:
      assert(false);
    }
  }
  return a;
}
