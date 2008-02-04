/*
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 * DESCRIPTION: 
 *
 */

#ifndef __PARSE_CONTEXT_IMPL_H__
#define __PARSE_CONTEXT_IMPL_H__

#include <string>
#include <deque>
#include <set>
#include <iostream>
#include <map>
#include <utility>
#include "compileContext.h"
#include "element.h"
#include "elementRegistry.h"
#include "tuple.h"
#include "value.h"
#include "val_str.h"
#include "val_null.h"
#include "val_int64.h"
#include "set.h"

class OLG_Lexer;

namespace compile {
  namespace parse {

    //    class Secure;

    class Exception : public compile::Exception {
    public:
      Exception(uint line, string msg) 
      : compile::Exception(msg), lineNumber_(line), message_(msg) {}
      virtual ~Exception() {};
    
      virtual string toString() const { 
        ostringstream oss; 
        oss << "Parse Error: line "  << lineNumber_ << ", " << message_;
        return oss.str();
      }
  
      virtual uint lineNumber() const
      { return lineNumber_; }      
    
      virtual string message() const
      { return message_; }
    
    private:
      uint   lineNumber_;
      string message_;
    };
  
    /* Top level compile structures */
    class Expression {
    public:
      virtual ~Expression() {};
  
      virtual string toString() const = 0;

      virtual Expression* copy() const = 0;
  
      virtual const ValuePtr value() const
      { throw compile::Exception("Expression does not have a value."); }

      virtual bool isEqual(Expression *e) const {
	return false;
      }
  
      /** Creates, but does not materialize, a tuple representing the expression.
       *  The identifier of any variables that the expression depends on will 
       *  be placed in 'depends' argument.
       */ 
      virtual ValuePtr tuple() const = 0; 
    };
    typedef std::deque<Expression *>     ExpressionList;
    typedef std::deque<ExpressionList *> ExpressionListList;
    
    class Term {
    public:

      virtual Term* copy() const = 0;

      virtual ~Term() {};
      
      virtual string toString() const = 0;

      virtual void makeNew(Expression* locSpec, Expression* opaque) { 
	throw compile::Exception("makeNew not defined for this class"); 
      };

      virtual TuplePtr materialize(CommonTable::ManagerPtr, ValuePtr, string, uint32_t pos) = 0;
    };
    typedef std::deque<Term *> TermList;
    typedef std::deque<TermList *> TermListList;
    
    class Statement {
    public:

      virtual Statement* copy() const = 0;

      virtual ~Statement() {};
  
      virtual void makeNew() { throw compile::Exception("makeNew not defined for this class"); }

      virtual string toString() const = 0;
  
      virtual TuplePtr materialize(CommonTable::ManagerPtr, ValuePtr, string) = 0;
    };
    typedef std::deque<Statement *> StatementList;
  
    class Value : public Expression { 
    public:
      Value(ValuePtr val) : _value(val) {};
    
      virtual Expression* copy() const{
	Value *v = new Value(*this);
	return v;
      }

      Value(const Value &v) : _value(v._value) {};

      virtual string toString() const 
      { return _value->toString(); };
  
      virtual ValuePtr tuple() const;

      virtual void replace(ValuePtr newVal){
	_value = newVal;
      }
      virtual bool isEqual(Expression *e) const {
	Value* v = dynamic_cast<Value*>(e);
	if(v == NULL){
	  return false;
	}
	else{
	  return _value->compareTo(v->_value) == 0;
	}
      }
  
      virtual const ValuePtr value() const { return _value; }
    private:
      ValuePtr _value;
    };
    
    class Variable : public Expression { 
    public:
      Variable(ValuePtr var, bool l=false, bool newLocSpec = false) 
      : _value(var), _location(l), _newLocSpec(newLocSpec) {};
    
      Variable(const string& var, bool l=false, bool newLocSpec = false) 
        : _value(Val_Str::mk(var)),
          _location(l),
          _newLocSpec(newLocSpec)   {};
  
      Variable(const Variable& var) 
      : _value(var._value), 
	_location(var._location),
        _newLocSpec(var._newLocSpec)  {};

