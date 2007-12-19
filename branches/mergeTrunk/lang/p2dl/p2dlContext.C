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
#include "p2dlContext.h"
#include "p2dl_lexer.h"
#include "reporting.h"
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
#include "identity.h"


namespace compile {
  namespace p2dl {
    using namespace opr;

    /***************************************************
     * EXPRESSIONS
     ***************************************************/
    
    string Value::toString() const {
      return _value->toString();
    }

    string Tuple::toString() const {
      return _tuple->toString();
    }

    string Variable::toString() const {
      return _variable->toString();
    }

    string Reference::toString() const {
      string ref = "";
      for (ExpressionList::iterator i = _reference->begin(); 
           i != _reference->end(); i++) {
        ref += (*i)->toString();
        if (i + 1 != _reference->end()) ref += ".";
      }
      return ref;
    }
    
    string Element::toString() const {
      return "Element " + _element->name();
    }

    string Dataflow::toString() const {
      return "Dataflow " + _dataflow->name();
    }

    string 
    Port::toString() const {
      ostringstream oss;
      if (_addPort) oss << "+";
      if (_port) oss << _port->toString();
      return oss.str();
    }

    unsigned 
    Port::inPort(ElementSpecPtr e) const {
      Value* v = _port ? dynamic_cast<Value*>(_port) : NULL;
      if (_addPort) {
        if (v) return e->add_input(v->value());
        else   return e->add_input();
      }
      else {
        if (!v) {
          throw p2dl::Exception(0, "Bad port value! " +
                                _port->toString());
        }

        if (v->value()->typeCode() == ::Value::INT64) {
          return Val_Int64::cast(v->value());
        } else {
          return e->element()->input(v->value());
        }
      }
      assert(0);
      return 0;
    }

    unsigned 
    Port::outPort(ElementSpecPtr e) const {
      Value* v = _port ? dynamic_cast<Value*>(_port) : NULL;
      if (_addPort) {
        if (v) return e->add_output(v->value());
        else   return e->add_output();
      }
      else {
        if (!v) {
          throw p2dl::Exception(0, "Bad port value! " +
                                _port->toString());
        }
        
        if (v->value()->typeCode() == ::Value::INT64) {
          return Val_Int64::cast(v->value());
        } else {
          return e->element()->output(v->value());
        }
      }
      assert(0);
      return 0;
    }
    
    string Link::toString() const {
      ostringstream oss;
      oss << "[ " << (_iport ? _iport->toString() : "-") << " ]" << _element->toString() 
          << "[ " << (_oport ? _oport->toString() : "-") << " ]";
      return oss.str();
    }

    /***************************************************
     * STATEMENTS
     ***************************************************/
  
    Graph::Graph(Expression *n, Expression *i, Expression *o, 
                 Expression *p, Expression *f, StatementList* s) 
    {
      Value *inputs      = dynamic_cast<Value*>(i);
      Value *outputs     = dynamic_cast<Value*>(o);
      Value *proccessing = dynamic_cast<Value*>(p);
      Value *flow_code   = dynamic_cast<Value*>(f);

      _dataflow.reset(
        new Plumber::Dataflow(n->toString(), 
                              Val_Int64::cast(inputs->value()),
                              Val_Int64::cast(outputs->value()),
                              Val_Str::cast(proccessing->value()),
                              Val_Str::cast(flow_code->value())));
      _statements = s;
    }

    void 
    Graph::commit(ScopeTable& scope) 
    {
      scope.enter(_dataflow);
      for (StatementList::iterator i = _statements->begin();
           i != _statements->end(); i++) {
        (*i)->commit(scope);
      }
      scope.exit();

      if (!_dataflow->ninputs() && !_dataflow->noutputs()) {
        /* No inputs or outputs, install now! */
        Plumber::install(_dataflow);
      }
      else {
        /* Has inputs/outputs, must be referenced in an edit.
           Store reference in the scope table for later lookup. */
        scope.map(_dataflow->name(), new Dataflow(_dataflow));
      }
    }

    string 
    Graph::toString() const 
    {
      return "Graph " + _dataflow->name();
    }
    
    Edit::Edit(Expression *n, StatementList* s) 
    {
      if (Plumber::dataflow(n->toString())) {
        _dataflow = Plumber::edit(n->toString());
      }
      else {
        throw Exception(0, "Edit: Unknown dataflow reference. " + n->toString());
      }
      _statements = s;
    }

    void 
    Edit::commit(ScopeTable& scope) 
    {
      scope.enter(_dataflow);
      for (StatementList::iterator i = _statements->begin();
           i != _statements->end(); i++) {
        (*i)->commit(scope);
      }
      scope.exit();
      if (Plumber::install(_dataflow) < 0) {
        TELL_ERROR << "EDIT INSTALL FAILURE" << std::endl;
        throw Exception(0, "Dataflow edit installation error: " + _dataflow->name());
      }
    }

