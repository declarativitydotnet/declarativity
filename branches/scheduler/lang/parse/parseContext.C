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
#include "parseContext.h"
#include "plumber.h"
#include "olg_lexer.h"
#include "tuple.h"

#include "systemTable.h"
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
      a << _variable->toString() << " >";
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
      tp->append(Val_Int64::mk(_args->size()));
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
      f << name() << "( ";
      for (ExpressionList::const_iterator iter = _args->begin(); 
           iter != _args->end(); iter++) {
        f << (*iter)->toString();
        if (iter+1 != _args->end()) f << ", ";
      }
      f << " )";
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
    
    Functor::Functor(Expression *n, ExpressionList *a)
      : _name(n->toString()), _args(a)
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
    
    void Functor::replace(ExpressionList::iterator i, 
                          Expression *e) {
      ExpressionList::iterator next = _args->erase(i);
      _args->insert(next, e);
    }
  
    string Assign::toString() const {
      return _variable->toString() + " = " + _assign->toString();
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

      canonicalizeAttributes(dynamic_cast<Functor*>(lhs), rhs, true);
      for (TermList::iterator iter = rhs->begin(); 
           iter != rhs->end(); iter++)
      {
        Functor *pred;
        if ((pred = dynamic_cast<Functor*>(*iter)) != NULL)
        {
          canonicalizeAttributes(pred, rhs, false);
        }
      }

      _head = dynamic_cast<Functor*>(lhs);
      _body = rhs;
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
      ruleTp->append(Val_Int64::mk(_delete));         // Delete rule?
      ruleTp->append(Val_Int64::mk(_body->size()+1)); // Term count?
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
      TuplePtr tpl = Tuple::mk();

      tpl->append(Val_Str::mk(n->toString())); // my tuple name

      // The fields
      ExpressionList::const_iterator iter;
      for (iter = a->begin();
           iter != a->end();
           iter++) {
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
                 ExpressionList *keys)
    {
      _name = name->toString();
    
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
          _keys.push_back(Val_Int64::cast((*i)->value()));
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
    Context::program(StatementList *s) 
    {
      assert(_statements == NULL);
      _statements = s;
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
        lexer = new OLG_Lexer(str);
        olg_parser_parse(this);
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
