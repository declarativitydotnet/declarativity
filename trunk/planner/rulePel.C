
/*
 * @(#)$Id$
 *
 *
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 * DESCRIPTION: Code to generate PEL expressions
 *
 */


#ifndef __PL_RULEPEL_C__
#define __PL_RULEPEL_C__

string pelMath(PlanContext* pc, Parse_Math *expr);
string pelFunction(PlanContext* pc, Parse_Function *expr);
string pelBool(PlanContext* pc, Parse_Bool *expr);

void error(string msg)
{
  std::cerr << "PLANNER ERROR: " << msg << "\n";
  exit(-1);
}


void error(PlanContext* pc, string msg)
{  
  ECA_Rule* curRule = pc->_ruleStrand->getRule();
  std::cerr << "PLANNER ERROR: " << msg << " for rule " 
	    << curRule->toString() << ". Planner exits.\n";
  exit(-1);
}

// convert expression to Pel
void expr2Pel(PlanContext* pc, ostringstream &pel, Parse_Expr *e) 
{
  PlanContext::FieldNamesTracker* names = pc->_namesTracker;
  Parse_Var*  var;
  Parse_Val*  val;
  Parse_Bool     *b   = NULL;
  Parse_Math     *m   = NULL;
  Parse_Function *f   = NULL; 
  Parse_Vector   *v   = NULL;
  Parse_VecAtom  *va  = NULL;

  if ((var = dynamic_cast<Parse_Var*>(e)) != NULL) {
	// expr is a variable
	int pos = names->fieldPosition(e->toString());
	if (pos < 0) {
	  error(pc, "Error parsing Pel expression " + e->toString());
	}
	pel << "$" << (pos+1) << " ";
  }
  else if ((val = dynamic_cast<Parse_Val*>(e)) != NULL) {
	// expr is a constant
	if (val->v->typeCode() == Value::STR)
	  pel << "\"" << val->toString() << "\"" << " "; 
	else pel << val->toString() << " "; 
	if (val->id()) pel << "->u32 ->id "; 
  }
  else if (e == Parse_Expr::Now)
    pel << "now "; 
  else if ((b = dynamic_cast<Parse_Bool*>(e)) != NULL)
    pel << pelBool(pc, b);
  else if ((m = dynamic_cast<Parse_Math*>(e)) != NULL) {
    string pelMathStr = pelMath(pc, m);
    pel << pelMathStr;
  }
  else if ((f = dynamic_cast<Parse_Function*>(e)) != NULL)
    pel << pelFunction(pc, f);
  else if ((v=dynamic_cast<Parse_Vector*>(e)) != NULL) {
	pel << v->offsets() << " initvec ";
	for (int i = 0; i < v->offsets(); i++) {
	  pel << i << " " << v->offset(i)->toString() << " setvectoroffset ";
	}
  }
  else if ((va=dynamic_cast<Parse_VecAtom*>(e)) != NULL) {
	int pos2 = names->fieldPosition(va->v->toString());
	if (pos2 < 0) {
	  error(pc, "Error parsing Pel vector variable " + va->v->toString());
	}
	pel << " $" << (pos2+1) << " ";
	expr2Pel(pc, pel, va->offset_);
	pel << " getvectoroffset ";
  }
  else {    
	// TODO: throw/signal some kind of error
	error(pc, "Error parsing Pel expression " + e->toString());
  }
}

string pelMath(PlanContext* pc, Parse_Math *expr) 
{
  ostringstream  pel;  

  if (expr->id && expr->oper == Parse_Math::MINUS) {
    Parse_Expr *tmp = expr->lhs;
    expr->lhs = expr->rhs;
    expr->rhs = tmp;
  }
  expr2Pel(pc, pel, expr->lhs);
  expr2Pel(pc, pel, expr->rhs);

  switch (expr->oper) {
    case Parse_Math::LSHIFT:  pel << (expr->id ? "<<id "      : "<< "); break;
    case Parse_Math::RSHIFT:  pel << ">> "; break;
    case Parse_Math::PLUS:    pel << "+ "; break;
    case Parse_Math::MINUS:   pel << "- "; break;
    case Parse_Math::TIMES:   pel << "* "; break;
    case Parse_Math::DIVIDE:  pel << "/ "; break;
    case Parse_Math::MODULUS: pel << "\% "; break;
  default: error(pc, "Pel Math error" + expr->toString());
  }

  return pel.str();
}