      virtual Expression* copy() const{
	Variable *v = new Variable(*this);
	return v;
      }

      virtual string toString() const 
      { 
	return string(_location ? "@" :"") +  
	  string(_newLocSpec ? "&" : "") + 
	  _value->toString(); 
      };

      virtual void resetLocSpec(){
	_location = false;
      }

      virtual bool location() const { return _location; }

      virtual bool isEqual(Expression *e) const {
	Variable* v = dynamic_cast<Variable*>(e);
	if(v == NULL){
	  return false;
	}
	else{
	  return _value->compareTo(v->_value) == 0;
	}
      }

      virtual ValuePtr tuple() const;
    private:
      ValuePtr _value;
      bool     _location;
      bool     _newLocSpec;
    
    };
    
    class Aggregation : public Expression {
    public:
      Aggregation(string o, Expression *v);
    
      Aggregation(const Aggregation &o):_operName(o._operName){
	_variable = new Variable(*o._variable);
      };

      virtual Expression* copy() const{
	Aggregation *v = new Aggregation(*this);
	return v;
      }

      virtual string toString() const;
    
      virtual string operName() const
      { return _operName; }
    
      virtual Variable* variable() const 
      { return _variable; }
    
      virtual ValuePtr tuple() const;
    private:
      string    _operName;
      Variable* _variable;
    };
    
    class Bool : public Expression {
    public:
      enum Operator{NOT, AND, OR, EQ, NEQ, GT, LT, LTE, GTE, RANGEI, NOP};
    
      Bool(int o, Expression *l, Expression *r=NULL)
      : _lhs(l), _rhs(r) {
        switch(o) {
          case NOT:   _operName = "not";    break;
          case RANGEI: _operName = "in";     break;
          case AND:   _operName = "and";    break;
          case OR:    _operName = "or";     break;
          case EQ:    _operName = "==";     break;
          case NEQ:   _operName = "== not"; break;
          case GT:    _operName = ">";      break;
          case LT:    _operName = "<";      break;
          case LTE:   _operName = "<=";     break;
          case GTE:   _operName = ">=";     break;
          default: assert(0);
        }
      } 
    
      Bool(const Bool &b)
	: _operName(b._operName){
	_lhs = b._lhs->copy();
	_rhs = b._rhs->copy();
      };

      virtual Expression* copy() const{
	Bool *v = new Bool(*this);
	return v;
      }

      virtual ~Bool() 
      { delete _lhs; if (_rhs) delete _rhs; }
    
      virtual string toString() const;
    
      virtual string operName() const 
      { return _operName; }
    
      virtual const Expression* lhs() const
      { return _lhs; }
      
      virtual const Expression* rhs() const
      { return _rhs; }
    
      virtual ValuePtr tuple() const;
    private:
      string      _operName;
      Expression* _lhs;
      Expression* _rhs;
    };
    
    class Range : public Expression {
    public:
      enum Operator{RANGEOO, RANGEOC, RANGECO, RANGECC};
    
      Range(Operator o, 
            Expression *l, 
            Expression *r)
        : _lhs(l), _rhs(r), _type(o) { 
        switch(o) {
          case RANGEOO: _operName = "()"; break;
          case RANGEOC: _operName = "(]"; break; 
          case RANGECO: _operName = "[)"; break; 
          case RANGECC: _operName = "[]"; break; 
        }
      };

      Range(const Range &b)
	: _operName(b._operName){
	_lhs = b._lhs->copy();
	_rhs = b._rhs->copy();
      };

      virtual Expression* copy() const{
	Range *v = new Range(*this);
	return v;
      }
    
      virtual string toString() const;
    
      virtual string operName() const 
      { return _operName; }
    
      virtual const Expression* lhs() const
      { return _lhs; }
      
      virtual const Expression* rhs() const
      { return _rhs; }
    
      virtual ValuePtr tuple() const;
    private:
      string      _operName;
      Expression* _lhs;
      Expression* _rhs;
      Operator _type;
    };
    
