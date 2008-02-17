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
#include "val_table_factor.h"
#include "val_str.h"
#include "val_int64.h"
#include "val_vector.h"

#include <prl/global.hpp>

#include "prl/detail/shortcuts_def.hpp"

class OperTableFactor : public opr::OperCompare<Val_Table_Factor> {

};

const opr::Oper* Val_Table_Factor::oper_ = new OperTableFactor();

const Val_Table_Factor::factor_type& 
Val_Table_Factor::cast(ValuePtr v)
{
  switch(v->typeCode()) {

  case Value::TABLE_FACTOR:
    return static_cast<Val_Table_Factor*>(v.get())->factor;

  default:
    throw Value::TypeError(v->typeCode(),
                           v->typeName(),
                           Value::TABLE_FACTOR,
                           "table_factor");
  }
}

std::string Val_Table_Factor::toString() const
{
  std::stringstream ss;
  ss << factor;
  return ss.str();
}

int Val_Table_Factor::compareTo(ValuePtr other) const
{
  if(other->typeCode() < Value::TABLE_FACTOR)
    return -1;
  else if(other->typeCode() > Value::TABLE_FACTOR)
    return 1;
  else {
    bool result = (factor != Val_Table_Factor::cast(other));
    // std::cerr << "Invoked compareTo: " << result << std::endl;
    return result;
  }
}

void Val_Table_Factor::xdr_marshal_subtype(XDR * x)
{
  // std::cerr << "Serializing " << factor << std::endl;

  // Serialize the factor into a string text archive
  std::stringstream ss;
  boost::archive::text_oarchive oa(ss);
  oa << const_cast<const Val_Table_Factor*>(this)->factor;
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
  factor_type factor;
  ia >> factor;

  // Match the factor's variables against the current list
  prl::var_map subst_map;
  // cerr << "Unmarshaled " << factor << endl;
  foreach(variable_h v, factor.arguments()) {
    subst_map[v] = registerFiniteVariable(v->name(), v->size());
    // cerr << "(v,subst_map[v])=" << v << "," << subst_map[v] << endl;
    assert(v != subst_map[v]);
  }
  factor.subst_args(subst_map);

  // Delete the variable objects created during deserialization
  foreach(prl::var_map::value_type& p, subst_map) delete p.first;
  
  // cerr << "Deserialized " << factor << endl;
  return mk(factor);
}

ValuePtr Val_Table_Factor::mk()
{
  return ValuePtr(new Val_Table_Factor());
}

ValuePtr Val_Table_Factor::mk(const factor_type& factor)
{
  return ValuePtr(new Val_Table_Factor(factor));
}

ValuePtr Val_Table_Factor::mk(ListPtr args, MatrixPtr p_values)
{
  const ValPtrMatrix& values = *p_values;
  std::vector<variable_h> vars = lookupVars(args);
  assert(values.size2() == vars.size()+1);
  factor_type f(vars, 0);

  prl::assignment_t a;
  for(size_t i = 0; i<values.size1(); i++) {
    // construct the assignment
    for(size_t j = 0; j<vars.size(); j++) {
      size_t aj = Val_Int64::cast(values[i][j]);
      assert(aj < vars[j]->size());
      a[vars[j]] = aj;
    }
    // set the factor value for the given assignment of variables
    f(a) = Val_Double::cast(values[i][vars.size()]);
  }
  
  return mk(f);
}

prl::domain_t Val_Table_Factor::arguments() const
{
  return factor.arguments();
}

ValuePtr Val_Table_Factor::multiply(ValuePtr other) const
{
  return mk(factor * Val_Table_Factor::cast(other));
}

ValuePtr Val_Table_Factor::marginal(ListPtr retain) const
{
  return mk(factor.marginal(domain_type(lookupVars(retain))));
}

ValuePtr Val_Table_Factor::normalize() const
{
  return mk(factor_type(factor).normalize());
}

ValuePtr Val_Table_Factor::values() const
{
  VectorPtr vector_ptr(new ValPtrVector(factor.size()));
  boost::transform(factor, vector_ptr->data().begin(), double2ValuePtr());
  return Val_Vector::mk(vector_ptr);
}