string pelRange(PlanContext* pc, Parse_Bool *expr) 
{
  PlanContext::FieldNamesTracker* names = pc->_namesTracker;
  Parse_Var*   range_var = dynamic_cast<Parse_Var*>(expr->lhs);
  Parse_Range* range     = dynamic_cast<Parse_Range*>(expr->rhs);
  ostringstream pel;
  int          pos;

  if (!range || !range_var) {
    std::cerr << "Error in pel generation " << expr->toString() << "\n";
    exit(-1);
    return "ERROR";
  }

  pos = names->fieldPosition(range_var->toString());
  if (pos < 0) {
    std::cerr << "Error in pel generation " << expr->toString() << "\n";
    exit(-1);
    return "ERROR";
  }
  pel << "$" << (pos + 1) << " ";

  expr2Pel(pc, pel, range->lhs);
  expr2Pel(pc, pel, range->rhs);
  switch (range->type) {
    case Parse_Range::RANGEOO: pel << "()id "; break;
    case Parse_Range::RANGEOC: pel << "(]id "; break;
    case Parse_Range::RANGECO: pel << "[)id "; break;
    case Parse_Range::RANGECC: pel << "[]id "; break;
    }

  return pel.str();
}


string pelFunction(PlanContext* pc, Parse_Function *expr) 
{
  ostringstream pel;

  if (expr->name() == "f_coinFlip") {
    Val_Double &val = dynamic_cast<Val_Double&>(*expr->arg(0)->v);
    pel << val.toString() << " coin "; 
  }
  else if (expr->name() == "f_rand") {
    pel << "rand "; 
  } 
  else if (expr->name() == "f_now") {
    pel << "now "; 
  }
  else if (expr->name() == "f_sha1") {
    if (expr->args() != 1) {
      error(pc, "Error in pel generation " + expr->toString());
    }
	expr2Pel(pc, pel, expr->arg(0));
	pel << "sha1 ";
  }
  // functions on lists
  else if (expr->name() == "f_append") {
    if (expr->args() == 2) {
	  expr2Pel(pc, pel, expr->arg(1));
	  expr2Pel(pc, pel, expr->arg(0));
      pel << "lappend "; 
    } 
    else if (expr->args() == 1) { // append null
      pel << " null ";
	  expr2Pel(pc, pel, expr->arg(0));
      pel << "lappend ";
    } else {
      error(pc, "Error in pel generation " + expr->toString());
    }
  }
  else if (expr->name() == "f_member") {
      if (expr->args() != 2) {
	std::cerr << "Error in pel generation " << expr->toString() << "\n";
	exit(-1);
	return "ERROR.";
      }
	  expr2Pel(pc, pel, expr->arg(0));
	  expr2Pel(pc, pel, expr->arg(1));
      pel << "member "; 
  }
  else if (expr->name() == "f_concat") {
      if (expr->args() != 2) {
	std::cerr << "Error in pel generation " << expr->toString() << "\n";
	exit(-1);
	return "ERROR.";
      }
	  expr2Pel(pc, pel, expr->arg(1));
	  expr2Pel(pc, pel, expr->arg(0));
      pel << "concat "; 
  }
  else if (expr->name() == "f_intersect") {
      if (expr->args() != 2) {
	std::cerr << "Error in pel generation " << expr->toString() << "\n";
	exit(-1);
	return "ERROR.";
      }
	  expr2Pel(pc, pel, expr->arg(0));
	  expr2Pel(pc, pel, expr->arg(1));
      pel << "intersect "; 
  }
  else if (expr->name() == "f_msintersect") {
      if (expr->args() != 2) {
	std::cerr << "Error in pel generation " << expr->toString() << "\n";
	exit(-1);
	return "ERROR.";
      }
	  expr2Pel(pc, pel, expr->arg(0));
	  expr2Pel(pc, pel, expr->arg(1));
      pel << "msintersect "; 
  } 
  else if (expr->name() == "f_initList") {
    if (expr->args() != 2) {
      error(pc, "Error in pel generation " + expr->toString());
    }
	expr2Pel(pc, pel, expr->arg(0));
	expr2Pel(pc, pel, expr->arg(1));
    pel << "initlist "; 
  }
  else if (expr->name() == "f_consList") {
    if (expr->args() != 2) {
      error(pc, "Error in pel generation " + expr->toString());
    }
	expr2Pel(pc, pel, expr->arg(0));
	expr2Pel(pc, pel, expr->arg(1));
    pel << "conslist "; 
  } 
  else if (expr->name() == "f_inList") {
    if (expr->args() != 2) {
      std::cerr << "Error in pel generation " << expr->toString() << "\n";
      exit(-1);
      return "ERROR.";
    }
	expr2Pel(pc, pel, expr->arg(0));
	expr2Pel(pc, pel, expr->arg(1));
    pel << "inlist "; 
  }
  else if (expr->name() == "f_removeLast") {
    if (expr->args() != 1) {
      std::cerr << "Error in pel generation " << expr->toString() << "\n";
      exit(-1);
      return "ERROR.";
    }
	expr2Pel(pc, pel, expr->arg(0));
    pel << "removeLast "; 
  }
  
  else if (expr->name() == "f_last") {
    if (expr->args() != 1) {
      std::cerr << "Error in pel generation " << expr->toString() << "\n";
      exit(-1);
      return "ERROR.";
    }
	expr2Pel(pc, pel, expr->arg(0));
    pel << "last "; 
  }

  else if (expr->name() == "f_size") {
    if (expr->args() != 1) {
      std::cerr << "Error in pel generation " << expr->toString() << "\n";
      exit(-1);
      return "ERROR.";
    }
	expr2Pel(pc, pel, expr->arg(0));
    pel << "size "; 
  }
  
  // functions on vectors
  else if (expr->name() == "f_getVectorOffset") {
    if (expr->args() != 2) {
      std::cerr << "Error in pel generation " << expr->toString() << "\n";
      exit(-1);
      return "ERROR.";
    }
   	expr2Pel(pc, pel, expr->arg(0));
	expr2Pel(pc, pel, expr->arg(1));
	pel << " getvectoroffset "; 
  }

  else if (expr->name() == "f_setVectorOffset") {
    if (expr->args() != 3) {
      std::cerr << "Error in pel generation " << expr->toString() << "\n";
      exit(-1);
      return "ERROR.";
    }
   	expr2Pel(pc, pel, expr->arg(0));
	expr2Pel(pc, pel, expr->arg(1));
	expr2Pel(pc, pel, expr->arg(2));
    pel << "setvectoroffset "; 
  }

  else {
    std::cerr << "Error in pel generation " << expr->toString() << "\n";
    exit(-1);
    return "ERROR: unknown function name.";
  }

  return pel.str();
}


