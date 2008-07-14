/*
 *
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 * DESCRIPTION: Parsing environment for Overlog (the P2 dialect of datalog)
 *
 */

#include <iostream>
#include <fstream>
#include <sys/wait.h>
#include <unistd.h>
#include <list>
#include "parseContext.h"
#include "plumber.h"
#include "olg_lexer.h"
#include "tuple.h"
#include "systemTable.h"
#include "refTable.h"
#include "val_tuple.h"
#include "val_vector.h"
#include "val_matrix.h"
#include "val_list.h"
#include "val_int64.h"
#include "val_str.h"
#include "val_time.h"
#include "oper.h"

namespace compile {
  namespace parse {
    using namespace opr;

    const string Context::STAGEVARPREFIX = "PS_";
    ValuePtr 
    Value::tuple() const
    {
      TuplePtr tp = Tuple::mk(VAL);
      tp->append(_value);
      tp->freeze();
      return Val_Tuple::mk(tp);
    }
  
    ValuePtr 
    Variable::tuple() const
    {
      TuplePtr tp = Tuple::mk((_location ? LOC : (_newLocSpec ? NEWLOCSPEC: VAR)));
      tp->append(_value);
      tp->freeze();
      return Val_Tuple::mk(tp);
    }
    
    Aggregation::Aggregation(string aggName, Expression *v) 
    {
      _variable = dynamic_cast<Variable*>(v);
      _operName = aggName;
    }

    string 
    Aggregation::toString() const {
      ostringstream a;
      a << _operName << "< ";
      a << (_variable != NULL?_variable->toString():"*") << " >";
      return a.str();
    }
    
    
    ValuePtr
    Aggregation::tuple() const
    {
      TuplePtr tp = Tuple::mk(AGG);
      tp->append(_variable ? _variable->tuple() : Val_Null::mk());
      tp->append(Val_Str::mk(_operName));
      tp->freeze();
      return Val_Tuple::mk(tp);
    }
    
    const string Function::max = "f_max";
    const string Function::concat = "f_concat";
    const string Function::mod = "f_mod";
    const string Says::verTable = "::verKey"; 
    const string Says::genTable = "::genKey"; 
    const string Says::hashFunc = "f_sha1"; 
    const string Says::verFunc = "f_verify"; 
    const string Says::genFunc = "f_gen"; 
    const string Says::encHint = "::encHint"; 
    const string Says::varPrefix = "PRW_"; 
    const string Says::rulePrefix = "prw_"; 
    const string Says::saysSuffix = "Says"; 
    const string Says::saysPrefix = "says"; 
    const string Says::makeSays = "makeSays"; 
    const string Says::globalScope = "::"; 
    const uint32_t TableEntry::numSecureFields = 5;
    uint32_t Functor::fictVarCounter = 0;
    const string Functor::FICTPREFIX = "NH";
    uint32_t Rule::ruleId = 0; 
    const bool Table::compoundRewrite = false;
    SetPtr Table::materializedSaysTables(new Set());
    
    string 
    Bool::toString() const {
      ostringstream b;
      if (_rhs) {
        b << "( " << _lhs->toString() << " )";
        b << " " << _operName << " ";
        b << "( " << _rhs->toString() << " )";
      }
      else {
        b << " " << _operName << "( ";
        b << _lhs->toString();
        b << " )";
      }
      return b.str();
    }
    
    ValuePtr
    Bool::tuple() const
    {
      TuplePtr tp = Tuple::mk(BOOL);
      
      tp->append(Val_Str::mk(_operName));
      tp->append(_lhs->tuple());
      if (_rhs) tp->append(_rhs->tuple());
      else tp->append(Val_Null::mk());
      tp->freeze();
      return Val_Tuple::mk(tp);
    }
    
    string 
    Range::toString() const {
      switch (_type) {
      case RANGEOO: 
        return "(" + _lhs->toString() + ", " + _rhs->toString() + ")"; break;
      case RANGEOC: 
        return "(" + _lhs->toString() + ", " + _rhs->toString() + "]"; break;
      case RANGECO: 
        return "[" + _lhs->toString() + ", " + _rhs->toString() + ")"; break;
      case RANGECC: 
        return "[" + _lhs->toString() + ", " + _rhs->toString() + "]"; break;
      default:
        ostringstream f;
        f << "parseContext.C: Unrecognized range type '"
          << _type
          << "'";
        throw compile::parse::Exception(__LINE__, f.str());
        return "";
      }
    }
    
    ValuePtr
    Range::tuple() const 
    {
      TuplePtr tp = Tuple::mk(RANGE);
    
      tp->append(Val_Str::mk(_operName));
      tp->append(_lhs->tuple());
      tp->append(_rhs->tuple());
      tp->freeze();
      return Val_Tuple::mk(tp);
    }
    
    
    const ValuePtr 
    Math::value() const {
      Math  *m;
      Value *v;
      ValuePtr l;
      ValuePtr r;
      
      if ((m=dynamic_cast<Math*>(_lhs)) != NULL) 
        l = m->value();
      else if ((v=dynamic_cast<Value*>(_lhs)) != NULL) 
        l = v->value();
      else
        return ValuePtr();
    
      if ((m=dynamic_cast<Math*>(_rhs)) != NULL) 
        r = m->value();
      else if ((v=dynamic_cast<Value*>(_rhs)) != NULL) 
        r = v->value();
      else
        return ValuePtr();	// TODO: throw some kind of exception.
    
      if (!l || !r) return ValuePtr();
    
      if (_operName == "<<") return l << r;
      if (_operName == ">>") return l << r;
      if (_operName == "+")  return l + r;
      if (_operName == "-")  return l - r;
      if (_operName == "*")  return l * r;
      if (_operName == "/")  return l / r;
      if (_operName == "%")  return l % r;
      return ValuePtr();
    }
    
    string 
    Math::toString() const {
      return _lhs->toString() + " " + _operName + " " + _rhs->toString();
    }
    
    ValuePtr
    Math::tuple() const 
    {
      TuplePtr tp = Tuple::mk(MATH);
    
      tp->append(Val_Str::mk(_operName));
      tp->append(_lhs->tuple());
      tp->append(_rhs->tuple());
      tp->freeze();
      return Val_Tuple::mk(tp);
    }
    
