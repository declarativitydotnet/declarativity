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
#include <iostream>
#include <map>
#include "compileContext.h"
#include "element.h"
#include "elementRegistry.h"
#include "tuple.h"
#include "value.h"
#include "val_str.h"

class OL_Lexer;

namespace compile {
  namespace parse {

    class Exception : public compile::Exception {
    public:
      Exception(uint line, string msg) 
      : lineNumber_(line), message_(msg) {}
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
  
      virtual const ValuePtr value() const
      { throw Exception(0, "Expression does not have a value."); }
  
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
      virtual ~Term() {};
  
      virtual string toString() const = 0;
  
      virtual TuplePtr materialize(CommonTable::ManagerPtr, ValuePtr, string) = 0;
    };
    typedef std::deque<Term *> TermList;
    typedef std::deque<TermList *> TermListList;
    
    class Statement {
    public:
      virtual ~Statement() {};
  
      virtual string toString() const = 0;
  
      virtual TuplePtr materialize(CommonTable::ManagerPtr, ValuePtr, string) = 0;
    };
    typedef std::deque<Statement *> StatementList;
  
    class Value : public Expression { 
    public:
      Value(ValuePtr val) : _value(val) {};
    
      virtual string toString() const 
      { return _value->toString(); };
  
      virtual ValuePtr tuple() const;
  
      virtual const ValuePtr value() const { return _value; }
    private:
      ValuePtr _value;
    };
    
    class Variable : public Expression { 
    public:
      Variable(ValuePtr var, bool l=false) 
      : _value(var), _location(l) {};
    
      Variable(const string& var, bool l=false) 
      : _value(Val_Str::mk(var)), _location(l)  {};
  
      virtual string toString() const 
      { return (_location ? "@" : "") + _value->toString(); };
    
      virtual bool location() const { return _location; }
    
      virtual ValuePtr tuple() const;
    private:
      ValuePtr _value;
      bool     _location;
    
    };
    
    class Aggregation : public Expression {
    public:
      Aggregation(string o, Expression *v);
    
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
        : _lhs(l), _rhs(r) { 
        switch(o) {
          case RANGEOO: _operName = "()"; break;
          case RANGEOC: _operName = "(]"; break; 
          case RANGECO: _operName = "[)"; break; 
          case RANGECC: _operName = "[]"; break; 
        }
      };
    
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
    
    class Math : public Expression {
    public:
      enum Operator {LSHIFT, RSHIFT, PLUS, MINUS, TIMES, DIVIDE, MODULUS, NOP};
    
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
          default: assert(0);
        }
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
    
      virtual string toString() const 
      { return _vector->toString(); };
    
      virtual ValuePtr tuple() const;
    private:
      ValuePtr _vector;
    };
    
    class Matrix : public Expression {
    public:
      Matrix(ExpressionListList *r);
    
      virtual string toString() const
      { return _matrix->toString(); };
    
      virtual ValuePtr tuple() const;
    private:
      ValuePtr _matrix;
    };
    
    class Function : public Expression {
    public:
      Function(Expression *n, ExpressionList *a) 
        : _name(n->toString()), _args(a) { };
    
      virtual ~Function() { delete _args; };
    
      virtual string toString() const;
    
      virtual const ExpressionList* arguments() const
      { return _args; };
    
      virtual ValuePtr tuple() const;
    private:
      string          _name;
      ExpressionList* _args;
    };
    
    /***************************************************
     * TERMS
     ***************************************************/
    
    class Functor : public Term {
    public:
      Functor(Expression *n, ExpressionList *a); 
     
      virtual ~Functor() { delete _args; }
    
      virtual string toString() const;
    
      virtual TuplePtr materialize(CommonTable::ManagerPtr, ValuePtr, string);
    
      virtual string name() const { return _name; }
    
      virtual const ExpressionList* arguments() const 
      { return _args; }
    
      virtual const Aggregation* aggregate() const;
    
      virtual ExpressionList::iterator
      find(const Expression *e) const;
    
      virtual void replace(ExpressionList::iterator i, 
                           Expression *e);
    
    private:
      string          _name;
      ExpressionList* _args;
    };
    
    class Assign : public Term {
    public:
      Assign(Expression *v, 
             Expression *a) 
        : _variable(dynamic_cast<Variable*>(v)), _assign(a) { }
    