    class Math : public Expression {
    public:
      enum Operator {LSHIFT, RSHIFT, PLUS, MINUS, TIMES, DIVIDE,
                     MODULUS, BITOR, BITAND, BITXOR, BITNOT, APPEND, NOP};
    
      Math(Operator o, 
           Expression *l, 
           Expression *r=NULL) 
        : _lhs(l), _rhs(r) {
        // TODO: if (oper != NOP && rhs == NULL) ERROR!
        switch(o) {
        case LSHIFT:  _operName = "<<"; break;
        case RSHIFT:  _operName = ">>"; break;
        case PLUS:    _operName = "+";  break;
        case MINUS:   _operName = "-";  break;
        case TIMES:   _operName = "*";  break;
        case DIVIDE:  _operName = "/";  break;
        case MODULUS: _operName = "%";  break;
        case BITAND: _operName = "&";  break;
        case BITOR: _operName = "|";  break;
        case BITXOR: _operName = "^"; break;
        case BITNOT: _operName = "~"; break;
        case APPEND: _operName = "|||";  break;
        default: assert(0);
        }
      }
    
      Math(const Math &b)
      : _operName(b._operName){
	_lhs = b._lhs->copy(); 
	_rhs = b._rhs->copy();
      };


      virtual Expression* copy() const{
	Math *v = new Math(*this);
	return v;
      }

      virtual ~Math() { delete _lhs; if (_rhs) delete _rhs; }
    
      virtual string toString() const;
    
      virtual string operName() const 
      { return _operName; }
    
      virtual const Expression* lhs() const 
      { return _lhs; }
    
      virtual const Expression* rhs() const 
      { return _rhs; }
    
      virtual ValuePtr tuple() const;
  
      virtual const ValuePtr value() const;
    private:
      string      _operName;
      Expression* _lhs;
      Expression* _rhs;
    };
    
    class Vector : public Expression {
    public:
      Vector(ExpressionList *o);

      Vector(const Vector &b)
	: _vector(b._vector){
      };
    
      virtual Expression* copy() const{
	Vector *v = new Vector(*this);
	return v;
      }

      virtual string toString() const 
      { return _vector->toString(); };
    
      virtual ValuePtr tuple() const;
    private:
      ValuePtr _vector;
    };
    
    class Matrix : public Expression {
    public:
      Matrix(ExpressionListList *r);

      Matrix(const Matrix &b)
	: _matrix(b._matrix){
      };
    
      virtual Expression* copy() const{
	Matrix *v = new Matrix(*this);
	return v;
      }

      virtual string toString() const
      { return _matrix->toString(); };
    
      virtual ValuePtr tuple() const;
    private:
      ValuePtr _matrix;
    };
    
    class Function : public Expression {
    public:
      const static string max;
      const static string concat;
      const static string mod;
      Function(Expression *n, ExpressionList *a) 
        : _name(n->toString()), _args(a) { };

      Function(const Function &f) 
        : _name(f._name){

	ExpressionList *a = new ExpressionList();
	ExpressionList::size_type sz = f._args->size();

	for (unsigned i=0; i<sz; i++){
	  Expression *e = f._args->at(i);
	  a->push_back(e->copy());
	}
	_args = a;
      };

      virtual Expression* copy() const{
	Function *v = new Function(*this);
	return v;
      }
    
      virtual ~Function() { delete _args; };
    
      virtual string toString() const;
    
      virtual const ExpressionList* arguments() const
      { return _args; };
    
      virtual ValuePtr tuple() const;
    private:
      string          _name;
      ExpressionList* _args;
    };

    class Sets : public Expression {
    public:
      Sets(ExpressionList *a) 
        : _args(a) { };

      Sets(const Sets &s) 
      {

	ExpressionList *a = new ExpressionList();
	ExpressionList::size_type sz = s._args->size();

	for (unsigned i=0; i<sz; i++){
	  Expression *e = s._args->at(i);
	  a->push_back(e->copy());
	}
	_args = a;
      };

      virtual Expression* copy() const{
	Sets *v = new Sets(*this);
	return v;
      }
    