    string 
    Function::toString() const {
      ostringstream f;
      f << _name << "( ";
      for (ExpressionList::iterator iter = _args->begin(); 
           iter != _args->end(); iter++) {
        f << (*iter)->toString();
        if (iter+1 != _args->end()) f << ", ";
      }
      f << ")";
      return f.str();
    }
    
    ValuePtr
    Function::tuple() const 
    {
      TuplePtr tp = Tuple::mk(FUNCTION);
  
      tp->append(Val_Str::mk(_name));
      tp->append(Val_Int64::mk(_args->size()));
      for (ExpressionList::iterator iter = _args->begin(); 
           iter != _args->end(); iter++) {
        tp->append((*iter)->tuple());
      }
  
      tp->freeze();
      return Val_Tuple::mk(tp);
    }

    string 
    Sets::toString() const {
      ostringstream f;
      f  << "{ ";
      for (ExpressionList::iterator iter = _args->begin(); 
           iter != _args->end(); iter++) {
        f << (*iter)->toString();
        if (iter+1 != _args->end()) f << ", ";
      }
      f << "}";
      return f.str();
    }
    
    ValuePtr
    Sets::tuple() const 
    {
      TuplePtr tp = Tuple::mk(SETS);
  
      tp->append(Val_Str::mk(SETS));
      tp->append(Val_Int64::mk(_args->size()));

      for (ExpressionList::iterator iter = _args->begin(); 
           iter != _args->end(); iter++) {
        tp->append((*iter)->tuple());
      }
  
      tp->freeze();
      return Val_Tuple::mk(tp);
    }
    
    Vector::Vector(ExpressionList *v) {
      VectorPtr vp(new boost::numeric::ublas::vector<ValuePtr>(v->size()));
      uint i = 0;
      for (ExpressionList::iterator iter = v->begin(); 
           iter != v->end(); iter++) {
        Value *val = dynamic_cast<Value*>(*iter);
        (*vp)(i++) = val->value();
      }
      _vector = Val_Vector::mk(vp);
    }
    
    ValuePtr
    Vector::tuple() const 
    {
      TuplePtr tp = Tuple::mk(VEC);
      tp->append(_vector);
      tp->freeze();
      return Val_Tuple::mk(tp);
    }
    
    
    Matrix::Matrix(ExpressionListList *m) 
    {
    
      // Confirm conformity first, then allocate and initialize rows.
      const uint columnArity = m->at(0)->size();
      for (ExpressionListList::iterator row = m->begin(); 
           row != m->end(); row++) {
        if ((*row)->size() != columnArity) {
          ostringstream oss;
          oss << "matrix has rows of nonuniform length";
          throw compile::Exception(oss.str());
        }
      }
    
      MatrixPtr mp(new boost::numeric::ublas::matrix<ValuePtr>(m->size(), columnArity));
      uint r=0;
      for (ExpressionListList::iterator row = m->begin(); 
           row != m->end(); row++, r++) {
        uint c=0;
        for (ExpressionList::iterator col = (*row)->begin();
             col != (*row)->end(); col++, c++) {
          Value *val = dynamic_cast<Value*>(*col);
          (*mp)(r, c) = val->value();
        }
      }
      _matrix = Val_Matrix::mk(mp);
    }
    
    ValuePtr
    Matrix::tuple() const 
    {
      TuplePtr tp = Tuple::mk(MAT);
      tp->append(_matrix);
      tp->freeze();
      return Val_Tuple::mk(tp);
    }
    
    
    /***************************************************
     * TERMS
     ***************************************************/
    
    string 
    Functor::toString() const {
      ostringstream f;
      if(_new){
	f<<"new< " << _locSpec->toString()<<", "<<_opaque->toString()<<", "<<_hint->toString()<<", ";;
      }

      f << (_complement?"!":"") <<name()<<  (_complement?"^":"") << "( ";
      for (ExpressionList::const_iterator iter = _args->begin(); 
           iter != _args->end(); iter++) {
        f << (*iter)->toString();
        if (iter+1 != _args->end()) f << ", ";
      }
      f << " )" << (_new?">":"");
      return f.str();
    }
    
    void Functor::processNew(TermList* termList, Expression *assignedVal){
      assert(_new);
      getLocSpec()->resetLocSpec();
      
      ostringstream oss;
      oss << FICTPREFIX << fictVarCounter++; 
      Variable *hintVar = new compile::parse::Variable(Val_Str::mk(oss.str())); 
      _args->push_front(hintVar->copy()); 
      
      _args->push_front(_opaque);
      _args->push_front(_locSpec);
      Variable *var;
      if ((var = dynamic_cast<Variable*>(_args->at(0))) != NULL &&
          !var->location()) {
	throw compile::Exception("Invalid location specifier. Variable: " 
				 + var->toString());
      }
      if(assignedVal != NULL){
	Term *assignTerm = new Assign(hintVar, assignedVal);
	//NULL if head is new: if yes, then put null in the hintFieldPos
	
	//	Value* null = new Value(Val_Null::mk());
	//	ExpressionList *argsList =  const_cast<ExpressionList*>(_head->arguments());
	//	ExpressionList::iterator iter = argsList->begin();
	//	iter++; // for location
	//	iter++; // for opaque
	//	Variable *var = dynamic_cast<Variable*>(*iter);
	//	_head->replace(iter, null);
	//	delete var;
	termList->push_back(assignTerm);
	
      }
      
      _name =  _name + compile::NEWSUFFIX;
    }
  

    string 
    Says::toString() const {
      ostringstream f;
      f <<  "says( ";
      for (ExpressionList::const_iterator iter = _says->begin(); 
           iter != _says->end(); iter++) {
        f << (*iter)->toString();
        if (iter+1 != _says->end()) f << ", ";
      }
      f << " ) <" << Functor::toString() << "> ";
      return f.str();
    }


//     TermList* Functor::generateEqTerms(Functor* s){
//       assert(name().compare(s->name()) == 0);
      
//       ExpressionList::iterator iter1; 
//       ExpressionList::iterator iter2; 
//       ExpressionList *saysList1 =  const_cast<ExpressionList*>(arguments());
//       ExpressionList *saysList2 =  const_cast<ExpressionList*>(s->arguments());
      
//       //skip location specifier terms
//       return Secure::generateEqTerms(saysList1->begin() + 1, saysList1->end(), saysList2->begin() + 1, saysList2->end());
      
//     }