      virtual ~Assign() { delete _variable; delete _assign; }
    
      virtual TuplePtr materialize(CommonTable::ManagerPtr, ValuePtr, string);
    
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
      virtual ~Select() { delete _select; }
    
      virtual string toString() const;
    
      virtual TuplePtr materialize(CommonTable::ManagerPtr, ValuePtr, string);
    
      virtual const Bool* select() const
      { return _select; }
    
    private:
      Bool* _select;
    };
    
    class AggregationView : public Term {
    public: 
      AggregationView(string aggName,
                      ExpressionList *gb, 
                      ExpressionList *agg, 
                      Term *bt)
        : _groupBy(gb),
          _agg(agg),
          _operName(aggName)
      {
        _baseTable = dynamic_cast<Functor*>(bt);
      }
    
      virtual string toString() const;
    
      virtual TuplePtr materialize(CommonTable::ManagerPtr, ValuePtr, string);
    
      virtual string operName() const 
      { return _operName; }
    
      virtual const Functor* baseTable() const 
      { return _baseTable; }
    
      virtual const ExpressionList* groupBy() const
      { return _groupBy; }
    
      virtual const ExpressionList* aggregates() const
      { return _agg; }
    
    private:
      ExpressionList* _groupBy; 
      ExpressionList* _agg;
      Functor*                        _baseTable;
      string                          _operName;
    };
    
    /***************************************************
     * STATEMENTS
     ***************************************************/

    class Namespace : public Statement {
    public:
      Namespace(string, StatementList*);

      virtual string toString() const;
    
      virtual TuplePtr materialize(CommonTable::ManagerPtr, ValuePtr, string);

    private:
      string         _name;
      StatementList* _statements;
    }; 
    typedef std::deque<Namespace*> NamespaceList;

    class Rule : public Statement {
    public:
      Rule(Term *t, TermList *rhs, bool deleteFlag, Expression *n=NULL); 
    
      virtual ~Rule() { delete _head; delete _body; };
    
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
    
    private:
      void canonicalizeAttributes(Functor*, TermList*, bool);

      string                    _name;
      bool                      _delete;
      Functor*                  _head;
      TermList* _body; 
    };
    typedef std::deque<Rule*> RuleList;
    
    /* The meta-data of the table */
    class Table : public Statement {
    public:
      Table(Expression *name, 
            Expression *ttl, 
            Expression *size, 
            ExpressionList *keys);
    
      virtual ~Table() {};
    
      virtual string toString() const;
    
      virtual TuplePtr materialize(CommonTable::ManagerPtr, ValuePtr, string);
    
      virtual string name() const 
      { return _name; }
    
      virtual boost::posix_time::time_duration lifetime() const
      { return _lifetime; }
    
      virtual uint32_t size() const
      { return _size; }
    
      virtual Table2::Key primaryKeys() const
      { return _keys; }
    
    private:
      string                           _name;
      boost::posix_time::time_duration _lifetime;
      int64_t                          _size;
      Table2::Key                      _keys;
    };
    typedef std::deque<Table*> TableList;
    
    class Watch : public Statement {
    public:
      Watch(string w,
            string modifiers);
    
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
    
      virtual TuplePtr materialize(CommonTable::ManagerPtr, ValuePtr, string);
    
      virtual string toString() const;
      
      virtual TuplePtr tuple() const { return _tuple; }
    
    private:
      TuplePtr _tuple;
    };
    typedef std::deque<Fact*> FactList;
  
    
    class Context : public compile::Context {
    
    public:
      /*******************************************************************/
      Context(string name) 
      : compile::Context(name), lexer(NULL), _statements(NULL) {};
  
      Context(TuplePtr args); 

      Context(string name, string prog, bool file=false);

      virtual ~Context();

      const char *class_name() const { return "parse::Context";}
  
      string toString() const;
    
      
      /******************************************************************
       * Interface exported to bison */
  
      void program(StatementList *statements);

      void error(string msg);
    
      OL_Lexer *lexer;
    
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
  

      DECLARE_PRIVATE_ELEMENT_INITS
    };
  }
}
  
extern int ol_parser_parse(compile::parse::Context *env );
  
#endif /* __PARSE_CONTEXT_H__ */