      virtual ~Sets() { delete _args; };
    
      virtual string toString() const;
    
      virtual const ExpressionList* arguments() const
      { return _args; };
    
      virtual ValuePtr tuple() const;
      
    private:
      ExpressionList* _args;
    };

    
    struct TableEntry{
      string name;
      int fields;
      static const uint32_t numSecureFields;
      TableEntry(string _name, int _fields){
	name = _name;
	fields = _fields;
      }
    };

    struct lttbl
    {
      bool operator()(const TableEntry *s1, const TableEntry *s2) const
      {
	return s1->name.compare(s2->name) < 0;
      }
    };
    
    /***************************************************
     * TERMS
     ***************************************************/

    typedef std::pair<int, ExpressionList::iterator> TotalIterator;

    class Functor : public Term {
    public:
      static uint32_t fictVarCounter;
      const static string FICTPREFIX;
      enum SubTerms{T_FUNCTOR = 0, T_LOCSPEC, T_OPAQUE, T_HINT, T_SAYS, NOT_FOUND};

      Functor(Expression *n, ExpressionList *a, bool complement = false, bool newF = false); 

      // shallow copy
      Functor(Functor *f){
	_name = f->_name;
	_args = f->_args;
	_complement = f->_complement;
	_new = f->_new;
	_opaque = f->_opaque;
	_locSpec = f->_locSpec;
	_hint = f->_hint;

      }; 
     
      Functor(TableEntry* t, int &fictVar);

      // deep copy
      Functor(const Functor &f) 
        : _name(f._name), _complement(f._complement),
          _locSpec(f._locSpec),
          _opaque(f._opaque),
          _hint(f._hint){ 
	ExpressionList *a = new ExpressionList();
	ExpressionList::iterator it = f._args->begin();
	while (it != f._args->end()){
	  Expression *expr = *it;
	  a->push_back(expr->copy());
	  it++;
	}
	_args = a;
	_new = f._new;
      };

      virtual Term* copy() const{
	Functor *v = new Functor(*this);
	return v;
      }

      virtual ~Functor() { 
	delete _args; 
	if(_new){
	  delete _locSpec;
	  delete _opaque;
	  delete _hint;
	}
      }
    
      virtual string toString() const;

      virtual void changeLocSpec(Variable *l);

      virtual Variable* getLocSpec();

      virtual Expression* getOpaque(){
	return _opaque;
      }

      virtual Expression* getNewLocSpec(){
	return _locSpec;
      }
    
      virtual Expression* getHint(){
	return _hint;
      }

      virtual TuplePtr materialize(CommonTable::ManagerPtr, ValuePtr, string, uint32_t);
    
      virtual string name() const { return _name; }

      virtual bool isComplement() const { return _complement; }

      virtual bool isNew() const { return _new; }
    
      virtual const ExpressionList* arguments() const 
      { return _args; }
    
      virtual const Aggregation* aggregate() const;
    
      virtual ExpressionList::iterator
      find(const Expression *e) const;

      virtual TotalIterator
      totalFind(const Expression *e) const;

      //      TermList* generateEqTerms(Functor* s);
    
      virtual void changeName(string newName)
      {
	_name = newName;
      }

      void makeNew(Expression* locSpec, Expression* opaque){
	_new = true;
	_locSpec = locSpec;
	_opaque = opaque;
	ostringstream oss;
	oss << FICTPREFIX << fictVarCounter++; 
        _hint = new compile::parse::Variable(Val_Str::mk(oss.str())); 
      }

      void resetHint(){
	if(_new){
	  _hint = new Value(Val_Null::mk());
	}
      }

      void processNew(TermList *t = NULL, Expression *val = NULL);

      virtual void replace(ExpressionList::iterator i, 
                           Expression *e);

      virtual void replace(TotalIterator i, 
                           Expression *e);
     
