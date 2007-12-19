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

#ifndef __P2DL_CONTEXT_IMPL_H__
#define __P2DL_CONTEXT_IMPL_H__

#include <boost/date_time/posix_time/posix_time.hpp>
#include <string>
#include <deque>
#include <iostream>
#include <map>
#include "element.h"
#include "elementRegistry.h"
#include "plumber.h"
#include "compileContext.h"
#include "tuple.h"
#include "value.h"
#include "val_str.h"
#include "val_null.h"

class P2DL_Lexer;

namespace compile {
  namespace p2dl {
  
    class Exception : public compile::Exception {
    public:
      Exception(unsigned l, string m) : _line(l), _message(m) {}
    
      string toString() const { 
        return "P2DL Error: " + _message;
      }

    private:
      unsigned _line;
      string   _message;
    };
  

    /***************************************************
     * EXPRESSIONS
     ***************************************************/
  
    class Expression {
    public:
      virtual ~Expression() {};
  
      virtual string toString() const = 0;
    };
    typedef std::deque<Expression *> ExpressionList;
    
    class Value : public Expression { 
    public:
      Value(::ValuePtr v) : _value(v) { }
    
      ::ValuePtr value() const { return _value; }
 
      string toString() const;
    private:
      ::ValuePtr _value;
    };

    class Tuple : public Expression {
    public:
      Tuple() { _tuple = ::Tuple::mk(); }

      TuplePtr tuple() const { return _tuple; }

      void prepend(Expression *e) {
        Value* v = dynamic_cast<Value*>(e);
        if (v) _tuple->prepend(v->value());
        else throw Exception(0, "Tuple contains values only!");
      }

      string toString() const;
    private:
      TuplePtr _tuple; 
    };

    class Variable : public Expression { 
    public:
      Variable(::ValuePtr v) : _variable(v) { }
    
      string toString() const;
    private:
      ::ValuePtr _variable;
    };

    class Reference : public Expression {
    public:
      Reference(ExpressionList* r) : _reference(r) {}

      ExpressionList* reference() const { return _reference; }

      string toString() const;
    private:
      ExpressionList* _reference;
    };
    
    class Element : public Expression {
    public:
      Element(Expression* n, ValueList* args) { 
        TuplePtr tp = ::Tuple::mk(n->toString());
        for (ValueList::iterator i = args->begin(); i != args->end(); i++)
          tp->append(*i);
        _element = ElementRegistry::mk(n->toString(), tp);
      }

      ::ElementPtr element() const { return _element; }
 
      string toString() const;
    private:
      ::ElementPtr _element;
    };

    class Dataflow : public Expression {
    public:
      Dataflow(Plumber::DataflowPtr d) { _dataflow = d; }

      Plumber::DataflowPtr dataflow() const { return _dataflow; }

      string toString() const;
    private:
      Plumber::DataflowPtr _dataflow;
    };
  
    class Port : public Expression {
    public:
      Port(Expression* e, bool a=false) 
      : _port(e), _addPort(a) {};

      unsigned inPort(ElementSpecPtr e) const;
      unsigned outPort(ElementSpecPtr e) const;
      string toString() const;
    public:
      Expression* _port;
      bool        _addPort;
    };

    class Link : public Expression {
    public:
      Link(Expression* e, Port* iport, Port* oport) 
      : _element(e), _iport(iport), _oport(oport) 
      { assert(_iport && _oport); }
    
      Expression* element()  const { return _element; }
      Port* input_port()  const { return _iport; }
      Port* output_port() const { return _oport; }

      string toString() const;
    private:
      Expression* _element;
      Port*       _iport;
      Port*       _oport;
    };

    /***************************************************
     * STATEMENTS
     ***************************************************/
    class ScopeTable;
  
    class Statement {
    public:
      virtual ~Statement() {};
  
      virtual void commit(ScopeTable&) = 0;

      virtual string toString() const = 0;
    };
    typedef std::deque<Statement *> StatementList;
    
    class Graph : public Statement {
    public:
      Graph(Expression *n, Expression *i, Expression *o, 
            Expression *p, Expression *f, StatementList* s); 
  
      void commit(ScopeTable&);

      string toString() const;

      Plumber::DataflowPtr dataflow() const{ return _dataflow; }
    private:
      Plumber::DataflowPtr _dataflow;
      StatementList*       _statements;
    };

    class Edit : public Statement {
    public:
      Edit(Expression *n, StatementList* s);

      void commit(ScopeTable&);

      string toString() const;

      Plumber::DataflowPtr dataflow() const { return _dataflow; }
    private:
      Plumber::DataflowPtr _dataflow;
      StatementList*       _statements;
    };

    class Assign : public Statement {
    public:
      Assign(Expression* v, Expression* a) 
      : _variable(v->toString()), _assignment(a) {} 
    
      void commit(ScopeTable&);
 
      string toString() const;
    private:
      string      _variable;
      Expression* _assignment;
    };
    
    class Strand : public Statement {
    public:
      Strand(ExpressionList* s) : _strand(s) {}
    