    TuplePtr 
    Functor::materialize(CommonTable::ManagerPtr catalog, ValuePtr ruleId, string ns, uint32_t pos)
    {
      CommonTablePtr funcTbl = catalog->table(FUNCTOR);
      TuplePtr     functorTp = Tuple::mk(FUNCTOR, true);
      if (ruleId) {
        functorTp->append(ruleId);
      }
      else {
        functorTp->append(Val_Null::mk());
      }
  
      string name;
      if (_name.size() >= 2 && _name.substr(0,2) == "::") {
        /* Referencing functor from some other namespace, 
           relative to the global namespace */
        name = _name.substr(2, _name.length()-2);
      }
      else {
        name = ns + _name;
      }
      functorTp->append(Val_Int64::mk(_complement)); // NOTIN?
      functorTp->append(Val_Str::mk(name));   // Functor name
  
      // Fill in table reference if functor is materialized
      CommonTable::Key nameKey;
      nameKey.push_back(catalog->attribute(FUNCTOR, "NAME"));
      CommonTablePtr tableTbl = catalog->table(TABLE);
      CommonTable::Iterator tIter = 
        tableTbl->lookup(nameKey, CommonTable::theKey(CommonTable::KEY3), functorTp);

      if (!tIter->done()) { 
        if (_complement && pos == 0) {
          throw compile::Exception("NOTIN does not apply to the head predicate."); 
        }
        TuplePtr table = tIter->next();
        functorTp->append((*table)[TUPLE_ID]);
      }
      else {
        if (_complement) {
          throw compile::Exception("NOTIN only applies to table predicates."); 
        }
        functorTp->append(Val_Null::mk());
      }
  
      functorTp->append(Val_Null::mk());          // The ECA flag
      functorTp->append(Val_Null::mk());          // The attributes field
      functorTp->append(Val_Int64::mk(pos));     // The position field
      functorTp->append(Val_Null::mk());          // The access method
      functorTp->append(Val_Int64::mk(_new?1:0)); // The new field
      funcTbl->insert(functorTp);                 // Add new functor to functor table

      // Now take care of the functor arguments and variable dependencies
      ListPtr attributes = List::mk();
      for (ExpressionList::iterator iter = _args->begin();
           iter != _args->end(); iter++) {
        attributes->append((*iter)->tuple());
      }
      functorTp->set(catalog->attribute(FUNCTOR, "ATTRIBUTES"), Val_List::mk(attributes));
      functorTp->freeze();
      funcTbl->insert(functorTp); // Update functor with dependency list

      // if new then also materialize new tuple
      if(_new){
	CommonTablePtr newTbl = catalog->table(NEW);
	TuplePtr newTp = Tuple::mk(NEW, true);
	newTp->append((*functorTp)[TUPLE_ID]); // add FID
	newTp->append(_locSpec->tuple());
	newTp->append(_opaque->tuple());
	newTp->append(_hint->tuple());
	newTp->freeze();
	newTbl->insert(newTp);
      }
      return functorTp;
    }
    
    TuplePtr 
    Says::materialize(CommonTable::ManagerPtr catalog, ValuePtr ruleId, string ns, uint32_t pos)
    {
      TuplePtr functorTp = Functor::materialize(catalog, ruleId, ns, pos);
      CommonTablePtr saysTbl = catalog->table(SAYS);
      TuplePtr saysTp = Tuple::mk(SAYS, true);
      saysTp->append((*functorTp)[TUPLE_ID]); // add FID

      // Now take care of the functor arguments and variable dependencies
      ListPtr attributes = List::mk();
      for (ExpressionList::iterator iter = _says->begin();
           iter != _says->end(); iter++) {
        attributes->append((*iter)->tuple());
      }
      saysTp->append(Val_List::mk(attributes));
      saysTp->freeze();
      saysTbl->insert(saysTp); // Update functor with dependency list
      return functorTp;
    }
        

    const Aggregation* 
    Functor::aggregate() const {
      for (ExpressionList::iterator iter = _args->begin(); 
           iter != _args->end(); iter++) {
        Aggregation* agg = dynamic_cast<Aggregation*>(*iter);
        if (agg != NULL) return agg;
      }
      return NULL;
    }
    
    Functor::Functor(Expression *n, ExpressionList *a, bool complement, bool newF)
      : _name(n->toString()), _args(a), _complement(complement)
    {
      Variable    *var;
      Aggregation *agg;
      _new = newF;
      bool locSpec   = false;
      bool aggregate = false;

      /** Ensure location sepecifier exists */
      for (ExpressionList::iterator iter = a->begin(); iter != a->end(); iter++) {
        if ((var = dynamic_cast<Variable*>(*iter)) != NULL && var->location()) {
          if (locSpec) {
            throw compile::Exception("Predicate " + n->toString() +
                                     " must define a unique predicate!");
          }
          locSpec = true;
        }
        else if ((agg = dynamic_cast<Aggregation*>(*iter)) != NULL) {
          if (agg->variable() != NULL && agg->variable()->location()) {
            locSpec = true;
          }

          if (aggregate) {
            throw compile::Exception("Only 1 aggregate can appear in a rule.");
          }
          aggregate = true;
        }
      }

      if (!locSpec) {
        throw compile::Exception("No location specifier in predicate " + n->toString() + ".");
      }
    }
    
    ExpressionList::iterator
    Functor::find(const Expression *e) const
    {
      ExpressionList::iterator iter; 
      for (iter = _args->begin(); 
           iter != _args->end(); iter++)
        if (e->toString() == (*iter)->toString()) return iter;
      return iter;
    }

    TotalIterator
    Functor::totalFind(const Expression *e) const
    {
      ExpressionList::iterator iter; 
      iter = find(e);
      if(iter != _args->end()){
	return TotalIterator(Functor::T_FUNCTOR, iter);
      }
      else{
	// check if new
	if(_new){
	  if (e->toString() == _locSpec->toString()) return TotalIterator(Functor::T_LOCSPEC, iter);
	  else if (e->toString() == _opaque->toString()) return TotalIterator(Functor::T_OPAQUE, iter);
	  else if (e->toString() == _hint->toString()) return TotalIterator(Functor::T_HINT, iter);
	}
	return TotalIterator(Functor::NOT_FOUND, iter);
      }
      
    }

