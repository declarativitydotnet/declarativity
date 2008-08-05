/*
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776,
 * Berkeley, CA,  94707. Attention: P2 Group.
 *
 * DESCRIPTION: P2's concrete type system: the factor type.
 *
 */


#ifndef __VAL_FACTOR_H__
#define __VAL_FACTOR_H__

// #define PRL_PRINT_VARIABLE_ADDRESS

#include "value.h"
#include "oper.h"
#include "val_double.h"
#include "val_list.h"

// #include <boost/thread/mutex.hpp>
// #include <boost/archive/text_iarchive.hpp>
// #include <boost/archive/text_oarchive.hpp>

#include <prl/variable.hpp>
#include <prl/named_universe.hpp>
#include <prl/factor/xml/polymorphic_factor.hpp>

typedef prl::polymorphic_factor<double> polymorphic_factor;

class Val_Factor : public Value {
 public:
  /////////////////////////////////////////////////////////
  // Public types
  ////////////////////////////////////////////////////////

  //! The type of argument sets used in the factors
  typedef prl::domain domain;

  //! The type of variable handles (pointers)
  typedef prl::variable_h variable_h;

  //! A map from variable names to handles
  typedef prl::map<std::string, variable_h> named_var_map;

  // Required type information
  const Value::TypeCode typeCode() const { return Value::FACTOR; };
  const char *typeName() const { return "factor"; };

  static const opr::Oper* oper_;

  /////////////////////////////////////////////////////////
  // Public member functions
  ////////////////////////////////////////////////////////

  // Constructors
  Val_Factor() { }
  Val_Factor(const polymorphic_factor& f) : factor(f) { }
  Val_Factor(const prl::factor& f) : factor(f) {}
  ~Val_Factor() { }

  static ValuePtr mk();
  static ValuePtr mk(const prl::factor& f);
  static ValuePtr mk(const polymorphic_factor& f);

  // Casts
  static polymorphic_factor cast(ValuePtr v);
  const ValuePtr toMe(ValuePtr other) const { return mk(cast(other)); }

  // String representation. What exactly is toConfString()?
  std::string toString() const;
  std::string toConfString() const;

  // Serialization
  void xdr_marshal_subtype( XDR *x );
  static ValuePtr xdr_unmarshal( XDR *x );

  //! Registers the local variable by name
  static variable_h 
    registerVectorVariable(const std::string& name, std::size_t size);

  static variable_h
    registerFiniteVariable(const std::string& name, std::size_t size);

  static variable_h
    registerVariable(const std::string& name, std::size_t size,
                     prl::variable::variable_type type);
  
  // Comparison
  int compareTo(ValuePtr v) const;

  //! Returns the number of arguments
  unsigned int size() const;

  //! Converts a list of variable names to a vector of variable handles
  static std::vector<variable_h> 
    lookupVars(ListPtr list_ptr, bool ignore_missing = false);

  //! Returns the names for a set of variables as a list
  static ValuePtr names(const domain& args);
  
 public:
  //! The set of all variables known to this host. (public for now)
  static prl::named_universe u;

 private:
  //! The underlying factor
  polymorphic_factor factor;

};

// Free functions
//! Converts a list of variable names to a vector of variable handles
prl::var_vector toVarVector(ValuePtr vars);

//! Converts a list of variable names to a domain
prl::domain toDomain(ValuePtr vars);

//! Converts a list of variable names and values to an assignment
prl::assignment toAssignment(ValuePtr vars, ValuePtr vals);


#endif /* __VAL_FACTOR_H_*/