string pelBool(PlanContext* pc, Parse_Bool *expr) 
{
  ostringstream   pel;  

  if (expr->oper == Parse_Bool::RANGE) return pelRange(pc, expr);

  expr2Pel(pc, pel, expr->lhs);
  expr2Pel(pc, pel, expr->rhs);
  switch (expr->oper) {
    case Parse_Bool::NOT: pel << "not "; break;
    case Parse_Bool::AND: pel << "and "; break;
    case Parse_Bool::OR:  pel << "or "; break;
    case Parse_Bool::EQ:  pel << "== "; break;
    case Parse_Bool::NEQ: pel << "== not "; break;
    case Parse_Bool::GT:  pel << "> "; break;
    case Parse_Bool::LT:  pel << "< "; break;
    case Parse_Bool::LTE: pel << "<= "; break;
    case Parse_Bool::GTE: pel << ">= "; break;
    default: return "ERROR";
    }
  return pel.str();
}

void pelSelect(PlanContext* pc, Parse_Select *expr, int selectionID)
{
  ECA_Rule* curRule = pc->_ruleStrand->getRule();
  ostringstream sPel;
  sPel << pelBool(pc, expr->select) << "not ifstop ";
  
  // put in the old fields (+1 since we have to include the table name)
  for (uint k = 0; k < pc->_namesTracker->fieldNames.size() + 1; k++) {
    sPel << "$" << k << " pop ";
  }

  debugRule(pc, "Generate selection functions for " + sPel.str() 
		    + " " + pc->_namesTracker->toString() + "\n");
 
  ElementSpecPtr sPelTrans =
    pc->createElementSpec(ElementPtr(new PelTransform("Selection:" 
							  + curRule->_ruleID + ":" + 
							  pc->_nodeID, sPel.str())));
  pc->addElementSpec(sPelTrans);
}


void pelAssign(PlanContext* pc, Parse_Assign* expr, int assignID) 
{
  ECA_Rule* curRule = pc->_ruleStrand->getRule();
  ostringstream pel;
  ostringstream pelAssign;
  Parse_Var      *a   = dynamic_cast<Parse_Var*>(expr->var);

  expr2Pel(pc, pelAssign, expr->assign);
   
  int pos = pc->_namesTracker->fieldPosition(a->toString());
  for (int k = 0; k < int(pc->_namesTracker->fieldNames.size()+1); k++) {
    if (k == pos) pel << pelAssign.str() << "pop ";
    else pel << "$" << k << " pop ";
  }
  if (pos < 0) { 
    pel << pelAssign.str() << "pop ";
    pc->_namesTracker->fieldNames.push_back(a->toString()); // the variable name
  } 

  debugRule(pc, "Generate assignments for " + a->toString() + " " 
	    + curRule->_ruleID + " " + pel.str() + " " 
	    + pc->_namesTracker->toString() + "\n");
  
  ElementSpecPtr assignPelTrans =
    pc->createElementSpec(ElementPtr(new PelTransform("Assignment:" + curRule->_ruleID + ":" 
							  + pc->_nodeID, pel.str())));
  
  pc->addElementSpec(assignPelTrans);  
}

#endif
