/*
 * @(#)$Id$
 *
 * Copyright (c) 2005 Intel Corporation. All rights reserved.
 *
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 * DESCRIPTION: Parsing environment for Overlog (the P2 dialect of datalog)
 *
 */

#ifndef __OL_PARSEENV_H__
#define __OL_PARSEENV_H__

#include <async.h>
#include <list>
#include <map>
#include "value.h"

class OL_Lexer;
#include "parser_stuff.h"

class OL_Context {

public:
  
  struct Functor;
  
  struct Arg {
    int    var;	// Position, or -1
    ValuePtr val;	// Value, if var == -1
    Arg(u_int v) : var(v) {};
    Arg(ValuePtr v): var(-1), val(v) {} ;
  };

  struct Term {
    Functor *fn;		// The actual functor this refers to. 
    int location;		// The 'location' of this eval, in the
				// positional list of the owning
				// Rule. 
    std::vector<Arg> args;	// functor->arity args. 
  };

  struct Rule {
    std::vector<str> args; // List of free variables in the rule.
    std::vector<Term> terms; // List of terms.

    str toString();	       // For debugging...
    int add_arg(str varname);  // Add a new var and return its position
    int find_arg(str varname); // Look up a var and ret. pos or -1
  };
  
  struct Functor {
    str name;		// The name of the functor or table
    u_int arity;	// The arity
    bool defined;	// Has it been defined yet?
    std::vector<Rule *> rules;	// List of associated rules
    
    Functor(str n, u_int a) : name(n), arity(a) {};
    static str calc_key(str name, u_int arity) {
      return strbuf() << name << "/" << arity;
    };
    str key() const { return calc_key(name, arity); }
  };

  /*******************************************************************/
  OL_Context();

  //
  // Parsing programs
  //
  void parse_string( const char *prog );
  void parse_stream( std::istream *str );

  /////
  // 
  // Parser interface: functions called from the parser to create
  // state. 
  //
  //////

  //
  // Add a new rule to the system
  void add_rule( Parse_Term *lhs, Parse_TermList *rhs );

  // Materialize a table
  void materialize( Parse_ExprList *l);

  void add_fact( Parse_Term *t);

  void error(str msg);

  OL_Lexer *lexer;

  str OL_Context::toString();
  
private:
  
  typedef std::map<str, OL_Context::Functor *> FunctorMap;
  FunctorMap functors;

  Functor *retrieve_functor( str name, int arity, 
					  bool create=false,
					  bool define=false );
  
  
};

extern int ol_parser_parse( OL_Context *env );

#endif /* __OL_PARSEENV_H_ */
