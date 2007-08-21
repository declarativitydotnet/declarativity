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
#include "ol_lexer.h"
#include "tuple.h"

#include "systemTable.h"
#include "val_tuple.h"
#include "val_vector.h"
#include "val_matrix.h"
#include "val_list.h"
#include "val_uint32.h"
#include "val_int32.h"
#include "val_uint64.h"
#include "val_int64.h"
#include "val_str.h"
#include "val_time.h"
#include "oper.h"

namespace compile {
  namespace parse {
    using namespace opr;
    
    ValuePtr 
    Value::tuple() const
    {
      TuplePtr tp = Tuple::mk(VAL);
      tp->append(_value);
      return Val_Tuple::mk(tp);
    }
  
    ValuePtr 
    Variable::tuple() const
    {
      TuplePtr tp = Tuple::mk((_location ? LOC : VAR));
      tp->append(_value);
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
      return Val_Tuple::mk(tp);
    }
    
    const string Function::max = "f_max";
    const string Function::mod = "f_mod";
    const string Says::verTable = "verKey"; 
    const string Says::genTable = "genKey"; 
    const string Says::hashFunc = "f_sha1"; 
    const string Says::verFunc = "f_verify"; 
    const string Says::genFunc = "f_gen"; 
    const string Says::encHint = "encHint"; 
    const string Says::varPrefix = "_"; 
    const string Says::saysPrefix = "says"; 
    const string Says::makeSays = "makeSays"; 
    const string Says::globalScope = "::"; 
    const int TableEntry::numSecureFields = 5;
    
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
    
      return Val_Tuple::mk(tp);
    }
    
    string 
    Range::toString() const {
      return _lhs->toString() + " " + _operName + " " + _rhs->toString();
    }
    
    ValuePtr
    Range::tuple() const 
    {
      TuplePtr tp = Tuple::mk(RANGE);
    
      tp->append(Val_Str::mk(_operName));
      tp->append(_lhs->tuple());
      tp->append(_rhs->tuple());
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
      tp->append(Val_UInt32::mk(_args->size()));
      for (ExpressionList::iterator iter = _args->begin(); 
           iter != _args->end(); iter++) {
        tp->append((*iter)->tuple());
      }
  
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
      TuplePtr tp = Tuple::mk(SET);
  
      tp->append(Val_Str::mk(SET));
      tp->append(Val_UInt32::mk(_args->size()));

      for (ExpressionList::iterator iter = _args->begin(); 
           iter != _args->end(); iter++) {
        tp->append((*iter)->tuple());
      }
  
      return Val_Tuple::mk(tp);
    }
    
    Vector::Vector(ExpressionList *v) {
      VectorPtr vp(new vector<ValuePtr>(v->size()));
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
    
      MatrixPtr mp(new matrix<ValuePtr>(m->size(), columnArity));
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
      return Val_Tuple::mk(tp);
    }
    
    
    /***************************************************
     * TERMS
     ***************************************************/
    
