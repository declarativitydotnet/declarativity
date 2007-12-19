/*
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

#ifndef __PARSER_UTIL_C__
#define __PARSER_UTIL_C__

#include "parser_util.h"
#include "ol_context.h"
#include "val_int64.h"
#include "oper.h"

using namespace opr;


Parse_Var::Parse_Var(ValuePtr var)
  : Parse_Expr(var),
    _locspec(false)
{
}

Parse_Var::Parse_Var(const string& var)
  : Parse_Expr(Val_Str::mk(var)),
    _locspec(false)
{
}
  

string
Parse_Var::toLocString()
{ 
  if (!locspec()) {
    return v->toString(); 
  } else {
    return ("@" + v->toString());
  }
}
  

string
Parse_Var::toString()
{ 
  return v->toString();
}
  

bool
Parse_Var::locspec()
{ 
  return _locspec;
}
  


void
Parse_Var::setLocspec()
{ 
  _locspec = true;
}
  





//=====================================

Parse_Expr* Parse_Agg::DONT_CARE = new Parse_Var(Val_Str::mk("*"));

bool Parse_Agg::operator==(const Parse_Expr &e){
  try {
    const Parse_Agg& a = dynamic_cast<const Parse_Agg&>(e);
    return Parse_Expr::operator==(e) && oper == a.oper;
  }
  catch (std::bad_cast e) {
    return false;
  }
}

string Parse_Agg::toString() {
  ostringstream a;
  a << oper << "< ";
  a << v->toString() << " >";
  return a.str();
}

string
Parse_Agg::aggName() {
  return oper;
}


Parse_Bool::Parse_Bool(Parse_Bool::Operator o, Parse_Expr *l,
                       Parse_Expr *r, bool id) 
  : oper(o), lhs(l), rhs(r), id_(id) {
  // TODO: if (oper != NOT && rhs == NULL) ERROR!
}

bool Parse_Bool::operator==(const Parse_Expr &e) {
  try {
    const Parse_Bool& b = dynamic_cast<const Parse_Bool&>(e);
    return *lhs == *b.lhs && *rhs == *b.rhs && oper == b.oper;
  } catch (std::bad_cast e) {
    return false;
  }
} 

string Parse_Bool::toString() {
  ostringstream b;
  if (oper == NOT) {
    b << "!(" << lhs->toString() << ")";
  }
  else if (oper == RANGE) {
    b << lhs->toString() << " in " << rhs->toString();
  }
  else {
    if (dynamic_cast<Parse_Bool*>(lhs) != NULL) b << "(";
    b << lhs->toString();
    if (dynamic_cast<Parse_Bool*>(lhs) != NULL) b << ")";
    switch (oper) {
      case AND: b << " && "; break;
      case OR:  b << " || "; break;
      case EQ:  b << " == "; break;
      case NEQ: b << " != "; break;
      case GT:  b << " > "; break;
      case LT:  b << " < "; break;
      case LTE: b << " <= "; break;
      case GTE: b << " >= "; break;
      default: assert(0);
    }
    if (dynamic_cast<Parse_Bool*>(rhs) != NULL) b << "(";
    b << rhs->toString();
    if (dynamic_cast<Parse_Bool*>(rhs) != NULL) b << ")";
  }

  return b.str();
}


bool Parse_Range::operator==(const Parse_Expr &e){
  try {
    const Parse_Range& r = dynamic_cast<const Parse_Range&>(e);
    return *lhs == *r.lhs && *rhs == *r.rhs && type == r.type;
  } catch (std::bad_cast e) {
    return false;
  }
}

string Parse_Range::toString() {
  ostringstream r;
  switch (type) {
    case RANGEOO: 
      r << "(" << lhs->toString() << ", " << rhs->toString() << ")"; break;
    case RANGEOC: 
      r << "(" << lhs->toString() << ", " << rhs->toString() << "]"; break;
    case RANGECO: 
      r << "[" << lhs->toString() << ", " << rhs->toString() << ")"; break;
    case RANGECC: 
      r << "[" << lhs->toString() << ", " << rhs->toString() << "]"; break;
  }
  return r.str();
}


ValuePtr Parse_Math::value() {
  Parse_Math *m;
  Parse_Val  *v;
  ValuePtr l;
  ValuePtr r;
  
  if ((m=dynamic_cast<Parse_Math*>(lhs)) != NULL) 
    l = m->value();
  else if ((v=dynamic_cast<Parse_Val*>(lhs)) != NULL) 
    l = v->value();
  else
    return ValuePtr();

  if ((m=dynamic_cast<Parse_Math*>(rhs)) != NULL) 
    r = m->value();
  else if ((v=dynamic_cast<Parse_Val*>(rhs)) != NULL) 
    r = v->value();
  else
    return ValuePtr();	// TODO: throw some kind of exception.

  if (!l || !r) return ValuePtr();

  switch (oper) {
    case LSHIFT:  return l << r;
    case RSHIFT:  return l >> r;
    case PLUS:    return l +  r;
    case MINUS:   return l -  r;
    case TIMES:   return l *  r;
    case DIVIDE:  return l /  r;
    case MODULUS: return l %  r;
    default: assert(0);
  }
}

bool Parse_Math::operator==(const Parse_Expr &e){
  try {
    const Parse_Math& m = dynamic_cast<const Parse_Math&>(e);
    return *lhs == *m.lhs && *rhs == *m.rhs && oper == m.oper;
  }
  catch (std::bad_cast e) {
    return false;
  }
}

string Parse_Math::toString() {
  ostringstream m;
  bool lpar = (dynamic_cast<Parse_Math*>(lhs) != NULL);
  bool rpar = (dynamic_cast<Parse_Math*>(rhs) != NULL);

  if (lpar) m << "(";
  m << lhs->toString(); 
  if (lpar) m << ")";

  switch (oper) {
    case LSHIFT:  m << "<<"; break;
    case RSHIFT:  m << ">>"; break;
    case PLUS:    m << "+"; break;
    case MINUS:   m << "-"; break;
    case TIMES:   m << "*"; break;
    case DIVIDE:  m << "/"; break;
    case MODULUS: m << "%"; break;
    case BIT_AND: m << "&"; break;
    case BIT_OR:  m << "|"; break;
    case BIT_XOR: m << "^"; break;
    case BIT_NOT: m << "~"; break;
    case APPEND: m << "|||"; break;
    default: assert(0);
  }
  m << " ";

  if (rpar) m << "(";
  m << rhs->toString();
  if (rpar) m << ")";

  return m.str();
}

Parse_FunctorName::Parse_FunctorName(Parse_Expr *n)
{
  name = n->v->toString();
  delete n;
}

string Parse_FunctorName::toString() {
  ostringstream fn;
  fn <<  name;
  return fn.str();
}


Parse_Functor::Parse_Functor(Parse_FunctorName* f,
                             Parse_ExprList* a,
                             Parse_Expr* l) 
  : fn(f),
    args_(a)
{ 
  if (l) {
    loc_ = l->v->toString(); delete l;
  } else {
    (void) getlocspec();
  }
}


string
Parse_Functor::getlocspec() {
  Parse_Var *p;
  Parse_Agg *a;
  
  // if the loc_ field was filled in, trust it.  Sometimes this may be
  // done externally to the parser (e.g. during rule localization.)
  // Otherwise, find the locspec among the args.
  if (loc_.empty())  {
    bool found = false;
    for (int k = 0; k < args(); k++) {
      if ((p = dynamic_cast<Parse_Var*>(arg(k)))
          && p->locspec()) {
        if (!found) {
          loc_ = p->toLocString();
          found = true;
        } else {
          TELL_ERROR << "PARSER ERROR: More than one location "
                     << "specifier in predicate " << toString();
          loc_.clear();
          break;
        }
      } else if ((a = dynamic_cast<Parse_Agg*>(arg(k)))
                 && a->locspec) {
        // This argument is an aggregate and also the location
        // specifier
        if (!found) {
          // The name of the locspec is the aggregation variable
          loc_ = "@" + a->v->toString();
          found = true;
        }
        else {
          TELL_ERROR << "PARSER ERROR: More than one location "
                     << "specifier in predicate " << toString();
          loc_.clear();
          break;
        }
      }
    }
    if (!found)
      TELL_WARN << "PARSER WARNING: No location specifier in predicate "
                << toString()
                << "\n";
    // drop through to return
  }
  return(loc_);
}

string
Parse_Functor::toString() {
  ostringstream f;
  f << fn->toString()
    << "(";
  for (int i = 0; i < args(); i++) {
    Parse_Expr* nextArg = arg(i);
    Parse_Var* var = dynamic_cast<Parse_Var*>(nextArg);
    if (var == NULL) {
      // Wasn't a variable, so just call normal toString
      f << nextArg->toString();
    } else {
      // It's a variable, so call toString with locspecs
      f << var->toLocString();
    }
    if (i + 1 < args()) {
      f << ", ";
    } else {
      f << ")";
    }
  }
  return f.str();
}

int Parse_Functor::find(Parse_Expr *arg) {
  return find(arg->v->toString());
}

int Parse_Functor::find(string argname) {
  int p = 0;
  for ( ; p < args() && arg(p)->toString() != argname; p++);
  return (p < args()) ? p : -1;
}

int
Parse_Functor::aggregate() {
  for (int i = 0;
       i < args();
       i++) {
    if (dynamic_cast<Parse_Agg*>(arg(i)) != NULL) {
      return i;
    }
  }
  return -1;
}

void Parse_Functor::replace(int p, Parse_Expr *e) {
  Parse_ExprList::iterator next = args_->erase((args_->begin()+p));
  args_->insert(next, e);
}

string
Parse_Assign::toString()
{
  return var->toString() + " := " + assign->toString();
}

string Parse_Select::toString() {
  return select->toString();
}


string
Parse_Function::toString() {
  ostringstream f;
  f << v->toString() << "(";
  for (int i = 0; i < args(); i++) {
    f << arg(i)->toString();
    if (i+1 < args()) f << ", ";
  }
  f << ")";
  return f.str();
}

string Parse_Vector::toString() {
  ostringstream f;
  f << "[";
  for (int i = 0; i < offsets(); i++) {
    f << offset(i)->toString();
    if (i+1 < offsets()) f << ", ";
  }
  f << "]";
  return f.str();
}

bool Parse_Vector::operator==(const Parse_Expr &e){
  try {
    const Parse_Vector& v = dynamic_cast<const Parse_Vector&>(e);
	int o1 = offsets();
	int o2 = v.offsets();
	if (o1 == o2) {
	  for (int i = 0; i < offsets(); i++) {
		if (offset(i) != v.offset(i))
		  return(false);
	  }
	  return(true);
	}
	else return(false);
  } catch (std::bad_cast e) {
    return false;
  }
}

// XXX this currently compares the syntax of the atom, not the
// expression's result!
bool Parse_VecAtom::operator==(const Parse_Expr &e)
{ 
	const Parse_VecAtom& v2 = dynamic_cast<const Parse_VecAtom&>(e);
	if (v == v2.v && offset_ == v2.offset())
	  return (true);
	else return(false);
}

string Parse_VecAtom::toString() {
  ostringstream f;
  f << v->toString() << "[";
  f << offset_->toString();
  f << "]";
  return f.str();
}

Parse_Matrix::Parse_Matrix(Parse_ExprListList *o, OL_Context *ctxt)
{
  Parse_ExprList *row;
  uint64_t columns;

  rows_ = o;
  // count to check that all rows are the same length!
  for (uint64_t i = 0; i < rows_->size(); i++) {
	row = rows_->at(i);
	if (i == 0) // initialize
	  columns = row->size();
	else if (columns != row->size()) {
	  ostringstream oss;
	  oss << "matrix " << toString() << " has rows of nonuniform length";
	  ctxt->error(oss.str());
	}
  }
}

string Parse_Matrix::toString() {
  ostringstream f;
  uint64_t rows, columns;
  bounds(rows, columns);
  f << "{";
  for (uint64_t i = 0; i < rows; i++) {
	f << "[";
	for (uint64_t j = 0; j < columns; j++) {
	  f << offset(i,j)->toString();
	  if (j+1 < columns) f << ", ";
	}
	f << "]";
	if (i+1 < rows) f << ", ";
  }
  f << "}";
  return f.str();
}

bool Parse_Matrix::operator==(const Parse_Expr &e){
  try {
    const Parse_Matrix& v = dynamic_cast<const Parse_Matrix&>(e);
	uint64_t r1, c1, r2, c2;
	bounds(r1, c1);
	v.bounds(r2, c2);
	if (r1 == r2 && c1 == c2) {
	  for (uint64_t i = 0; i < r1; i++) 
		for (uint64_t j = 0; j < c1; j++)
		  if (offset(i,j) != v.offset(i,j))
			return(false);
	  return(true);
	}
	else return(false);
  } catch (std::bad_cast e) {
    return false;
  }
}

// XXX this currently compares the syntax of the atom, not the
// expression's result!
bool Parse_MatAtom::operator==(const Parse_Expr &e)
{ 
	const Parse_MatAtom& v2 = dynamic_cast<const Parse_MatAtom&>(e);

	if (v == v2.v && offset1_ == v2.offset1_ && offset2_ == v2.offset2_)
	  return (true);
	else return(false);
}

string Parse_MatAtom::toString() {
  ostringstream f;
  f << v->toString() << "[";
  f << ", " << offset1_->toString() << "]";
  return f.str();
}


string Parse_AggTerm::toString() {

  ostringstream aggFieldStr;
  ostringstream groupByFieldStr;

  aggFieldStr << "(";
  groupByFieldStr << "(";

  for (unsigned k = 0; k < _groupByFields->size(); k++) {
    groupByFieldStr << _groupByFields->at(k)->toString();
    if (k != _groupByFields->size() - 1) {
      groupByFieldStr << ", ";
    }
  }
  groupByFieldStr << ")";


  for (unsigned k = 0; k < _aggFields->size(); k++) {
    aggFieldStr << _aggFields->at(k)->toString();
    if (k != _aggFields->size() - 1) {
      aggFieldStr << ", "; 
   }
  }
  aggFieldStr << ")";
  
  return _oper + "( "
    + groupByFieldStr.str()
    + ", "
    + aggFieldStr.str()
    + ", "
    + _baseTerm->toString()
    + " )";    
}


#endif /* __PARSER_UTIL_C__ */