    TotalIterator
    Says::totalFind(const Expression *e) const
    {
      TotalIterator iter; 
      iter = Functor::totalFind(e);
      if(iter.first != Functor::NOT_FOUND){
	return iter;
      }
      else{
	ExpressionList::iterator eiter; 
	for (eiter = _says->begin(); 
	     eiter != _says->end(); eiter++)
	  if (e->toString() == (*eiter)->toString()) return TotalIterator(Functor::T_SAYS, eiter);

	return TotalIterator(Functor::NOT_FOUND, eiter);
      }
    }
    
    // careful with its use as it doesn't free the memory of the replaced variable. 
    // caller should take care of it
    void Functor::replace(ExpressionList::iterator i, 
                          Expression *e) {
      ExpressionList::iterator next = _args->erase(i);
      _args->insert(next, e);
    }


    void Functor::replace(TotalIterator i, 
                          Expression *e) {
      if(i.first == Functor::T_FUNCTOR){
	ExpressionList::iterator next = _args->erase(i.second);
	_args->insert(next, e);
      }
      else if(i.first == Functor::T_LOCSPEC){
	_locSpec = e;
      }
      else if(i.first == Functor::T_OPAQUE){
	_opaque = e;
      }
      else if(i.first == Functor::T_HINT){
	_hint = e;
      }
      else{
	assert(0);
      }
    }

    void Says::replace(TotalIterator i, 
		       Expression *e) {
      if(i.first == Functor::T_SAYS){
	ExpressionList::iterator next = _says->erase(i.second);
	_says->insert(next, e);
      }
      else{
	Functor::replace(i, e);
      }
    }

    void Functor::changeLocSpec(Variable *l) {
      Variable    *var;
      
      if(l->location() && (var = dynamic_cast<Variable*>(_args->at(0))) != NULL && var->location()) {
	replace(_args->begin(), l);
	delete var;
      }
      else{
	throw compile::Exception("Location specifier is not the first field in functor" + toString() + 
				 "incorrect new location specifier " + l->toString());
      }
    }  

    Variable* Functor::getLocSpec() {
      Variable    *var;
      
      if((var = dynamic_cast<Variable*>(_args->at(0))) != NULL && var->location()) {
	return var;
      }
      else{
	throw compile::Exception("Location specifier is not the first field in functor" + toString());
	return NULL;
      }
    }  



    string Assign::toString() const {
      return _variable->toString() + " := " + _assign->toString();
    }
    
    TuplePtr 
    Assign::materialize(CommonTable::ManagerPtr catalog, ValuePtr ruleId, string ns, uint32_t pos)
    {
      CommonTablePtr assignTbl = catalog->table(ASSIGN);
      TuplePtr       assignTp  = Tuple::mk(ASSIGN, true);
      if (ruleId) {
        assignTp->append(ruleId);
      }
      else {
        assignTp->append(Val_Null::mk());
      }
      ValuePtr variable = _variable->tuple();
      ValuePtr value    = _assign->tuple();
  
      assignTp->append(variable);               // Assignment variable
      assignTp->append(value);                  // Assignemnt value
      assignTp->append(Val_Int64::mk(pos));         // Position
      assignTp->freeze();
      assignTbl->insert(assignTp); 
      return assignTp;
    }
    
    string Select::toString() const {
      return _select->toString();
    }
    
    TuplePtr 
    Select::materialize(CommonTable::ManagerPtr catalog, ValuePtr parentKey, string ns, uint32_t pos)
    {
      CommonTablePtr selectTbl = catalog->table(SELECT);
      TuplePtr       selectTp  = Tuple::mk(SELECT, true);
      if (parentKey) {
        selectTp->append(parentKey);   // Should be rule identifier
      }
      else {
        selectTp->append(Val_Null::mk());
      }
      ValuePtr boolExpr = _select->tuple();
     
      selectTp->append(boolExpr);              // Boolean expression
      selectTp->append(Val_Int64::mk(pos));        // Position
      selectTp->append(Val_Null::mk());        // Access method
      selectTp->freeze();
      selectTbl->insert(selectTp); 
      return selectTp;
    }
    
/***************************************************************************/

    Namespace::Namespace(string name, StatementList *s)
      : _name(name), _statements(s) { }

    TuplePtr
    Namespace::materialize(CommonTable::ManagerPtr catalog, 
                           ValuePtr programID, string ns)
    {
        for (StatementList::iterator iter = _statements->begin();
             iter != _statements->end(); iter++) { 
          compile::parse::Table *table = dynamic_cast<compile::parse::Table*>(*iter);
          if (table != NULL) {
            table->materialize(catalog, programID, ns + _name + "::");
          }
        }

        for (StatementList::iterator iter = _statements->begin();
             iter != _statements->end(); iter++) { 
          compile::parse::Index *index = dynamic_cast<compile::parse::Index*>(*iter);
          if (index != NULL) {
            index->materialize(catalog, programID, ns + _name + "::");
          }
        }

        for (StatementList::iterator iter = _statements->begin(); 
             iter != _statements->end(); iter++) {
          if (!dynamic_cast<compile::parse::Table*>(*iter) && !dynamic_cast<compile::parse::Index*>(*iter)) {
            (*iter)->materialize(catalog, programID, ns + _name + "::");
          }
        }
        return TuplePtr();
    }

    string 
    Namespace::toString() const
    {
      ostringstream oss;
      oss << "namespace " << _name << std::endl;
      for (StatementList::iterator iter = _statements->begin(); 
           iter != _statements->end(); iter++) {
        oss << (*iter)->toString() << "\n";
      }
      return oss.str();
    }


    Rule::Rule(Term *lhs, TermList *rhs, bool deleteFlag, Expression *n)
    {
    	if (n) {
    		_name = n->toString();
    	}
    	else {
    		ostringstream name;
    		name << "rule_" << ruleId++;
    		_name = name.str();
    	}
      _delete = deleteFlag;
      _new = false;
      _head = dynamic_cast<Functor*>(lhs);
      _head->resetHint(); // reset the hint var to null if its not null already
      _body = rhs;
    }
    