    string 
    Edit::toString() const 
    {
      return "Edit";
    }

    void 
    Assign::commit(ScopeTable& scope) 
    {
      scope.map(_variable, _assignment);
    }

    string Assign::toString() const {
      return "";
    }

    void 
    Strand::commit(ScopeTable& scope) 
    {
      Plumber::DataflowPtr dataflow = scope.dataflow();

      ExpressionList::iterator first  = _strand->begin();
      ExpressionList::iterator second = first + 1;
      while (second != _strand->end()) {
        Link* flink = dynamic_cast<Link*>(*first);
        Link* slink = dynamic_cast<Link*>(*second);
        assert (flink && slink);
        ::ElementSpecPtr felement = resolve(scope, flink->element());
        ::ElementSpecPtr selement = resolve(scope, slink->element());
        if (!felement && !selement) {
          ::ElementSpecPtr identity = dataflow->addElement(ElementPtr(new Identity()));
          dataflow->hookUp(felement, flink->output_port()->outPort(felement),
                           identity, 0);
          dataflow->hookUp(identity, 0,
                           selement, slink->input_port()->inPort(selement));
        }
        else {
          dataflow->hookUp(felement, flink->output_port()->outPort(felement),
                           selement, slink->input_port()->inPort(selement));
        }
        first++; second++;
      }
    }

    string Strand::toString() const {
      return "";
    }

    ::ElementSpecPtr
    Strand::resolve(ScopeTable& scope, Expression *expr)
    {
      Variable *v = NULL;
      Element  *e = NULL;
      Dataflow *d = NULL;
      if ((v = dynamic_cast<Variable*>(expr)) != NULL) {
        if (v->toString() == "input" || v->toString() == "output")
          return ::ElementSpecPtr(); // Special case
        expr = scope.lookup(v->toString());
        if (!expr) {
          throw Exception(0, "Strand: Unknown variable reference. " + v->toString());
        } else if ((e = dynamic_cast<Element*>(expr)) != NULL) {
          return scope.dataflow()->addElement(e->element());
        }
        else return resolve(scope, expr);
      }
      else if ((e = dynamic_cast<Element*>(expr)) != NULL) {
        return scope.dataflow()->addElement(e->element());
      }
      else if ((d = dynamic_cast<Dataflow*>(expr)) != NULL) {
        return scope.dataflow()->addElement(d->dataflow());
      }
      else throw Exception(0, "Strand element not of type Element!"); 
      return ::ElementSpecPtr();
    }

    ::ElementSpecPtr 
    EditStrand::resolve(ScopeTable& scope, Expression *expr) {
      Reference* ref = dynamic_cast<Reference*>(expr);
      if (ref != NULL) {
        ExpressionList*    reflist  = ref->reference();
        Plumber::Dataflow* dataflow = NULL;

        if (reflist->size() == 1) {
          /* Normal resolve will handle single level variable resolution */
          return Strand::resolve(scope, reflist->front());
        }
        else if (reflist->size() != 2)
          throw Exception(1, "EditStrand: Only single level reference name allowed for now.");

        Plumber::DataflowPtr d = scope.dataflow();
        if (d->name() != reflist->front()->toString()) {
          throw Exception(1, "EditStrand: top level reference name \
                              must refer to dataflow edit name." + 
                             ref->toString());
        }
        dataflow = d.get();
        
        ::ElementSpecPtr esp;
        for (ExpressionList::iterator i = reflist->begin() + 1;
             i != reflist->end(); i++) {
          if (dataflow == NULL) 
            throw Exception(2, "EditStrand: Unknown variable reference. " + ref->toString());
          esp = dataflow->find((*i)->toString());
          if (!esp) {
            std::cerr << "EditStrand: Unknown variable reference. " << ref->toString() << std::endl;
            throw Exception(3, "EditStrand: Unknown variable reference. " + ref->toString());
          }
          dataflow = dynamic_cast<Plumber::Dataflow*>(esp->element().get()); 
        }

        if (!esp) {
          throw Exception(4, "EditStrand: Unknown variable reference. " + ref->toString());
        }
        return esp;
      }
      else {
        return Strand::resolve(scope, expr);
      }
      return ElementSpecPtr();
    }

    void Watch::commit(ScopeTable& scope) {

    }

    string Watch::toString() const {
      return "";
    }

    Fact::Fact(ValueList *v) {
      _fact = ::Tuple::mk();
      for (ValueList::iterator i = v->begin(); i != v->end(); i++)
        _fact->append(*i); 
      _tablename = (*_fact)[TNAME]->toString();
    }

    void Fact::commit(ScopeTable& scope) {
      CommonTablePtr table = Plumber::catalog()->table(_tablename);
      if (table) {
        table->insert(_fact);
      }
      else {
        TELL_ERROR << "P2DL ERROR: fact table " << _tablename << " does not exist!";
        throw compile::p2dl::Exception(0, "FACT TABLE DOES NOT EXIST!");
      }
    }