    string 
    Functor::toString() const {
      ostringstream f;
      f << (_complement?"!":"") <<name()<<  (_complement?"^":"") << "( ";
      for (ExpressionList::const_iterator iter = _args->begin(); 
           iter != _args->end(); iter++) {
        f << (*iter)->toString();
        if (iter+1 != _args->end()) f << ", ";
      }
      f << " )";
      return f.str();
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

    TuplePtr 
    Functor::materialize(CommonTable::ManagerPtr catalog, ValuePtr ruleId, string ns)
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
      functorTp->append(Val_Str::mk(name));   // Functor name
  
      // Fill in table reference if functor is materialized
      CommonTable::Key nameKey;
      nameKey.push_back(catalog->attribute(FUNCTOR, "NAME"));
      CommonTablePtr tableTbl = catalog->table(TABLE);
      CommonTable::Iterator tIter = 
        tableTbl->lookup(nameKey, CommonTable::theKey(CommonTable::KEY3), functorTp);
      if (!tIter->done()) 
        functorTp->append((*tIter->next())[TUPLE_ID]);
      else {
        functorTp->append(Val_Null::mk());
      }
  
      functorTp->append(Val_Null::mk());          // The ECA flag
      functorTp->append(Val_Null::mk());          // The attributes field
      functorTp->append(Val_Null::mk());          // The position field
      functorTp->append(Val_Null::mk());          // The access method
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
    
    Functor::Functor(Expression *n, ExpressionList *a, bool complement, bool event)
      : _name(n->toString()), _args(a), _complement(complement), _event(event)
    {
      Variable    *var;
      Aggregation *agg;
  
      /** Ensure location sepecifier is first */
      for (ExpressionList::iterator iter = a->begin(); iter != a->end(); iter++) {
        if ((var = dynamic_cast<Variable*>(a->at(0))) != NULL && var->location()) {
          a->erase(iter);
          a->push_front(var);
          break;
        }
        else if ((agg = dynamic_cast<Aggregation*>(a->at(0))) != NULL &&
               !agg->variable()->location()) {
          a->erase(iter);
          a->push_front(agg);
          break;
        }
      }
  
      if ((var = dynamic_cast<Variable*>(a->at(0))) != NULL &&
          !var->location()) {
        throw compile::Exception("Invalid location specifier. Variable: " 
                                        + var->toString());
      }
      else if ((agg = dynamic_cast<Aggregation*>(a->at(0))) != NULL &&
               !agg->variable()->location()) {
        throw compile::Exception("Invalid location specifier. Aggregate: " 
                                        + agg->toString());
      }
      else if (var == NULL && agg == NULL) {
        throw compile::Exception("Invalid location specifier. Not variable or aggregate: "
                                         + a->at(0)->toString());
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
    
    // careful with its use as it doesn't free the memory of the replaced variable. 
    // caller should take care of it
    void Functor::replace(ExpressionList::iterator i, 
                          Expression *e) {
      ExpressionList::iterator next = _args->erase(i);
      _args->insert(next, e);
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

    // return a list of terms that needs to be added to the rule on converting the 
    // securelog term f into overlog.
    // Also converts f into the appropriate overlog form
    TermList* Says::normalizeGenerate(Functor* f, int& newVariable)
    {
      Says *s;
      if ((s = dynamic_cast<Says*>(f)) != NULL)
      {
	
	TermList *newTerms = new TermList();

	// create encryption hint
	ExpressionList *t = new ExpressionList();
	t->push_back(Variable::getLocalLocationSpecifier());
	ExpressionList::iterator iter; 
	ExpressionList *saysList =  const_cast<ExpressionList*>(s->saysParams());
	//generate TabelEntry::numSecureFields -1 new terms for P, R, k, V on rhs
	ExpressionList *rhsSaysParams = new ExpressionList();
	for(int i = 0; i< TableEntry::numSecureFields -1; i++)
	{
	  ostringstream v;
	  v << Says::varPrefix << newVariable++;
	  rhsSaysParams->push_back(new Variable(Val_Str::mk(v.str())));
	}

	// finally extract the appropriate proof using the rhsSaysParams
	for (iter = rhsSaysParams->begin(); 
	     iter != rhsSaysParams->end(); iter++){
	  t->push_back(*iter);
	}
	ostringstream var1;
	var1 << Says::varPrefix << newVariable++;
	Variable *encId = new Variable(Val_Str::mk(var1.str()));
	t->push_back(encId);
	Functor *genHint = new Functor(new Value(Val_Str::mk(Says::globalScope + Says::encHint)), t);

	//create key generation table
	ExpressionList *genT = new ExpressionList();
	genT->push_back(Variable::getLocalLocationSpecifier());
	genT->push_back(encId);

	ostringstream var2;
	var2 << Says::varPrefix << newVariable++;
	Variable *genKey = new Variable(Val_Str::mk(var2.str()));
	genT->push_back(genKey);
	Functor *genTable = new Functor(new Value(Val_Str::mk(Says::globalScope + Says::genTable)), genT);

	//create buffer creation logic
	ostringstream var3;
	var3 << Says::varPrefix << newVariable++;
	Variable *buf = new Variable(Val_Str::mk(var3.str()));

	// check if the concatenate operation is needed or not
	// remember that the loc specifier is excluded but the table name must be included
	Value *table = new Value(Val_Str::mk(s->_name));
	Expression *cur = table;

	// since loc spec is not included
	if(s->_args->size() > 1)
	{
	  ExpressionList *list = s->saysArgs();
	  iter = list->begin();
	  //exclude location specifier
	  iter++; 
	  for (; iter != list->end(); iter++){
	    cur = new Math(Math::APPEND, cur, *iter);
	  }
	}

	Assign *assBuf = new Assign(buf, cur);

	//create generation logic
	ostringstream var4;
	var4 << Says::varPrefix << newVariable++;
	Variable *hash = new Variable(Val_Str::mk(var4.str()));
	ostringstream var5;
	var5 << Says::varPrefix << newVariable++;
	Variable *proof = new Variable(Val_Str::mk(var5.str()));

	ExpressionList *hashArg = new ExpressionList();
	hashArg->push_back(buf);
	Function *f_hash = new Function(new Value(Val_Str::mk(Says::hashFunc)), hashArg);
	Assign *assHash = new Assign(hash, f_hash);


	ExpressionList *genArg = new ExpressionList();
	genArg->push_back(hash);
	genArg->push_back(genKey);
	Function *f_gen = new Function(new Value(Val_Str::mk(Says::genFunc)), genArg);
	Assign *assGen = new Assign(proof, f_gen);


	newTerms->push_back(genHint);
	newTerms->push_back(genTable);
	newTerms->push_back(assBuf);
	newTerms->push_back(assHash);
	newTerms->push_back(assGen);

	// add constraints between saysParams and rhsSaysParams
	Says::generateAlgebraLT(saysList->begin(), 
				rhsSaysParams->begin(), newTerms);

	
	// now modify the says tuple

	ExpressionList *arg = s->saysArgs();
	for (iter = saysList->begin(); 
	     iter != saysList->end(); iter++){
	  arg->push_back(*iter);
	}
	arg->push_back(proof);
	
	s->changeName(Says::makeSays + s->name());

	// finally return the new terms
	return newTerms;
      }
      return NULL;
    }


    // return a list of terms that needs to be added to the rule on converting the 
    // securelog term f into overlog.
    // Also converts f into the appropriate overlog form
    // the last boolean indicates if the list of terms should also include the 
    // constraint that "the verifying principal doesn't have the authority to 
    // generate the proof"
    void Says::normalizeVerify(Functor* f, int& newVariable)
    {
      Says *s;
      //      TermList *newTerms = NULL;
      if ((s = dynamic_cast<Says*>(f)) != NULL)
      {
	// now modify the says tuple
	ExpressionList::iterator iter;
	ExpressionList *arg = s->saysArgs();
	ExpressionList *saysList =  const_cast<ExpressionList*>(s->saysParams());
	
	// if(head != NULL){
// 	  ExpressionList *headList =  const_cast<ExpressionList*>(head->saysParams());
// 	  newTerms = Says::generateAlgebraLT(headList->begin(), saysList->begin());
// 	}

	for (iter = saysList->begin(); 
	     iter != saysList->end(); iter++){
	  arg->push_back(*iter);
	}

 	ostringstream var5;
 	var5 << Says::varPrefix << newVariable++;
 	Variable *proof = new Variable(Val_Str::mk(var5.str()));
	arg->push_back(proof);
	
	s->changeName(Says::saysPrefix + s->name());

      }
      //      return newTerms;
    }

//     TermList* Says::normalizeVerify(Functor* f, int& newVariable, bool addNoKeyConstraint)
//     {
//       Says *s;
//       if ((s = dynamic_cast<Says*>(f)) != NULL)
//       {
	
// 	TermList *newTerms = new TermList();

// 	// create encryption hint
// 	ExpressionList *t = new ExpressionList();
// 	t->push_back(s->getLocSpec());
// 	ExpressionList::iterator iter; 
// 	ExpressionList *saysList =  const_cast<ExpressionList*>(s->saysParams());
// 	for (iter = saysList->begin(); 
// 	     iter != saysList->end(); iter++){
// 	  t->push_back(*iter);
// 	}
// 	ostringstream var1;
// 	var1 << Says::varPrefix << newVariable++;
// 	Variable *encId = new Variable(Val_Str::mk(var1.str()));
// 	t->push_back(encId);
// 	Functor *encHint = new Functor(new Value(Val_Str::mk(Says::globalScope + Says::encHint)), t);

// 	//create verification table
// 	ExpressionList *verT = new ExpressionList();
// 	verT->push_back(s->getLocSpec());
// 	verT->push_back(encId);

// 	ostringstream var2;
// 	var2 << Says::varPrefix << newVariable++;
// 	Variable *verKey = new Variable(Val_Str::mk(var2.str()));
// 	verT->push_back(verKey);
// 	Functor *verTable = new Functor(new Value(Val_Str::mk(Says::globalScope + Says::verTable)), verT);

// 	//create buffer creation logic
// 	ostringstream var3;
// 	var3 << Says::varPrefix << newVariable++;
// 	Variable *buf = new Variable(Val_Str::mk(var3.str()));

// 	// check if the concatenate operation is needed or not
// 	// remember that the loc specifier is excluded but the table name must be included
// 	Value *table = new Value(Val_Str::mk(s->_name));
// 	Expression *cur = table;

// 	if(s->_args->size() > 1)
// 	{
// 	  ExpressionList *list = s->saysArgs();
// 	  iter = list->begin();
// 	  //exclude location specifier
// 	  iter++; 
// 	  for (; iter != list->end(); iter++){
// 	    cur = new Math(Math::BITOR, cur, *iter);
// 	  }
// 	}

// 	Assign *assBuf = new Assign(buf, cur);

// 	//create verification logic
// 	ostringstream var4;
// 	var4 << Says::varPrefix << newVariable++;
// 	Variable *hash = new Variable(Val_Str::mk(var4.str()));
// 	ostringstream var5;
// 	var5 << Says::varPrefix << newVariable++;
// 	Variable *proof = new Variable(Val_Str::mk(var5.str()));

// 	ExpressionList *hashArg = new ExpressionList();
// 	hashArg->push_back(buf);
// 	Function *f_hash = new Function(new Value(Val_Str::mk(Says::hashFunc)), hashArg);
// 	Assign *assHash = new Assign(hash, f_hash);


// 	ExpressionList *verArg = new ExpressionList();
// 	verArg->push_back(proof);
// 	verArg->push_back(verKey);
// 	Function *f_verify = new Function(new Value(Val_Str::mk(Says::verFunc)), verArg);
// 	Assign *assVer = new Assign(hash, f_verify);


// 	newTerms->push_back(encHint);
// 	newTerms->push_back(verTable);
// 	newTerms->push_back(assBuf);
// 	newTerms->push_back(assHash);
// 	newTerms->push_back(assVer);
	
// 	if(addNoKeyConstraint)
// 	{
// 	  ExpressionList *genT = new ExpressionList();
// 	  genT->push_back(Variable::getLocalLocationSpecifier());
// 	  genT->push_back(encId);
// 	  ostringstream var6;
// 	  var6 << Says::varPrefix << newVariable++;
// 	  Variable *genKey = new Variable(Val_Str::mk(var6.str()));
// 	  genT->push_back(genKey);
// 	  Functor *genTable = new Functor(new Value(Val_Str::mk(Says::globalScope + Says::genTable)), genT, true);
	  
// 	  newTerms->push_back(genTable);
// 	}
// 	// now modify the says tuple

// 	ExpressionList *arg = s->saysArgs();
// 	for (iter = saysList->begin(); 
// 	     iter != saysList->end(); iter++){
// 	  arg->push_back(*iter);
// 	}
// 	arg->push_back(proof);
	
// 	s->changeName(Says::saysPrefix + s->name());

// 	// finally return the new terms
// 	return newTerms;
//       }
//       return NULL;
//     }



    string Assign::toString() const {
      return _variable->toString() + " := " + _assign->toString();
    }
    
    TuplePtr 
    Assign::materialize(CommonTable::ManagerPtr catalog, ValuePtr ruleId, string ns)
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
      assignTp->append(Val_Null::mk());         // Position
      assignTp->freeze();
      assignTbl->insert(assignTp); 
      return assignTp;
    }
    
    string Select::toString() const {
      return _select->toString();
    }
    
    TuplePtr 
    Select::materialize(CommonTable::ManagerPtr catalog, ValuePtr parentKey, string ns)
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
      selectTp->append(Val_Null::mk());        // Position
      selectTp->append(Val_Null::mk());        // Access method
      selectTp->freeze();
      selectTbl->insert(selectTp); 
      return selectTp;
    }
    
    string AggregationView::toString() const {
      ostringstream aggFieldStr;
      ostringstream groupByFieldStr;
    
      aggFieldStr << "(";
      groupByFieldStr << "(";
    
      for (ExpressionList::iterator iter = _groupBy->begin();
           iter != _groupBy->end(); iter++) {
        groupByFieldStr << (*iter)->toString();
        if (iter + 1 != _groupBy->end()) {
          groupByFieldStr << ", ";
        }
      }
      groupByFieldStr << ")";
    
    
      for (ExpressionList::iterator iter = _agg->begin();
           iter != _agg->end(); iter++) {
        aggFieldStr << (*iter)->toString();
        if (iter + 1 != _agg->end()) {
          aggFieldStr << ", "; 
        }
      }
      aggFieldStr << ")";
      
      return _operName + "( "
        + groupByFieldStr.str()
        + ", "
        + aggFieldStr.str()
        + ", "
        + _baseTable->toString()
        + " )";    
    }
    
    TuplePtr 
    AggregationView::materialize(CommonTable::ManagerPtr catalog, ValuePtr ruleId, string ns)
    {
      CommonTablePtr aggViewTbl = catalog->table(AGG_VIEW);
      TuplePtr       aggViewTp  = Tuple::mk(AGG_VIEW, true);
      if (ruleId) {
        aggViewTp->append(ruleId);
      }
      else {
        aggViewTp->append(Val_Null::mk());
      }
  
      // Add group by variables 
      ListPtr groupBy = List::mk();
      for (ExpressionList::iterator iter = _groupBy->begin();
           iter != _groupBy->end(); iter++) {
        groupBy->append((*iter)->tuple());
      }
    
      // Add aggregation variables
      ListPtr aggVar = List::mk();
      for (ExpressionList::iterator iter = _agg->begin();
           iter != _agg->end(); iter++) {
        aggVar->append((*iter)->tuple());
      }
  
      /* Pass the baseTable functor the ruleId 
         (which is the rule defining this aggView). */
      TuplePtr baseTableTp = _baseTable->materialize(catalog, ruleId, ns);
      aggViewTp->append((*baseTableTp)[TUPLE_ID]); // Primary key of functor
      
      aggViewTp->append(Val_Str::mk(_operName));   // Aggregation operator
      aggViewTp->append(Val_List::mk(groupBy));    // Group By variables 
      aggViewTp->append(Val_List::mk(aggVar));     // Aggregation variables
  
      aggViewTp->freeze();
      aggViewTbl->insert(aggViewTp);
      return aggViewTp;
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
        (*iter)->materialize(catalog, programID, ns + _name + "::");
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
      _name = (n) ? n->toString() : "";
      _delete = deleteFlag;

      _head = dynamic_cast<Functor*>(lhs);
      _body = rhs;

    }

    StatementList* Table::generateMaterialize()
    {
      StatementList *mat = new StatementList();
      ValuePtr minusOne = Val_Int32::mk(-1);
      ValuePtr one = Val_Int32::mk(1);
      ValuePtr two = Val_Int32::mk(2);
      ValuePtr three = Val_Int32::mk(3);
      ValuePtr four = Val_Int32::mk(4);
      ValuePtr five = Val_Int32::mk(5);
      
      //generate materialize for encHint
      Value *encHintName = new Value(Val_Str::mk(Says::encHint));
      ExpressionList *t = new ExpressionList();
      t->push_back(new compile::parse::Value(one)); 
      t->push_back(new compile::parse::Value(two)); 
      t->push_back(new compile::parse::Value(three)); 
      t->push_back(new compile::parse::Value(four)); 
      t->push_back(new compile::parse::Value(five));
      
      Table *encHint = new Table(encHintName, new compile::parse::Value(minusOne), new compile::parse::Value(minusOne), t);
      
      mat->push_back(encHint);

      //generate materialize for verTable
      Value *verTableName = new Value(Val_Str::mk(Says::verTable));
      t = new ExpressionList();
      t->push_back(new compile::parse::Value(one)); 
      t->push_back(new compile::parse::Value(two)); 
      
      Table *verTable = new Table(verTableName, new compile::parse::Value(minusOne), new compile::parse::Value(minusOne), t);
      
      mat->push_back(verTable);

      //generate materialize for genTable
      Value *genTableName = new Value(Val_Str::mk(Says::genTable));
      t = new ExpressionList();
      t->push_back(new compile::parse::Value(one)); 
      t->push_back(new compile::parse::Value(two)); 
      
      Table *genTable = new Table(genTableName, new compile::parse::Value(minusOne), new compile::parse::Value(minusOne), t);
      
      mat->push_back(genTable);

      return mat;
    }

    TermList* Functor::generateEqTerms(Functor* s){
      assert(name().compare(s->name()) == 0);
      
      ExpressionList::iterator iter1; 
      ExpressionList::iterator iter2; 
      ExpressionList *saysList1 =  const_cast<ExpressionList*>(arguments());
      ExpressionList *saysList2 =  const_cast<ExpressionList*>(s->arguments());
      
      //skip location specifier terms
      return generateEqTerms(saysList1->begin() + 1, saysList1->end(), saysList2->begin() + 1, saysList2->end());
      
    }

    TermList* Functor::generateEqTerms(ExpressionList::iterator start1, ExpressionList::iterator end1, 
				       ExpressionList::iterator start2, ExpressionList::iterator end2,
				       TermList *t){
      TermList *newTerms;
      if(t == NULL){
	newTerms = new TermList();
      }
      else{
	newTerms = t;
      }
      //skip location specifier terms
      for (;start1 < end1 && start2 < end2; start1++, start2++){
	if(!(*start1)->isEqual(*start2))
	{
	  newTerms->push_back(new Assign((*start1)->copy(), (*start2)->copy()));
	}
      }

      return newTerms;
    }

    TermList* Functor::generateSelectTerms(ExpressionList::iterator start1, ExpressionList::iterator end1, 
					   ExpressionList::iterator start2, ExpressionList::iterator end2, int o,
					   TermList *t){
      TermList *newTerms;
      if(t == NULL){
      newTerms = new TermList();
      }
      else{
	newTerms = t;
      }
      //skip location specifier terms
      for (;start1 < end1 && start2 < end2; start1++, start2++){
	if(!(*start1)->isEqual(*start2))
	{
	  newTerms->push_back(new Select(new Bool(o, (*start1)->copy(), (*start2)->copy())));
	}
      }

      return newTerms;
    }

    /**
       generate rules for creating authentication algebra for functors with the given table entry
     */
    void Rule::getAlgebra(TableEntry *tableEntry, StatementList *s){
      TableEntry localTableEntry(Says::saysPrefix + tableEntry->name, tableEntry->fields);
      TableEntry *te = &localTableEntry;
      static int index[]={0, 1, 3};
      for(int i = 0; i < 3; i++){
	int fictVar = 1;
	Functor *f1 = new Functor(te, fictVar);
	Functor *f2 = new Functor(te, fictVar);
	Functor *f3 = new Functor(te, fictVar);
	Functor *head = new Functor(te, fictVar);
	// make all of them local
	f2->changeLocSpec(new Variable(*f1->getLocSpec()));
	f3->changeLocSpec(new Variable(*f1->getLocSpec()));
	head->changeLocSpec(new Variable(*f1->getLocSpec()));
	ExpressionList* f1Args = const_cast<ExpressionList*>(f1->arguments());
	ExpressionList* f2Args = const_cast<ExpressionList*>(f2->arguments());
	ExpressionList* f3Args = const_cast<ExpressionList*>(f3->arguments());
	ExpressionList* headArgs = const_cast<ExpressionList*>(head->arguments());
	
	TermList *t = new TermList();
	t->push_back(f1);
	t->push_back(f2);
	t->push_back(f3);
	
	// generate equality constraints for data parts of f1, f2 and f3
	Functor::generateSelectTerms(f1Args->begin()+1, 
				     f1Args->begin()+(te->fields - TableEntry::numSecureFields),
				     f2Args->begin()+1, 
				     f2Args->begin()+(te->fields - TableEntry::numSecureFields),
				     Bool::EQ, 
				     t);
	
	Functor::generateSelectTerms(f1Args->begin()+1, 
				     f1Args->begin()+(te->fields - TableEntry::numSecureFields),
				     f3Args->begin()+1, 
				     f3Args->begin()+(te->fields - TableEntry::numSecureFields),
				     Bool::EQ, 
				     t);
	
	
	// generate assign constraints for all fields of head
	Says::generateAlgebraCombine(index[i],
				     f1Args->begin()+(te->fields - TableEntry::numSecureFields), 
				     f2Args->begin()+(te->fields - TableEntry::numSecureFields), 
				     headArgs->begin()+(te->fields - TableEntry::numSecureFields), 
				     t);
	
	// finally check they are NOT weaker* than any other known says primitive
	// by copying data parts and using algebra to generate the (P, R, k, V) parts
	Says::generateAlgebraLT(
				f3Args->begin()+(te->fields - TableEntry::numSecureFields), 
				headArgs->begin()+(te->fields - TableEntry::numSecureFields), 
				t);
	
	Functor::generateEqTerms(headArgs->begin()+1, 
				 headArgs->begin()+(te->fields - TableEntry::numSecureFields),
				 f1Args->begin()+1, 
				 f1Args->begin()+(te->fields - TableEntry::numSecureFields),
				 t);

	Rule *r = new Rule(head, t, false);
	s->push_back(r);
      }
      //issues: what if the tableEntry is not materialized?
      // check before calling this func
    }

    TermList* Says::generateAlgebraLT(ExpressionList::iterator start1, 
				      ExpressionList::iterator start2, 
				      TermList *newTerms){
      TermList *t;
      if(newTerms == NULL){
      t = new TermList();
      }
      else{
	t = newTerms;
      }


      //      is 1 < 2
      // assert that there are only four terms in each of the iterator
      // first generate the lte terms
      Functor::generateSelectTerms(start1, start1 + (TableEntry::numSecureFields - 1), 
				   start2, start2 + (TableEntry::numSecureFields - 1), Bool::LTE, t);
      ExpressionList *expT = new ExpressionList();
      expT->push_back((*start1)->copy());
      Expression *modP = new Function(new Value(Val_Str::mk(Function::mod)), expT);
      expT = new ExpressionList();
      expT->push_back((*start2)->copy());
      Expression *modPPrime = new Function(new Value(Val_Str::mk(Function::mod)), expT);
      Math *lhs = new Math(Math::MINUS, modP, modPPrime);
      Math *rhs = new Math(Math::MINUS, (*(start1+Says::K))->copy(), (*(start2+Says::K))->copy());
      Bool *eq = new Bool(Bool::LTE, lhs, rhs);
      Select *term = new Select(eq);
      t->push_back(term);

      return t;
    }

    TermList* Says::generateAlgebraCombine(int o, 
					   ExpressionList::iterator start1, 
					   ExpressionList::iterator start2, 
 					   ExpressionList::iterator headStart, 
					   TermList *newTerms){
      TermList *t;
      if(newTerms == NULL){
	t = new TermList();
      }
      else{
	t = newTerms;
      }

      // using || at the index[o] location and & at other locations
      // k combination is decided later
      int count = 0;
      for(; count < 4; count++){
	if(count != Says::K){
	  int op = ((count == o)?Math::BITOR:Math::BITAND);
	  Expression *p = new Bool(op, (*(start1+count))->copy(), (*(start2+count))->copy());
	  Term *assign = new Assign((*(headStart+count))->copy(), p);
	  t->push_back(assign);
	}
      }
      
      // now generate the appropriate k value
      //first generate k1+k2
      Expression *plus = new Math(Math::PLUS, (*(start1+Says::K))->copy(), (*(start2+Says::K))->copy());
      // now the union/intersection set
      int op = ((Says::SPEAKER == o)?Math::BITAND:Math::BITOR);
      Expression *combinedSet = new Bool(op, (*(start1+Says::K))->copy(), (*(start2+Says::K))->copy());
      //mod
      ExpressionList *expT = new ExpressionList();
      expT->push_back(combinedSet);
      Expression *mod = new Function(new Value(Val_Str::mk(Function::mod)), expT);

      Expression *rhsMax;
      if(o == Says::SPEAKER){
	ExpressionList *maxT = new ExpressionList();
	maxT->push_back( (*(start1+Says::K))->copy());
	maxT->push_back( (*(start2+Says::K))->copy());

	Expression *firstMax = new Function(new Value(Val_Str::mk(Function::max)), maxT);

	maxT = new ExpressionList();
	maxT->push_back(firstMax);
	maxT->push_back(mod);

	rhsMax = new Function(new Value(Val_Str::mk(Function::max)), maxT);
      }
      else{
	ExpressionList *maxT = new ExpressionList();
	maxT->push_back(new Value(Val_UInt32::mk(0)));
	maxT->push_back(mod);

	rhsMax = new Function(new Value(Val_Str::mk(Function::max)), maxT);

      }
      // final rhs
      Math *rhs = new Math(Math::MINUS, plus, mod);
      Term *assign = new Assign((*(headStart+Says::K))->copy(), rhs);
      t->push_back(assign);
      return t;
    }

    // add rule to verify the correctness of proof for tables of type TableEntry t
    void Rule::getVerifier(TableEntry *t, StatementList *s){
      int newVariable = 1;
      TableEntry localTableEntry(t->name, t->fields);
      TableEntry *te = &localTableEntry;
      te->name = Says::makeSays + t->name;
      Functor *rhs = new Functor(te, newVariable);
      
      Functor *head = new Functor(*rhs);      
      head->changeName(Says::saysPrefix + t->name);

      TermList *newTerms = new TermList();
      newTerms->push_back(rhs);

      ExpressionList::iterator iter; 

      //create buffer creation logic
      ostringstream var3;
      var3 << Says::varPrefix << newVariable++;
      Variable *buf = new Variable(Val_Str::mk(var3.str()));
      
      // check if the concatenate operation is needed or not
      // remember that the loc specifier is excluded but the table name must be included
      Value *table = new Value(Val_Str::mk(t->name));
      Expression *cur = table;
      
      ExpressionList *list = const_cast<ExpressionList*>(rhs->arguments());
      if(rhs->arguments()->size() > TableEntry::numSecureFields + 1)
      {
	iter = list->begin();
	//exclude location specifier and all secure fields
	iter++; 
	for (; iter < list->end() - TableEntry::numSecureFields; iter++){
	  cur = new Math(Math::APPEND, cur, *iter);
	}
      }
      
      Assign *assBuf = new Assign(buf, cur);
      
      //create verification logic
      ostringstream var4;
      var4 << Says::varPrefix << newVariable++;
      Variable *hash = new Variable(Val_Str::mk(var4.str()));
      
      ExpressionList *hashArg = new ExpressionList();
      hashArg->push_back(buf);
      Function *f_hash = new Function(new Value(Val_Str::mk(Says::hashFunc)), hashArg);
      Assign *assHash = new Assign(hash, f_hash);
      
      
      ExpressionList *verArg = new ExpressionList();
      for (; iter < list->end(); iter++){
	verArg->push_back((*iter)->copy());
      }
      verArg->push_back(hash);

      Function *f_verify = new Function(new Value(Val_Str::mk(Says::verFunc)), verArg);
      Select *assVer = new Select(new Bool(Bool::EQ, new Value(Val_UInt32::mk(1)), f_verify));
      
      newTerms->push_back(assBuf);
      newTerms->push_back(assHash);
      newTerms->push_back(assVer);

      Rule *r = new Rule(head, newTerms, false);
      r->canonicalizeRule();
      s->push_back(r);
    }
    
    Functor::Functor(TableEntry *t, int &fictVar){
      _complement = false;
      _name = t->name;
      _args = new ExpressionList();
      for(int i = 0; i < t->fields; i++){
	ostringstream oss;
	oss << Says::varPrefix << fictVar++;
	Variable *tmp = new Variable(Val_Str::mk(oss.str()), i == 0);	
	_args->push_back(tmp);
      }
    }

    TableSet* Rule::initializeRule(StatementList *s)
    {
      int newVariable = 1;
      Says *sh;
      std::list<TermList*> newRuleComparators;
      TableSet *tables = new TableSet();
      sh = dynamic_cast<Says*>(_head);
      string headTableName = ((sh!=NULL)?sh->name():"");
      if(sh != NULL)
      {
	// first rule generates the proof assuming that the node 
	// executing the rule has the key
	TermList *newTermsGen = Says::normalizeGenerate(sh, newVariable);
	if(newTermsGen != NULL){
	  newRuleComparators.push_back(newTermsGen);
	  _head = new Functor(sh);

	}
      }
      //      TermList *_bodyClone = NULL;

      for (TermList::iterator iter = _body->begin(); 
           iter != _body->end(); iter++) {
	Says *s;
	if ((s = dynamic_cast<Says*>(*iter)) != NULL)
	{
	  //  if(_bodyClone == NULL){
	  //	  _bodyClone = new TermList(_body->begin, iter);
	  //}
	  tables->insert(new TableEntry(s->name(), s->saysArgs()->size() + TableEntry::numSecureFields));
	  string saysTableName = s->name();
	  Says::normalizeVerify(s, newVariable);

	  //	  Says::normalizeVerify(s, newVariable);
	  if(sh != NULL && (saysTableName.compare(headTableName)==0))
	  {
	    ExpressionList *head =  const_cast<ExpressionList*>(_head->arguments());

	    TermList *comparisonTerms = Says::generateEqTerms(head->begin(),
							      head->end()-TableEntry::numSecureFields,
							      s->saysArgs()->begin(), 
							      s->saysArgs()->end()-TableEntry::numSecureFields);
	    Says::generateAlgebraLT(head->end()-TableEntry::numSecureFields, 
				    s->saysArgs()->end()-TableEntry::numSecureFields, 
				    comparisonTerms);
	    Says::generateEqTerms(head->end()-1, head->end(), 
				  s->saysArgs()->end()-1, s->saysArgs()->end(), 
				  comparisonTerms);
	    newRuleComparators.push_back(comparisonTerms);
	  }
	 
	  
	  _body->erase(iter);
	  // make a deep copy even though we can live with shallow one 
	  // to ensure that the following delete can be executed safely
	  _body->push_back(new Functor(*s));
	  delete s;
	  
// 	  if(newTerms != NULL){

// 	    for (TermList::iterator it = newTerms->begin(); 
// 		 it != newTerms->end(); it++) {
// 	      //      _bodyClone->push_back(*it);
// 	      _body->push_back(*it);
// 	    }
// 	    delete newTerms;
// 	  }
	  
	  
	}
      }

      //      if(_bodyClone != NULL){
      //delete _body;
      //_body = _bodyClone;
      //}
      
      if(sh != NULL)
      {
	//	delete sh;
	// copy this rule into a new rule and insert the new rule into the list as well 
	// as modify the existing rule
	int size = newRuleComparators.size();
	Rule *r;
	for(int i = 0; i < size; i++){
	// if i < size - 1, then create a copy for next stage
	// else use the current copy
	  if(i < size - 1){
	    r = new Rule(*this);
	    r->resetName();
	    s->push_back(r);
	  }
	  else{
	    r = this;
	  }

	  TermList *newTermsUse = newRuleComparators.front();
	  newRuleComparators.pop_front();
	  if(newTermsUse != NULL){
	    
	    for (TermList::iterator it = newTermsUse->begin(); 
		 it != newTermsUse->end(); it++) {
	      r->_body->push_back(*it);
	    }
	    delete newTermsUse;
	  }

	}
      }


      return tables;
//      canonicalizeRule();
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
      if (pred->name() == "periodic") {
        // Skip this one
        return;
      }

      // Next, we canonicalize all the args in the functor.  We build up
      // a list of argument names - the first 'arity' of these will be the
      // free variables in the rule head.  Literals and duplicate free
      // variables here are eliminated, by a process of appending extra
      // "eq" terms to the body, and inventing new free variables.
      ExpressionList *args = const_cast<ExpressionList*>(pred->arguments());
      for (ExpressionList::iterator i = args->begin();
           i != pred->arguments()->end(); i++) {
        Variable *var = NULL;
        Value    *val = NULL;
        Math     *math = NULL;
        Function *func = NULL;
    
        if ((var = dynamic_cast<Variable*>(*i)) != NULL) {
          // The argument is a free variable - the usual case. 
          ExpressionList::iterator prev = pred->find(var);
          if (prev < i) {
            ostringstream oss;
            oss << "$" << fict_varnum++;
    	    // We've found a duplicate variable in the head. Add a new
    	    // "eq" term to the front of the term list. 
            Variable *tmp = new Variable(Val_Str::mk(oss.str()));
            (headPred) ? ruleBody->push_back(new Assign(tmp, *i))
                       : ruleBody->push_back(new Select(new Bool(Bool::EQ, tmp, *i)));
            pred->replace(i, tmp);
          }
        }
        else if ((val = dynamic_cast<Value*>(*i)) != NULL) {
          ostringstream oss;
          oss << "$" << fict_varnum++;
          Variable *tmp = new Variable(Val_Str::mk(oss.str()));
          pred->replace(i, tmp);
          (headPred) ? ruleBody->push_back(new Assign(tmp, val))
                     : ruleBody->push_back(new Select(new Bool(Bool::EQ, tmp, val)));
        }
        else if ((math = dynamic_cast<Math*>(*i)) != NULL) {
          ostringstream oss;
          oss << "$" << fict_varnum++;
          Variable *tmp = new Variable(Val_Str::mk(oss.str()));
          pred->replace(i, tmp);
          (headPred) ? ruleBody->push_back(new Assign(tmp, math))
                     : ruleBody->push_back(new Select(new Bool(Bool::EQ, tmp, math)));
        }
        else if ((func = dynamic_cast<Function*>(*i)) != NULL) {
          ostringstream oss;
          oss << "$" << fict_varnum++;
          Variable *tmp = new Variable(Val_Str::mk(oss.str()));
          pred->replace(i, tmp);
          (headPred) ? ruleBody->push_back(new Assign(tmp, func))
                     : ruleBody->push_back(new Select(new Bool(Bool::EQ, tmp, func)));
        }
        else if (dynamic_cast<Aggregation*>(*i) == NULL) {
          throw Exception(0, "Parse rule unknown functor body type.");
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
      TuplePtr headTp = _head->materialize(catalog, (*ruleTp)[TUPLE_ID], ns);
      ruleTp->append((*headTp)[TUPLE_ID]);             // Add the "head" functor identifer.
      ruleTp->append(Val_Null::mk());                  // The P2DL desc. of this rule
      ruleTp->append(Val_UInt32::mk(_delete));         // Delete rule?
      ruleTp->append(Val_UInt32::mk(_body->size()+1)); // Term count?
      ruleTp->freeze();
      ruleTbl->insert(ruleTp);	               // Add rule to rule table.
    
      /* Create the functors associated with the rule body. Each functor row
       * will reference (foreign key) the rule identifier of the rule 
       * created above. */
      for (TermList::const_iterator iter = _body->begin();
           iter != _body->end(); iter++) {
        (*iter)->materialize(catalog, (*ruleTp)[TUPLE_ID], ns);
      }
      return ruleTp;
    }

    Watch::Watch(string w, string modifiers) 
      : _watch(w), _modifiers(modifiers) { }

    TuplePtr 
    Watch::materialize(CommonTable::ManagerPtr catalog, ValuePtr programId, string ns)
    {
      CommonTablePtr watchTbl = catalog->table(WATCH);
      TuplePtr       watchTp  = Tuple::mk(WATCH, true);
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
        watchTp->append(Val_Null::mk()); // Watch modifier
      else
        watchTp->append(Val_Str::mk(_modifiers)); // Watch name
      // TODO Add a field for the modifiers (also in the watch table)
      watchTp->freeze();
      watchTbl->insert(watchTp); 
      return watchTp;
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
      TuplePtr tpl = Tuple::mk(n->toString());
    
      ExpressionList::const_iterator iter;
      for (iter = a->begin(); iter != a->end(); iter++) {
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
    
    Table::Table(Expression *name, 
                 Expression *ttl, 
                 Expression *size, 
                 ExpressionList *keys,
		 bool says)
    {
      _name = name->toString() ;
      _says = says;

      int myTtl = Val_Int64::cast(ttl->value());
      if (myTtl == -1) {
        _lifetime = Table2::NO_EXPIRATION;
      } else if (myTtl == 0) {
        // error("bad timeout for materialized table");
      } else {
        _lifetime = boost::posix_time::seconds(myTtl);
      }
    
    
      _size = Val_Int64::cast(size->value());
      // Hack because infinity token has a -1 value
      if (_size == -1) {
        _size = Table2::NO_SIZE;
      }
    
      if (keys) {
        for (ExpressionList::const_iterator i = keys->begin();
             i != keys->end(); i++)
          _keys.push_back(Val_UInt32::cast((*i)->value()));
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
	_name = Says::saysPrefix + _name;
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

      return catalog->createTable(scopedName, _keys, _size, _lifetime);
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
      static bool materialized = false;
      if(parserCall)
      {
	_statements = s;
	if(!materialized){
	  StatementList *mat = Table::generateMaterialize();
	  for (StatementList::iterator iter = mat->begin();
	       iter != mat->end(); iter++) { 
	    s->push_back(*iter);
	  }
	  delete mat;
	  materialized = true;
	}
      }

      Rule* r;
      Namespace *nmSpc;
      TableSet secureTable;
      Table *tab;
      std::set<string> materializedTables;
      for (StatementList::iterator iter = s->begin();
             iter != s->end(); iter++) { 

	if ((r = dynamic_cast<Rule*>(*iter)) != NULL) 
	{
	  TableSet *table = r->initializeRule(s);
	  if(table->size() > 0)
	  {
	    for(TableSet::iterator iter = table->begin(); 
		iter != table->end(); iter++){
	      secureTable.insert(*iter);
	    }
	  }
	  delete table;
	  if(printOverLog)
	  {
	    std::cout<<r->toString()<<std::endl;
	  }
	  r->canonicalizeRule();
	  
	}
	else if((nmSpc = dynamic_cast<Namespace*>(*iter)) != NULL) 
	{
	  if(nmSpc->statements() != NULL)
	    program(nmSpc->statements(), false);
	}
	else if((tab = dynamic_cast<Table*>(*iter)) != NULL)
	{
	  if(tab->says()){
	    materializedTables.insert(tab->name());
	  }
	  tab->initialize();
	}
	else
	{
	  // do nothing
	}

      }

      //insert rules for authentication algebra and validation
      for(TableSet::iterator iter = secureTable.begin(); 
	  iter != secureTable.end(); iter++){
	//	std::cout<<"statement list size before getVerifier"<<s->size()<<std::endl;
	//	std::cout<<"statement list size before getVerifier"<<_statements->size()<<std::endl;
	Rule::getVerifier(*iter, s);
	//	std::cout<<"statement list size after getVerifier"<<_statements->size()<<std::endl;
	//	std::cout<<"statement list size after getVerifier"<<s->size()<<std::endl;
	if(materializedTables.find((*iter)->name) != materializedTables.end())
	{
	  Rule::getAlgebra(*iter, s);
	}
	delete *iter;
      }
      
       if(parserCall){
	for (StatementList::iterator iter = s->begin();
             iter != s->end(); iter++) { 
	  std::cout<<(*iter)->toString()<<std::endl;
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
      program = program->clone(PROGRAM);
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
          (*iter)->materialize(catalog, pid, ""); 
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
        lexer = new OL_Lexer(str);
        ol_parser_parse(this);
        delete lexer; 
        lexer = NULL;
      } 
      catch (compile::Exception e) {
        int line = lexer->line_num();
        delete lexer; 
        lexer = NULL;

        TELL_ERROR << "compile::Exception: " << e.toString() << std::endl;
        throw compile::parse::Exception(line, e.toString());
      }
    }
    
    void 
    Context::error(string msg)
    {
      throw compile::parse::Exception(lexer->line_num(), msg);
    }
  }
}