    Functor::Functor(TableEntry *t, int &fictVar){
      _complement = false;
      _name = t->name;
      _new = false;
      _args = new ExpressionList();
      for(int i = 0; i < t->fields; i++){
	ostringstream oss;
	oss << Says::varPrefix << fictVar++;
	Variable *tmp = new Variable(Val_Str::mk(oss.str()), i == 0);	
	_args->push_back(tmp);
      }
    }

    void Rule::canonicalizeRule() 
    {
      canonicalizeAttributes(_head, _body, true);
      for (TermList::iterator iter = _body->begin(); 
           iter != _body->end(); iter++)
      {
        Functor *pred;
        if ((pred = dynamic_cast<Functor*>(*iter)) != NULL)
        {
          canonicalizeAttributes(pred, _body, false);
        }
      }

    }

    void 
    Rule::canonicalizeAttributes(Functor *pred,      /* Predicate to canonicalize */
                                 TermList *ruleBody, /* List of Predicates making 
                                                        up rule body */
                                 bool headPred       /* Is 'pred' the head? */
                                 )
    {
      static int fict_varnum = 1; // Counter for inventing anonymous variables. 

      // We should not be canonicalize periodics
      if (pred->name() == "periodic" || headPred == false) {
        // Skip this one
        return;
      }

      ExpressionList *argsList[Functor::NOT_FOUND];
      ExpressionList locationList;
      ExpressionList opaqueList;
      ExpressionList hintList;
      ExpressionList saysList;

      if(pred->isNew()){
	locationList.push_back(pred->getNewLocSpec());
	opaqueList.push_back(pred->getOpaque());
	hintList.push_back(pred->getHint());
      }
      argsList[Functor::T_FUNCTOR] = const_cast<ExpressionList*>(pred->arguments());
      Says *s;
      if((s = dynamic_cast<Says*>(pred)) != NULL){
	argsList[Functor::T_SAYS] = const_cast<ExpressionList*>(s->saysParams());
      }
      else{
	argsList[Functor::T_SAYS] = &saysList;
      }

      argsList[Functor::T_LOCSPEC] = &locationList;
      argsList[Functor::T_OPAQUE] = &opaqueList;      
      argsList[Functor::T_HINT] = &hintList;      

      ExpressionList *args;
      // Next, we canonicalize all the args in the functor.  We build up
      // a list of argument names - the first 'arity' of these will be the
      // free variables in the rule head.  Literals and duplicate free
      // variables here are eliminated, by a process of appending extra
      // "eq" terms to the body, and inventing new free variables.
      for(int count = 0; count < Functor::NOT_FOUND; count++){
	args = argsList[count];
	for (ExpressionList::iterator i = args->begin();
	     i != args->end(); i++) {
	  Variable *var = NULL;
	  Value    *val = NULL;
	  Math     *math = NULL;
	  Function *func = NULL;
    
	  if ((var = dynamic_cast<Variable*>(*i)) != NULL) {
	    // The argument is a free variable - the usual case. 
	    TotalIterator prev = pred->totalFind(var);
	    if (prev.first < count || ((prev.first == count) && (prev.second < i))) {
	      if((prev.first != count) || ((count != Functor::T_LOCSPEC) && (count != Functor::T_OPAQUE) && (count != Functor::T_HINT))){
		ostringstream oss;
		oss << Context::STAGEVARPREFIX << fict_varnum++;
		// We've found a duplicate variable in the head. Add a new
		// "eq" term to the front of the term list. 
		Variable *tmp = new Variable(Val_Str::mk(oss.str()));
		(headPred) ? ruleBody->push_back(new Assign(tmp, *i))
		  : ruleBody->push_back(new Select(new Bool(Bool::EQ, tmp, *i)));
		pred->replace(TotalIterator(count, i), tmp);
	      }
	    }
	  }
	  else if ((val = dynamic_cast<Value*>(*i)) != NULL) {
	    ostringstream oss;
	    oss << Context::STAGEVARPREFIX << fict_varnum++;
	    Variable *tmp = new Variable(Val_Str::mk(oss.str()));
	    pred->replace(TotalIterator(count, i), tmp);
	    (headPred) ? ruleBody->push_back(new Assign(tmp, val))
	      : ruleBody->push_back(new Select(new Bool(Bool::EQ, tmp, val)));
	  }
	  else if ((math = dynamic_cast<Math*>(*i)) != NULL) {
	    ostringstream oss;
	    oss << Context::STAGEVARPREFIX << fict_varnum++;
	    Variable *tmp = new Variable(Val_Str::mk(oss.str()));
	    pred->replace(TotalIterator(count, i), tmp);
	    (headPred) ? ruleBody->push_back(new Assign(tmp, math))
	      : ruleBody->push_back(new Select(new Bool(Bool::EQ, tmp, math)));
	  }
	  else if ((func = dynamic_cast<Function*>(*i)) != NULL) {
	    ostringstream oss;
	    oss << Context::STAGEVARPREFIX << fict_varnum++;
	    Variable *tmp = new Variable(Val_Str::mk(oss.str()));
	    pred->replace(TotalIterator(count, i), tmp);
	    (headPred) ? ruleBody->push_back(new Assign(tmp, func))
	      : ruleBody->push_back(new Select(new Bool(Bool::EQ, tmp, func)));
	  }
	  else if (dynamic_cast<Aggregation*>(*i) == NULL) {
	    throw Exception(0, "Parse rule unknown functor body type.");
	  }
	}
      }

    } 
    
    string Rule::toString() const
    {
      ostringstream b;
      b << "Rule " << _name << ": ";
      if (_delete) b << "delete ";
    
      b << _head->toString() << " :- ";
      for (TermList::const_iterator iter = _body->begin();
           iter != _body->end(); iter++) {
        b << (*iter)->toString();
        if (iter + 1 != _body->end()) b << ", ";
      }
      b << ".";
      return b.str();  
    }
    