    protected:
      string          _name;
      ExpressionList* _args;
      bool _complement;
      bool _new;
      Expression *_locSpec;
      Expression* _opaque;
      Expression* _hint;
    };
    class Says : public Functor {
    public:
      const static string verTable;
      const static string genTable;
      const static string encHint;
      const static string varPrefix;
      const static string rulePrefix;
      const static string hashFunc; 
      const static string verFunc;
      const static string genFunc;
      const static string saysPrefix;
      const static string saysSuffix;
      const static string makeSays;
      const static string globalScope;
      enum additionAxis{SPEAKER=0, RECEIVER, K, VERIFIER};

      Says(Functor* f, ExpressionList* s):Functor(f), _says(s){
	if(f->isComplement())
	{
	  throw compile::Exception("Functor with ! can't be said");
	}
      };

      Says(const Says &s):Functor(s){
	ExpressionList *a = new ExpressionList();
	ExpressionList::iterator it = s._says->begin();
	while (it != s._says->end()){
	  a->push_back((*it)->copy());
	  it++;
	}
	_says = a;
      };

      virtual Term* copy() const{
	Says *v = new Says(*this);
	return v;
      }

      TuplePtr materialize(CommonTable::ManagerPtr catalog, ValuePtr ruleId, string ns, uint32_t pos);

      virtual string toString() const;

      virtual ~Says() { delete _says; }

      virtual const ExpressionList* saysParams() const 
      { return _says; }

      virtual ExpressionList* saysArgs() const 
      { return _args; }


      virtual TotalIterator
      totalFind(const Expression *e) const;

      virtual void replace(TotalIterator i, 
                           Expression *e);

    private:
      ExpressionList* _says;
  
    };
    
    class Assign : public Term {
    public:
      Assign(Expression *v, 
             Expression *a) 
        : _variable(dynamic_cast<Variable*>(v)), _assign(a) {assert(v); assert(a); }
    
      Assign(const Assign &a) {
	_assign = a._assign->copy();
	_variable = new Variable(*a._variable);
     };


      virtual Term* copy() const{
	Assign *v = new Assign(*this);
	return v;
      }
	
      virtual ~Assign() { delete _variable; delete _assign; }
    
      virtual TuplePtr materialize(CommonTable::ManagerPtr, ValuePtr, string, uint32_t);
    
      virtual string toString() const;
    
      virtual const Variable* variable() const 
      { return _variable; }
    
      virtual const Expression* assignment() const 
      { return _assign; }
    
    private:
      Variable*   _variable;
      Expression* _assign;
    };
    
    class Select : public Term {
    public:
      Select(Expression *s) 
      : _select(dynamic_cast<Bool*>(s)) { };

      Select(const Select &s){
	_select = new Bool(*s._select);
      };

      virtual Term* copy() const{
	Select *v = new Select(*this);
	return v;
      }

      virtual ~Select() { delete _select; }
    
      virtual string toString() const;
    
      virtual TuplePtr materialize(CommonTable::ManagerPtr, ValuePtr, string, uint32_t);
    
      virtual const Bool* select() const
      { return _select; }
    
    private:
      Bool* _select;
    };
    
    /***************************************************
     * STATEMENTS
     ***************************************************/

    class Namespace : public Statement {
    public:
      Namespace(string, StatementList*);

      virtual Statement* copy() const{
	Namespace *v = new Namespace(*this);
	return v;
      }

      virtual StatementList* statements(){
	return _statements;
      }
 
      virtual string toString() const;
    
      virtual TuplePtr materialize(CommonTable::ManagerPtr, ValuePtr, string);

    private:
      string         _name;
      StatementList* _statements;
    }; 
    typedef std::deque<Namespace*> NamespaceList;

    typedef std::set<TableEntry*, lttbl> TableSet;

    class Rule : public Statement {
    public:
      
      static uint32_t ruleId;

      Rule(Term *t, TermList *rhs, bool deleteFlag, Expression *n=NULL); 
      
      Rule(const Rule &r):_name(r._name), _delete(r._delete){
	_head = dynamic_cast<Functor*>(r._head->copy());
	TermList *tl = new TermList();
	TermList::iterator it = r._body->begin();
	while (it != r._body->end()){
	  tl->push_back((*it)->copy());
	  it++;
	}
	_body = tl;
	_new = r._new;
      };
      
