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
#include "val_str.h"

#include <prl/global.hpp>

#include "prl/detail/shortcuts_def.hpp"

prl::universe Val_Factor::u;

Val_Factor::named_var_map Val_Factor::named_var;

std::string Val_Factor::toConfString() const
{
  return toString();
}

unsigned int Val_Factor::size() const
{
  return arguments().size();
}

ValuePtr Val_Factor::args() const
{
  //typedef std::list< ValuePtr > ValPtrList;
  ValPtrList list;
  foreach(variable_h v, this->arguments()) {
    list.push_back(Val_Str::mk(v->name()));
  }
  return Val_List::mk(ListPtr(new List(list)));
}

prl::variable_h 
Val_Factor::registerVectorVariable(const std::string& name, std::size_t size)
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

prl::variable_h
Val_Factor::registerFiniteVariable(const std::string& name, std::size_t size)
{
  variable_h v;
  if (named_var.contains(name)) {
    v = named_var[name];
    assert(v->size() == size);
  } else {
    variable_h v = u.new_finite_variable(name, size);
    named_var[name] = v;
  }
  return v;
}

std::vector<prl::variable_h> Val_Factor::lookupVars(ListPtr list_ptr) {
  std::vector<variable_h> vars;
  vars.reserve(list_ptr->size());
  foreach(ValuePtr val_ptr, std::make_pair(list_ptr->begin(),list_ptr->end())) {
    vars.push_back(named_var.get(val_ptr->toString()));
  }
  return vars;
}