    TuplePtr 
    Rule::materialize(CommonTable::ManagerPtr catalog, ValuePtr programID, string ns)
    {
      CommonTablePtr ruleTbl = catalog->table(RULE);
      uint32_t pos = 0;    
      /* First create a tuple represening the rule and add it to the rule
       * table. */
      TuplePtr ruleTp = Tuple::mk(RULE, true);
      if (programID) {
        ruleTp->append(programID);
      }
      else {
        ruleTp->append(Val_Null::mk());
      }
      ruleTp->append(Val_Str::mk(_name));
      TuplePtr headTp = _head->materialize(catalog, (*ruleTp)[TUPLE_ID], ns, pos++);
      ruleTp->append((*headTp)[TUPLE_ID]);             // Add the "head" functor identifer.
      ruleTp->append(Val_Null::mk());                  // The P2DL desc. of this rule
      ruleTp->append(Val_Int64::mk(_delete));         // Delete rule?
      ruleTp->append(Val_Int64::mk(_body->size()+1)); // Term count?
      ruleTp->append(Val_Int64::mk(_new?1:0)); // The access method
      ruleTp->freeze();
      ruleTbl->insert(ruleTp);	               // Add rule to rule table.
    
      /* Create the functors associated with the rule body. Each functor row
       * will reference (foreign key) the rule identifier of the rule 
       * created above. */

      for (TermList::const_iterator iter = _body->begin();
           iter != _body->end(); iter++, pos++) {
        (*iter)->materialize(catalog, (*ruleTp)[TUPLE_ID], ns, pos);
      }
      return ruleTp;
    }

    Watch::Watch(string w, string modifiers) 
      : _watch(w), _modifiers(modifiers) { }

    TuplePtr 
    Watch::materialize(CommonTable::ManagerPtr catalog, ValuePtr programId, string ns)
    {
      CommonTablePtr watchTbl = catalog->table(WATCH);
      CommonTable::Key key;
      key.push_back(catalog->attribute(WATCH, "NAME"));
      key.push_back(catalog->attribute(WATCH, "MOD"));

      int mod = 0;
      do {
          TuplePtr watchTp = Tuple::mk(WATCH, true);
          if (programId) {
              watchTp->append(programId);
          }

          string name;
          if (_watch.size() >= 2 && _watch.substr(0,2) == "::") {
              name = _watch.substr(2, _watch.length()-2);
          }
          else {
              name = ns + _watch; 
          }
  
          watchTp->append(Val_Str::mk(name)); // Watch name
          if (_modifiers == "") 
              watchTp->append(Val_Str::mk("")); // Watch modifier
          else
              watchTp->append(Val_Str::mk(string(_modifiers,mod,1))); // Watch name

          // Fill in table reference if functor is materialized
          CommonTable::Iterator Iter = watchTbl->lookup(key, key, watchTp);
          if (Iter->done()) { 
              watchTp->freeze();
              watchTbl->insert(watchTp); 
          }
      } while (++mod < (int)_modifiers.size());
      return TuplePtr();
    }

    Stage::Stage(string p, string i, string o)
      : _processor(p), _input(i), _output(o)
    {
    }
    

    TuplePtr
    Stage::materialize(CommonTable::ManagerPtr catalog,
                       ValuePtr programID, string ns)
    {
      string input;
      string output;
      if (_input.size() >= 2 && _input.substr(0,2) == "::") {
        input = _input.substr(2, _input.length()-2);
      }
      else {
        input = ns + _input; 
      }

      if (_output.size() >= 2 && _output.substr(0,2) == "::") {
        output = _output.substr(2, _output.length()-2);
      }
      else {
        output = ns + _output; 
      }

      TuplePtr tuple = Tuple::mk(STAGE, true);
      tuple->append(programID);
      tuple->append(Val_Str::mk(_processor));
      tuple->append(Val_Str::mk(input));
      tuple->append(Val_Str::mk(output));
      tuple->freeze();

      catalog->table(STAGE)->insert(tuple);
      return tuple;
    }


    Fact::Fact(Expression *n, ExpressionList *a)
    {
      TuplePtr tpl = Tuple::mk();
      tpl->append(Val_Str::mk(n->toString()));

      // The fields
      uint32_t pos = 1;
      ExpressionList::const_iterator iter;

      for (iter = a->begin(); iter != a->end(); iter++, pos++) {
	if(pos == compile::VERPOS && Table::compoundRewrite){
	  tpl->append(Val_Int64::mk(0));
	}
        Value *v = dynamic_cast<Value*>(*iter);
        Math  *m = dynamic_cast<Math*>(*iter);
        if (v != NULL) {
          tpl->append(v->value());
        } 
        else if (m != NULL) {
          tpl->append(m->value());
        }
        else {
          throw compile::Exception(
            "Free variables and don't-cares not allowed in facts:" 
            + (*iter)->toString());
        }
      }
      tpl->freeze();
      _tuple = tpl;
    } 
    
    string Fact::toString() const
    {
      return _tuple ? "Fact ( " + _tuple->toString() + " )." : "Empty fact";
    }
    
    TuplePtr 
    Fact::materialize(CommonTable::ManagerPtr catalog, ValuePtr parentKey, string ns)
    {
      if (!_tuple) {
        throw compile::Exception("Tuple does not exist for fact");
      }

      string name = (*_tuple)[TNAME]->toString();
      if (name.size() >= 2 && name.substr(0,2) == "::") {
        name = name.substr(2, name.length()-2);
      }
      else {
        name = ns + name; 
      }

      TuplePtr tuple = _tuple->clone();
      tuple->set(TNAME, Val_Str::mk(name)); // Set the scoped name
      tuple->freeze();

      // Update the the fact table to contain this fact as being installed
      CommonTablePtr factTbl = catalog->table(FACT);
      TuplePtr       factTp  = Tuple::mk(FACT, true);
      if (parentKey)   {
        factTp->append(parentKey  );
      }
      factTp->append(Val_Str::mk(name));    // Foreign key to table is table name
      factTp->append(Val_Tuple::mk(tuple)); // Fact tuple value
      factTp->freeze();
      factTbl->insert(factTp); 
      return factTp;
    }
    