      void commit(ScopeTable&);

      string toString() const;
    protected:
      virtual ::ElementSpecPtr resolve(ScopeTable&, Expression*);

      ExpressionList* _strand;
    };

    class EditStrand : public Strand {
    public:
      EditStrand(ExpressionList* s) : Strand(s) {}
    private:
      ::ElementSpecPtr resolve(ScopeTable&, Expression*);
    };
    
    class Watch : public Statement {
    public:
      Watch(Expression* w) : _watch(w->toString()) {}
    
      void commit(ScopeTable&);

      string toString() const;
    private:
      string _watch; 
    };
    
    class Fact : public Statement {
    public:
      Fact(ValueList *v);
    
      void commit(ScopeTable&);

      string toString() const;
    private:
      string _tablename;
      TuplePtr _fact;
    };
  
    /***************************
     * ScopeTable definition
     **************************/
    class ScopeTable {
    public:
      ScopeTable() {
        _variables.push_front(std::map<string, Expression*>()); 
        _dataflows.push_front(Plumber::DataflowPtr());
      };

      string
      toString() const {
        std::deque<std::deque<string> > stack;
        if (stackTrace(stack) > 0) {
          ostringstream oss;
          for (std::deque<std::deque<string> >::iterator level = stack.begin();
               level != stack.end(); level++) {
            oss << "< ";
            for (std::deque<string>::iterator ref = level->begin();
               ref != level->end(); ref++) {
              oss << *ref << " ";
            }
            oss << ">\n";
          } 
          return oss.str(); 
        }
        return "ScopeTable empty."; 
      }

      void enter(Plumber::DataflowPtr d) { 
        _variables.push_front(std::map<string, Expression*>()); 
        _dataflows.push_front(d);
      }

      void exit() { 
        if (_variables.size() > 0) {
          _variables.pop_front(); _dataflows.pop_front(); }
        else throw Exception(0, "Scope table underflow"); 
      }

      void map(string v, Expression *e) {
        if (_variables.size() > 0) {
          _variables.front().insert(std::make_pair(v, e));
        } else throw Exception(0, "No scope entered!!"); 
      }

      void unmap(string v) {
        for (std::deque<std::map<string, Expression*> >::iterator i1 = _variables.begin();
             i1 != _variables.end(); i1++) {
          std::map<string, Expression*>::iterator i2 = i1->find(v);
          if (i2 != i1->end()) {
            i1->erase(i2); 
            return;
          }
        }
      }

      unsigned
      stackTrace(std::deque<std::deque<string> >& stack) const {
        unsigned references = 0;
        stack.clear();
        for (std::deque<std::map<string, Expression*> >::const_iterator i1 = _variables.begin();
             i1 != _variables.end(); i1++) {
          std::deque<string> level;
          for (std::map<string, Expression*>::const_iterator i2 = i1->begin();
               i2 != i1->end(); i2++) {
            level.push_back(i2->first); 
            references++;
          }
          stack.push_back(level);
        }
        return references;
      }

      Expression* lookup(string v) const {
        for (std::deque<std::map<string, Expression*> >::const_iterator i = _variables.begin();
             i != _variables.end(); i++) {
          std::map<string, Expression*>::const_iterator i2 = i->find(v);
          if (i2 != i->end()) return i2->second;
        }
        return NULL;
      }

      Plumber::DataflowPtr 
      dataflow() const { return _dataflows.front(); }

    private:
      std::deque<std::map<string, Expression*> > _variables;
      std::deque<Plumber::DataflowPtr>           _dataflows;
    };
    
    /***************************
     * p2dl::Context definition
     **************************/
    class Context : public compile::Context {
    public:
      Context(string name); 
  
      Context(string name, string prog, bool file=false);

      Context(TuplePtr args);
  
      virtual ~Context() {};
  
      const char *class_name() const { return "p2dl::Context";}
  
      string toString() const;
    
      
      /******************************************************************
       * Bison interface */

      /** Install the current compiled graphs, tables, facts, and watches */
      void commit();
  
      void graph(Graph *g)     { _graphs.push_back(g); }

      void edit(Edit *e)       { _edits.push_back(e); }

      void fact(Fact *f)       { _facts.push_back(f); }
    
      void watch(Watch *w)     { _watches.push_back(w); }
    
      void error(string msg) {}

      P2DL_Lexer *lexer;

      DECLARE_PUBLIC_ELEMENT_INITS


    private:
      /** Overridden from compile context */
      TuplePtr program(CommonTable::ManagerPtr catalog, TuplePtr program);

      void rule(CommonTable::ManagerPtr catalog, TuplePtr rule);

      void parse_stream(std::istream *str);
  
      StatementList _graphs;

      StatementList _edits;

      StatementList _tables;
    
      StatementList _watches;
    
      StatementList _facts;

      DECLARE_PRIVATE_ELEMENT_INITS
    };
  }
}
  
extern int p2dl_parser_parse(compile::p2dl::Context *env );
  
#endif /* __P2DL_CONTEXT_H__ */