    string Fact::toString() const {
      return _fact->toString();
    }
    
    /***************************************************
     * P2DL CONTEXT
     ***************************************************/

    DEFINE_ELEMENT_INITS_NS(Context, "P2DLContext", compile::p2dl)

    Context::Context(string name)
    : compile::Context(name), lexer(NULL) { }
    
    Context::Context(string name, string prog, bool file)
    : compile::Context(name), lexer(NULL)
    {
      if (file) {
        string processed(prog+".processed");
  
        // Run the OverLog through the preprocessor
        pid_t pid = fork();
        if (pid == -1) {
          TELL_ERROR << "Cannot fork a preprocessor\n";
          exit(-1);
        } else if (pid == 0) {
          // I am the preprocessor
          execlp("cpp", "cpp", "-P", prog.c_str(), processed.c_str(),
                 (char*) NULL);
          // If I'm here, I failed
          TELL_ERROR << "Preprocessor execution failed" << std::endl;;
          exit(-1);
        } else {
          // I am the child
          wait(NULL);
        }
  
        // Parse the preprocessed file
        std::ifstream prog(processed.c_str());
        parse_stream(&prog);
        unlink(processed.c_str());
      }
      else {
        std::istringstream p2dl(prog, std::istringstream::in);
        parse_stream(&p2dl);
      }
    }

    Context::Context(TuplePtr args)
    : compile::Context((*args)[2]->toString()), lexer(NULL)
    {
      if (args->size() > 3) {
        std::istringstream p2dl((*args)[3]->toString(), std::istringstream::in);
        parse_stream(&p2dl);
      }
    }

    string 
    Context::toString() const 
    {
      return "compile::p2dl::Context";
    }
  
    void 
    Context::commit()
    {
      ScopeTable scope;
      TELL_INFO << "COMMIT TABLES" << std::endl;
      for (StatementList::iterator i = _tables.begin();
           i != _tables.end();
           i++) {
        TELL_WORDY << (*i)->toString() << std::endl;
        (*i)->commit(scope);
      }
      TELL_INFO << "COMMIT WATCHES" << std::endl;
      for (StatementList::iterator i = _watches.begin();
           i != _watches.end();
           i++) {
        TELL_WORDY << (*i)->toString() << std::endl;
        (*i)->commit(scope);
      }
      TELL_INFO << "COMMIT GRAPHS" << std::endl;
      for (StatementList::iterator i = _graphs.begin();
           i != _graphs.end();
           i++) {
        TELL_WORDY << (*i)->toString() << std::endl;
        (*i)->commit(scope);
      }
      TELL_INFO << "COMMIT EDITS" << std::endl;
      for (StatementList::iterator i = _edits.begin();
           i != _edits.end();
           i++) {
        (*i)->commit(scope);
      }
      TELL_INFO << "COMMIT FACTS" << std::endl;
      for (StatementList::iterator i = _facts.begin();
           i != _facts.end();
           i++) {
        (*i)->commit(scope);
      }
    }

    TuplePtr 
    Context::program(CommonTable::ManagerPtr catalog, TuplePtr program)
    {
      program = this->compile::Context::program(catalog, program);

      if (program && (*program)[catalog->attribute(PROGRAM, "P2DL")] != Val_Null::mk()) {
        ValuePtr programP2DL = (*program)[catalog->attribute(PROGRAM, "P2DL")];
        if (programP2DL != Val_Null::mk()) {
          // std::cerr << programP2DL->toString() << std::endl;
          std::istringstream p2dl(programP2DL->toString(), std::istringstream::in);
          parse_stream(&p2dl);
        }
      }
      return program;
    }

    void
    Context::rule(CommonTable::ManagerPtr catalog, TuplePtr rule)
    {
      ValuePtr ruleText = (*rule)[catalog->attribute(RULE, "P2DL")];

      if (ruleText != Val_Null::mk()) {

        //        if ((*rule)[catalog->attribute(RULE, "NAME")]->toString() == "rule_aggview_rule_mv_r1") { 
        //std::cerr << "RULE P2DL TEXT: " << std::endl << ruleText->toString() << std::endl;
        //}

        std::istringstream p2dl(ruleText->toString(), std::istringstream::in);
        parse_stream(&p2dl);
      }
    }

    void Context::parse_stream(std::istream *str)
    { 
      try {
        assert(lexer==NULL);
        lexer = new P2DL_Lexer(str);
        _graphs.clear();
        _edits.clear();
        _tables.clear();
        _watches.clear();
        _facts.clear(); 
        p2dl_parser_parse(this);
        delete lexer; 
        lexer = NULL;
      }
      catch (compile::Exception e) {
        TELL_ERROR << "compile::Exception: " << e.toString() << std::endl;
        throw compile::p2dl::Exception(lexer->line_num(), e.toString());
      }
    }
  }
}