    Index::Index(Expression *name, Expression *type, ExpressionList *keys)
    {
      _name = name->toString() ;
      _type = type->toString() ;

      uint32_t fieldNum = 0;
      ExpressionList::const_iterator i = keys->begin();
      for (; (i != keys->end()) && (fieldNum <= 1); i++){
        fieldNum= Val_Int64::cast((*i)->value());
        _keys.push_back(fieldNum);
      }       
    }

    string 
    Index::toString() const 
    {
      ostringstream b;
    
      b << "Index: " << _name << "( ";
      for (Table2::Key::const_iterator iter = _keys.begin();
           iter != _keys.end(); iter++) {
        b << *iter;
        if (iter + 1 != _keys.end()) b << ", ";
      }
      b << " ).";
      return b.str();
    }
    
    TuplePtr 
    Index::materialize(CommonTable::ManagerPtr catalog, ValuePtr parentKey, string ns)
    {
      string scopedName;
      if (_name.size() >= 2 && _name.substr(0,2) == "::") {
        scopedName = _name.substr(2, _name.length()-2);
      }
      else {
        scopedName = ns + _name; 
      }

      return catalog->createIndex(scopedName, _type, _keys);
    }

    Table::Table(Expression *name, 
                 Expression *ttl, 
                 Expression *size, 
                 ExpressionList *keys,
		 bool says,
		 bool versioned)
    {
      _name = name->toString() ;
      _says = says;
      _versioned = versioned;

      if(_says){
	materializedSaysTables->insert(Val_Str::mk(_name));
	initialize();
      }

      int myTtl = Val_Int64::cast(ttl->value());
      if (myTtl == -1) {
        _lifetime = CommonTable::NO_EXPIRATION;
      } else if (myTtl == 0) {
        // error("bad timeout for materialized table");
      } else {
        _lifetime = boost::posix_time::seconds(myTtl);
      }
    
    
      _size = Val_Int64::cast(size->value());
      // Hack because infinity token has a -1 value
      if (_size == -1) {
        _size = CommonTable::NO_SIZE;
      }
    
      if (keys) {
	uint32_t fieldNum = 0;
	ExpressionList::const_iterator i = keys->begin();
	ExpressionList::const_iterator i1 = keys->begin();
	for (; (i != keys->end()) && (fieldNum <= 1); i++){
	  fieldNum= Val_Int64::cast((*i)->value());
	  if(fieldNum <= 1){
	      _keys.push_back(fieldNum);
	      i1++;
	  }
	}       
	uint32_t offset = 1; // how much to offset
	if(compoundRewrite && _versioned){
	  _keys.push_back(compile::VERPOS); 
	  ++offset;
	  assert(compile::VERPOS == offset);
	}
	if(says){
	  _keys.push_back(++offset); //P
	  _keys.push_back(++offset); //R
	  _keys.push_back(++offset); //K
	  _keys.push_back(++offset); //V
	  _keys.push_back(++offset); //Proof
	}
        for (;i1 != keys->end(); i1++){
	  fieldNum= Val_Int64::cast((*i1)->value());
	  if(fieldNum > 1){
	    fieldNum += (offset -1);	  
	  }
	  _keys.push_back(fieldNum);
	}
      }
    }
    
    string 
    Table::toString() const 
    {
      ostringstream b;
    
      b << "Table: " << _name << " ttl = " << _lifetime
        << ", size " << _size << ", keys (";
      for (Table2::Key::const_iterator iter = _keys.begin();
           iter != _keys.end(); iter++) {
        b << *iter;
        if (iter + 1 != _keys.end()) b << ", ";
      }
      b << ").";
      return b.str();
    }
    
    void 
    Table::initialize(){
      if(_says)
      {
	_name =  _name + Says::saysSuffix;
      }
    }

    TuplePtr 
    Table::materialize(CommonTable::ManagerPtr catalog, ValuePtr parentKey, string ns)
    {
      string scopedName;
      if (_name.size() >= 2 && _name.substr(0,2) == "::") {
        scopedName = _name.substr(2, _name.length()-2);
      }
      else {
        scopedName = ns + _name; 
      }
      if(compoundRewrite && _versioned){
	compile::Context::materializedTables->insert(Val_Str::mk(scopedName));
      }      
      if (_lifetime == CommonTable::NO_EXPIRATION && _size == CommonTable::NO_SIZE) {
        return RefTable::mk(*catalog, scopedName, _lifetime, _size, _keys, ListPtr(), parentKey);
      }
      return Table2::mk(*catalog,scopedName,_lifetime, _size, _keys, ListPtr(), parentKey);
    }

    Ref::Ref(int refType, 
	     Expression *from, 
	     Expression *to, 
	     Expression *locSpecField){
      _from = from->toString() ;
      _to = to->toString() ;
      _locSpecField = Val_Int64::cast(locSpecField->value());
      _refType = refType;

    }

    string 
    Ref::toString() const 
    {
      ostringstream b;
      b << ((_refType == Ref::STRONG)?"Strong ":"Weak ") << "Ref( " << _locSpecField << ", " << _from
        << ", " << _to << ").";
      return b.str();
    }

    TuplePtr 
    Ref::materialize(CommonTable::ManagerPtr catalog, ValuePtr parentKey, string ns)
    {
      string scopedFrom;
      string scopedTo;
      if (_from.size() >= 2 && _from.substr(0,2) == "::") {
        scopedFrom = _from.substr(2, _from.length()-2);
      }
      else {
        scopedFrom = ns + _from; 
      }

      if (_to.size() >= 2 && _to.substr(0,2) == "::") {
        scopedTo = _to.substr(2, _to.length()-2);
      }
      else {
        scopedTo = ns + _to; 
      }

      TuplePtr tpl = Tuple::mk(REF, true);
      
      tpl->append(parentKey);      
      tpl->append(Val_Str::mk(scopedFrom));
      tpl->append(Val_Str::mk(scopedTo));
      tpl->append(Val_Int64::mk(_locSpecField));
      tpl->append(Val_Int64::mk(_refType));
      tpl->freeze();

      // Update the the ref table to contain this fact as being installed
      CommonTablePtr refTbl = catalog->table(REF);

      refTbl->insert(tpl); 
      return tpl;
    }

    /**********************************************************************
     *
     * parse::Context methods
     *
     *********************************************************************/
    DEFINE_ELEMENT_INITS_NS(Context, "ParseContext", compile::parse)