      virtual Statement* copy() const{
	Rule *v = new Rule(*this);
	return v;
      }

      virtual ~Rule() { delete _head; delete _body; };
    
      void makeNew() { 
	_new = true; 
      }

      virtual string toString() const;
    
      virtual TuplePtr materialize(CommonTable::ManagerPtr, ValuePtr, string);
    
      virtual string name() const
      { return _name; }
    
      virtual bool deleteRule() const
      { return _delete; }
      
      virtual const Functor* head() const
      { return _head; }
    
      virtual const TermList* body() const
      { return _body; }

      virtual void canonicalizeRule();

    private:


      void canonicalizeAttributes(Functor*, TermList*, bool);

      string                    _name;
      bool                      _delete;
      Functor*                  _head;
      TermList* _body; 
      bool _new;
    };
    typedef std::deque<Rule*> RuleList;

    class Index : public Statement {
    public:
      Index(Expression *name, Expression *type, ExpressionList *keys);

      virtual string toString() const;
    
      virtual TuplePtr materialize(CommonTable::ManagerPtr, ValuePtr, string);

      virtual Statement* copy() const{
	Index *v = new Index(*this);
	return v;
      }

    private:
      string      _name;
      string      _type;
      Table2::Key _keys;
    };
    
    /* The meta-data of the table */
    class Table : public Statement {
    public:
      const static bool compoundRewrite;
      static SetPtr materializedSaysTables;

      Table(Expression *name, 
            Expression *ttl, 
            Expression *size, 
            ExpressionList *keys, 
	    bool says = false,
	    bool versioned = true);
    
      virtual ~Table() {};
    
      virtual Statement* copy() const{
	Table *v = new Table(*this);
	return v;
      }
     
      virtual string toString() const;
    
      virtual TuplePtr materialize(CommonTable::ManagerPtr, ValuePtr, string);
    
      virtual string name() const 
      { return _name; }
    
      virtual boost::posix_time::time_duration lifetime() const
	{ return _lifetime; }
    
      virtual uint32_t size() const
      { return _size; }

      virtual bool says() const{
	return _says;
      }
    
      virtual Table2::Key primaryKeys() const
      { return _keys; }

      virtual void initialize();

    private:
      string                           _name;
      boost::posix_time::time_duration _lifetime;
      int64_t                          _size;
      Table2::Key                      _keys;
      bool                             _says;
      bool                             _versioned;
    };
    typedef std::deque<Table*> TableList;


    /* The meta-data of the table */
    class Ref : public Statement {
    public:

      enum RefType{WEAK = 0, STRONG, WEAKSAYS, STRONGSAYS};

      Ref(int refType, 
	  Expression *from, 
	  Expression *to, 
	  Expression *locSpecField);
    
      virtual ~Ref() {};
    
      virtual Statement* copy() const{
	Ref *v = new Ref(*this);
	return v;
      }
     
      virtual string toString() const;
    
      virtual TuplePtr materialize(CommonTable::ManagerPtr, ValuePtr, string);
    
    private:
      string                           _from;
      string                           _to;
      uint32_t                         _locSpecField;
      uint32_t                         _refType;
    };
    
    class Watch : public Statement {
    public:
      Watch(string w,
            string modifiers);
    
      virtual Statement* copy() const{
	Watch *v = new Watch(*this);
	return v;
      }
     
      virtual ~Watch() {}
    
      virtual TuplePtr materialize(CommonTable::ManagerPtr, ValuePtr, string);
    
      virtual string toString() const
      { return "Watch ( " + _watch + " )."; }
    
      virtual string watch() const
      { return _watch; }
    
    private:
      /** The name of the watched tuple */
      string _watch;

      /** The watch modifiers for the tuple */
      string _modifiers;
    };
    typedef std::deque<Watch*> WatchList;
    
    class Stage : public Statement {
    public:
      Stage(string processorName,
            string inputName,
            string outputName);

      virtual Statement* copy() const{
	Stage *v = new Stage(*this);
	return v;
      }
    