    Context::~Context()
    {
    }
    
    Context::Context(string name, string prog, bool file)
      : compile::Context(name), lexer(NULL), _statements(NULL)
    {
      printOverLog = false;
      std::istream* pstream;

      if (file) {
        string processed(prog+".processed");
      
        // Run the OverLog through the preprocessor
        pid_t pid = fork();
        if (pid == -1) {
          std::cerr << "Cannot fork a preprocessor\n";
          exit(-1);
        } else if (pid == 0) {
          // I am the preprocessor
          execlp("cpp", "cpp", "-P", prog.c_str(), processed.c_str(),
                 (char*) NULL);
          // If I'm here, I failed
          std::cerr << "Preprocessor execution failed" << std::endl;;
          exit(-1);
        } else {
          // I am the child
          wait(NULL);
        }
      
        // Parse the preprocessed file
        pstream = new std::ifstream(processed.c_str());
        unlink(processed.c_str());
      }
      else {
        pstream = new std::istringstream(prog, std::istringstream::in);
      }
      
      CommonTablePtr programTbl = Plumber::catalog()->table(PROGRAM);
      TuplePtr       programTp  = Tuple::mk(PROGRAM, true);
      programTp->append(Val_Str::mk("main"));           // Program name
      programTp->append(Val_Null::mk());                // Rewrite predecessor
      programTp->append(Val_Str::mk(name));             // Program status
      programTp->append(Val_Str::mk(prog));             // Program text/filename
      programTp->append(Val_Null::mk());                // Result
      programTp->append(Val_Null::mk());                // P2DL text
      programTp->append(Plumber::catalog()->nodeid());  // Source address 
      programTp->freeze();
      
      parse(Plumber::catalog(), (*programTp)[TUPLE_ID], pstream);
      programTbl->insert(programTp);           // Commit program tuple
    }
    
    Context::Context(TuplePtr args)
      : compile::Context((*args)[2]->toString()), 
        lexer(NULL), _statements(NULL)
    {
      printOverLog = false;
      if (args->size() > 3) {
        CommonTablePtr programTbl = Plumber::catalog()->table(PROGRAM);
        TuplePtr       programTp  = Tuple::mk(PROGRAM, true);
        programTp->append(Val_Str::mk("main"));                 // Program name
        programTp->append(Val_Null::mk());                      // Rewrite predecessor
        programTp->append(Val_Str::mk((*args)[2]->toString())); // Program status
        programTp->append(Val_Str::mk((*args)[3]->toString())); // Program text/filename
        programTp->append(Val_Null::mk());                      // Result
        programTp->append(Val_Null::mk());                      // P2DL text
        programTp->append(Plumber::catalog()->nodeid());        // Source address 
        programTp->freeze();
      
        std::istringstream overlog(Val_Str::cast((*args)[3]), std::istringstream::in);
        parse(Plumber::catalog(), (*programTp)[TUPLE_ID], &overlog);
        programTbl->insert(programTp);           // Commit program tuple
      }
    }
    
    void 
    Context::program(StatementList *s, bool parserCall) 
    {
      assert(_statements == NULL || !parserCall);
      if(parserCall)
      {
	_statements = s;
	//	compile::parse::Secure::program(_statements, true);
      }

      Rule* r;
      Namespace *nmSpc;
      Table *tab;
      SetPtr materializedSaysTables(new Set());
      for (StatementList::iterator iter = s->begin();
             iter != s->end(); iter++) { 

	if ((r = dynamic_cast<Rule*>(*iter)) != NULL) 
	{
	  r->canonicalizeRule();
	}
	else if((nmSpc = dynamic_cast<Namespace*>(*iter)) != NULL) 
	{
	  if(nmSpc->statements() != NULL)
	    program(nmSpc->statements(), false);
	}
	else if((tab = dynamic_cast<Table*>(*iter)) != NULL)
	{
	  //	  tab->initialize();
	}
	else
	{
	  // do nothing
	}

      }
    }

    TuplePtr 
    Context::program(CommonTable::ManagerPtr catalog, TuplePtr program)
    {
      CommonTablePtr programTbl = Plumber::catalog()->table(PROGRAM);
      string ptext = (*program)[catalog->attribute(PROGRAM, "TEXT")]->toString();
      std::istringstream overlog(ptext, std::istringstream::in);
      parse(catalog, (*program)[TUPLE_ID], &overlog); 
      program = program->clone(PROGRAM_STREAM);
      program->freeze();
      return program;
    }
  
    void
    Context::parse(CommonTable::ManagerPtr catalog, ValuePtr pid, std::istream *prog)
    {
      parse_stream(prog);
  
      if (_statements) {
        for (StatementList::iterator iter = _statements->begin();
             iter != _statements->end(); iter++) { 
          compile::parse::Table *table = dynamic_cast<compile::parse::Table*>(*iter);
          if (table != NULL) {
            table->materialize(catalog, pid, ""); 
          }
        }

        for (StatementList::iterator iter = _statements->begin();
             iter != _statements->end(); iter++) { 
          compile::parse::Index *index = dynamic_cast<compile::parse::Index*>(*iter);
          if (index != NULL) {
            index->materialize(catalog, pid, ""); 
          }
        }

        for (StatementList::iterator iter = _statements->begin();
             iter != _statements->end(); iter++) { 
          if (!dynamic_cast<compile::parse::Table*>(*iter) && !dynamic_cast<compile::parse::Index*>(*iter)) {
            (*iter)->materialize(catalog, pid, ""); 
          }
        }

        delete _statements;
        _statements = NULL;
      }
    }
    
    //
    // Print out the whole parse result, if we can
    //
    string 
    Context::toString() const
    {
      return "";
    }
    
    void 
    Context::parse_stream(std::istream *str)
    {
      try {
        assert(lexer==NULL);
        lexer = new OLG_Lexer(str);
        olg_parser_parse(this);
        delete lexer; 
        lexer = NULL;
      } 
      catch (compile::Exception e) {
        delete lexer; 
        lexer = NULL;
        throw e;
      }
    }
    
    void 
    Context::error(string msg)
    {
      throw compile::parse::Exception(lexer->line_num(), msg);
    }
  }
}