      virtual ~Stage() {}
    
      virtual TuplePtr materialize(CommonTable::ManagerPtr, ValuePtr, string);
    
      virtual string toString() const { 
        return "Stage ( " + _processor + ", " + _input + ", " + _output + " )."; 
      }
    
    private:
      string _processor;
      string _input;
      string _output;
    };
    typedef std::deque<Stage*> StageList;
    
    class Fact : public Statement {
    public:
      Fact(Expression *n, 
           ExpressionList *a); 
    
      virtual Statement* copy() const{
	Fact *v = new Fact(*this);
	return v;
      }

      virtual TuplePtr materialize(CommonTable::ManagerPtr, ValuePtr, string);
    
      virtual string toString() const;
      
      virtual TuplePtr tuple() const { return _tuple; }
    
    private:
      TuplePtr _tuple;
    };
    typedef std::deque<Fact*> FactList;
  
    class Context : public compile::Context {
    
    public:
      static const string STAGEVARPREFIX;
      /*******************************************************************/
      Context(string name) 
      : compile::Context(name), lexer(NULL), _statements(NULL) {printOverLog = true;};
  
      Context(TuplePtr args); 

      Context(string name, string prog, bool file=false);

      virtual ~Context();

      const char *class_name() const { return "parse::Context";}
  
      string toString() const;
    
      
      /******************************************************************
       * Interface exported to bison */
  
      void program(StatementList *statements, bool parserCall = true);

      void error(string msg);
    
      OLG_Lexer *lexer;
      
      void testParse(std::istream *str){parse_stream(str);}
    
      DECLARE_PUBLIC_ELEMENT_INITS

    private:
      StatementList *_statements;

      /** Overridden from compile context */
      TuplePtr program(CommonTable::ManagerPtr catalog, TuplePtr program);

      /** Parses the program defined in istream
        * arguement. The parse will add rows to the system tables defined
        * provided by the CommonTable::ManagerPtr (catalog). */
      void parse(CommonTable::ManagerPtr, ValuePtr, std::istream*);

      /** Call bison to parse the given program contained in the istream argument. */
      void parse_stream(std::istream *str);
  
      bool printOverLog;

      DECLARE_PRIVATE_ELEMENT_INITS
	};

/*     class Secure{ */
/*     public: */
/*       /\** */
/*        * Returns a list of tables used in says: for authentication algebra generation */
/*        *\/ */
/*       static void initializeRule(Rule *r, StatementList *s); */
      
/*       static void program(StatementList *s, bool parserCall); */

/*       static TermList* generateAlgebraLT(ExpressionList::iterator start1,  */
/* 					ExpressionList::iterator start2,  */
/* 					TermList *newTerms = NULL); */


/*       static TermList* generateSelectTerms(ExpressionList::iterator start1, ExpressionList::iterator end1,  */
/* 					     ExpressionList::iterator start2, ExpressionList::iterator end2, int o, */
/* 					     TermList *t = NULL); */
     

/*       static TermList* generateEqTerms(ExpressionList::iterator start1, ExpressionList::iterator end1,  */
/* 					 ExpressionList::iterator start2, ExpressionList::iterator end2, */
/* 					 TermList *t = NULL); */

/*       static StatementList* generateMaterialize(); */

/*       // return a list of terms that needs to be added to the rule on converting the  */
/*       // securelog term f into overlog. */
/*       // Also converts f into the appropriate overlog form */
/*       // the last boolean indicates if the list of terms should also include the  */
/*       // constraint that "the verifying principal doesn't have the authority to  */
/*       // generate the proof" */
/*       static void normalizeVerify(Functor* f, int& newVariable); */

/*       // return a list of terms that needs to be added to the rule on converting the  */
/*       // securelog term f into overlog. */
/*       // Also converts f into the appropriate overlog form */
/*       static TermList* normalizeGenerate(Functor* f, int& newVariable); */
            
/*     }; */


  }
}
  
extern int olg_parser_parse(compile::parse::Context *env );
  
#endif /* __PARSE_CONTEXT_H__ */
