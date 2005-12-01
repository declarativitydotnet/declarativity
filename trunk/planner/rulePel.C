
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
 * DESCRIPTION: Code to generate PEL expressions
 *
 */


#ifndef __PL_RULEPEL_C__
#define __PL_RULEPEL_C__



str pelFunction(PlanContext::FieldNamesTracker* names, Parse_Function *expr);

str pelMath(PlanContext::FieldNamesTracker* names, Parse_Math *expr) 
{
  Parse_Var*  var;
  Parse_Val*  val;
  Parse_Math* math;
  Parse_Function* fn  = NULL;
  strbuf      pel;  


  if (expr->id && expr->oper == Parse_Math::MINUS) {
    Parse_Expr *tmp = expr->lhs;
    expr->lhs = expr->rhs;
    expr->rhs = tmp;
  }

  if ((var = dynamic_cast<Parse_Var*>(expr->lhs)) != NULL) {
    int pos = names->fieldPosition(var->toString());
    pel << "$" << (pos+1) << " ";
  }
  else if ((val = dynamic_cast<Parse_Val*>(expr->lhs)) != NULL) {
    if (val->v->typeCode() == Value::STR)
      pel << "\"" << val->toString() << "\"" << " "; 
    else pel << val->toString() << " "; 

    if (val->id()) pel << "->u32 ->id "; 
    else if (expr->id) pel << "->u32 ->id "; 
  }
  else if ((math = dynamic_cast<Parse_Math*>(expr->lhs)) != NULL) {
    pel << pelMath(names, math); 
  }
  else if ((fn = dynamic_cast<Parse_Function*>(expr->lhs)) != NULL) {
    pel << pelFunction(names, fn); 
  }
  else {
    // TODO: throw/signal some kind of error
    std::cerr << "Error in pel generation " << expr->toString() << "\n";
    exit(-1);
    return "MATH ERROR";
  }

  if ((var = dynamic_cast<Parse_Var*>(expr->rhs)) != NULL) {
    int pos = names->fieldPosition(var->toString());
    pel << "$" << (pos+1) << " ";
  }
  else if ((val = dynamic_cast<Parse_Val*>(expr->rhs)) != NULL) {    
    if (val->v->typeCode() == Value::STR)
      pel << "\"" << val->toString() << "\"" << " "; 
    else pel << val->toString() << " "; 

    if (val->id()) pel << "->u32 ->id "; 
    else if (expr->id) pel << "->u32 ->id "; 
  }
  else if ((math = dynamic_cast<Parse_Math*>(expr->rhs)) != NULL) {
    pel << pelMath(names, math); 
  }
  else {
    // TODO: throw/signal some kind of error
    std::cerr << "Error in pel generation " << expr->toString() << "\n";
    exit(-1);

    return "MATH ERROR";
  }

  switch (expr->oper) {
    case Parse_Math::LSHIFT:  pel << (expr->id ? "<<id "      : "<< "); break;
    case Parse_Math::RSHIFT:  pel << ">> "; break;
    case Parse_Math::PLUS:    pel << "+ "; break;
    case Parse_Math::MINUS:   pel << "- "; break;
    case Parse_Math::TIMES:   pel << "* "; break;
    case Parse_Math::DIVIDE:  pel << "/ "; break;
    case Parse_Math::MODULUS: pel << "\% "; break;
    default: return "ERROR";
  }

  return pel;
}


str pelRange(PlanContext::FieldNamesTracker* names, Parse_Bool *expr) 
{
  Parse_Var*   var       = NULL;
  Parse_Val*   val       = NULL;
  Parse_Math*  math      = NULL;
  Parse_Var*   range_var = dynamic_cast<Parse_Var*>(expr->lhs);
  Parse_Range* range     = dynamic_cast<Parse_Range*>(expr->rhs);
  strbuf       pel;
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

  if ((var = dynamic_cast<Parse_Var*>(range->lhs)) != NULL) {
    pos = names->fieldPosition(var->toString());
    if (pos < 0) {
      std::cerr << "Error in pel generation " << expr->toString() << "\n";
      exit(-1);
      return "ERROR";
    }
    pel << "$" << (pos + 1) << " ";
  }
  else if ((val = dynamic_cast<Parse_Val*>(range->lhs)) != NULL) {
    pel << val->toString() << " ";
  }
  else if ((math = dynamic_cast<Parse_Math*>(range->lhs)) != NULL) {
   pel << pelMath(names, math);
  }
  else {
    std::cerr << "Error in pel generation " << expr->toString() << "\n";
    exit(-1);
    return "ERROR";
  }

  if ((var = dynamic_cast<Parse_Var*>(range->rhs)) != NULL) {
    pos = names->fieldPosition(var->toString());
    if (pos < 0) {
      std::cerr << "Error in pel generation " << expr->toString() << "\n";
      exit(-1);      
      return "ERROR";
    }
    pel << "$" << (pos + 1) << " ";
  }
  else if ((val = dynamic_cast<Parse_Val*>(range->rhs)) != NULL) {
    pel << val->toString() << " ";
  }
  else if ((math = dynamic_cast<Parse_Math*>(range->rhs)) != NULL) {
   pel << pelMath(names, math);
  }
  else {
    std::cerr << "Error in pel generation " << expr->toString() << "\n";
    exit(-1);

    return "ERROR";
  }

  switch (range->type) {
    case Parse_Range::RANGEOO: pel << "() "; break;
    case Parse_Range::RANGEOC: pel << "(] "; break;
    case Parse_Range::RANGECO: pel << "[) "; break;
    case Parse_Range::RANGECC: pel << "[] "; break;
    }

  return pel;
}


str pelFunction(PlanContext::FieldNamesTracker* names, Parse_Function *expr) 
{
  strbuf pel;

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
  else if (expr->name() == "f_initList") {
    if (expr->args() != 2) {
      std::cerr << "Error in pel generation " << expr->toString() << "\n";
      exit(-1);
      return "ERROR.";
    }
    pel << "$" << 1 + names->fieldPosition(expr->arg(0)->toString());
    pel << " $" << 1 + names->fieldPosition(expr->arg(1)->toString()) << " ";
    pel << "initlist "; 
  }
  else if (expr->name() == "f_consList") {
    if (expr->args() != 2) {
      std::cerr << "Error in pel generation " << expr->toString() << "\n";
      exit(-1);
      return "ERROR.";
    }
    pel << "$" << 1 + names->fieldPosition(expr->arg(0)->toString());
    pel << " $" << 1 + names->fieldPosition(expr->arg(1)->toString()) << " ";
    pel << "conslist "; 
  } 
  else if (expr->name() == "f_inList") {
    if (expr->args() != 2) {
      std::cerr << "Error in pel generation " << expr->toString() << "\n";
      exit(-1);
      return "ERROR.";
    }
    pel << "$" << 1 + names->fieldPosition(expr->arg(0)->toString());
    pel << " $" << 1 + names->fieldPosition(expr->arg(1)->toString()) << " ";
    pel << "inlist "; 
  }
  else if (expr->name() == "f_removeLast") {
    if (expr->args() != 1) {
      std::cerr << "Error in pel generation " << expr->toString() << "\n";
      exit(-1);
      return "ERROR.";
    }
    pel << "$" << 1 + names->fieldPosition(expr->arg(0)->toString()) << " ";
    pel << "removeLast "; 
  }
  
  else if (expr->name() == "f_last") {
    if (expr->args() != 1) {
      std::cerr << "Error in pel generation " << expr->toString() << "\n";
      exit(-1);
      return "ERROR.";
    }
    pel << "$" << 1 + names->fieldPosition(expr->arg(0)->toString()) << " ";
    pel << "last "; 
  }

  else if (expr->name() == "f_size") {
    if (expr->args() != 1) {
      std::cerr << "Error in pel generation " << expr->toString() << "\n";
      exit(-1);
      return "ERROR.";
    }
    pel << "$" << 1 + names->fieldPosition(expr->arg(0)->toString()) << " ";
    pel << "size "; 
  }
  

  else {
    std::cerr << "Error in pel generation " << expr->toString() << "\n";
    exit(-1);
    return "ERROR: unknown function name.";
  }

  return pel;
}


str pelBool(PlanContext::FieldNamesTracker* names, Parse_Bool *expr) 
{
  Parse_Var*      var = NULL;
  Parse_Val*      val = NULL;
  Parse_Function* fn  = NULL;
  Parse_Math*     m   = NULL;
  Parse_Bool*     b   = NULL;
  strbuf          pel;  

  if (expr->oper == Parse_Bool::RANGE) return pelRange(names, expr);

  bool strCompare = false;
  if ((var = dynamic_cast<Parse_Var*>(expr->lhs)) != NULL) {
    int pos = names->fieldPosition(var->toString());
    pel << "$" << (pos+1) << " ";
  }
  else if ((val = dynamic_cast<Parse_Val*>(expr->lhs)) != NULL) {
    if (val->v->typeCode() == Value::STR) { 
      strCompare = true; 
      pel << "\"" << val->toString() << "\" "; 
    } else {
      strCompare = false;
      pel << val->toString() << " "; 
    }
  }
  else if ((b = dynamic_cast<Parse_Bool*>(expr->lhs)) != NULL) {
    pel << pelBool(names, b); 
  }
  else if ((m = dynamic_cast<Parse_Math*>(expr->lhs)) != NULL) {
    pel << pelMath(names, m); 
  }
  else if ((fn = dynamic_cast<Parse_Function*>(expr->lhs)) != NULL) {
      pel << pelFunction(names, fn); 
  }
  else {
    // TODO: throw/signal some kind of error
    std::cerr << "Error in pel generation " << expr->toString() << "\n";
    exit(-1);
    return "UNKNOWN BOOL OPERAND ERROR";
  }

  if (expr->rhs != NULL) {
    if ((var = dynamic_cast<Parse_Var*>(expr->rhs)) != NULL) {
      int pos = names->fieldPosition(var->toString());
      pel << "$" << (pos+1) << " ";
    }
    else if ((val = dynamic_cast<Parse_Val*>(expr->rhs)) != NULL) {      
      if (val->v->typeCode() == Value::STR) { 
	strCompare = true; 
	pel << "\"" << val->toString() << "\" "; 
      } else {
	strCompare = false;
	pel << val->toString() << " "; 
      }
    }
    else if ((b = dynamic_cast<Parse_Bool*>(expr->rhs)) != NULL) {
      pel << pelBool(names, b); 
    }
    else if ((m = dynamic_cast<Parse_Math*>(expr->rhs)) != NULL) {
      pel << pelMath(names, m); 
    }
    else if ((fn = dynamic_cast<Parse_Function*>(expr->rhs)) != NULL) {
      pel << pelFunction(names, fn); 
    }
    else {
      // TODO: throw/signal some kind of error
      std::cerr << "Error in pel generation " << expr->toString() << "\n";
      exit(-1);
      return "UNKNOWN BOOL OPERAND ERROR";
    }
  }

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
  return pel;
}

void pelSelect(PlanContext* pc, Parse_Select *expr, int selectionID)
{
  ECA_Rule* curRule = pc->_ruleStrand->_eca_rule;
  strbuf sPel = pelBool(pc->_namesTracker, expr->select) << "not ifstop ";
  
  // put in the old fields (+1 since we have to include the table name)
  for (uint k = 0; k < pc->_namesTracker->fieldNames.size() + 1; k++) {
    sPel << "$" << k << " pop ";
  }

  debugRule(pc, str(strbuf() << "Generate selection functions for " << sPel 
		    << " " << pc->_namesTracker->toString() << "\n"));
 
  ElementSpecRef sPelTrans =
    pc->_conf->addElement(New refcounted< PelTransform >(strbuf("Selection:") 
						     << curRule->_ruleID << ":" << 
						     selectionID << ":" 
						     << pc->_nodeID, sPel));
  pc->_ruleStrand->addElement(pc->_conf, sPelTrans);
}


void pelAssign(PlanContext* pc, Parse_Assign* expr, int assignID) 
{
  ECA_Rule* curRule = pc->_ruleStrand->_eca_rule;
  strbuf pel;
  strbuf pelAssign;
  Parse_Var      *a   = dynamic_cast<Parse_Var*>(expr->var);
  Parse_Var      *var = NULL;
  Parse_Val      *val = NULL;
  Parse_Bool     *b   = NULL;
  Parse_Math     *m   = NULL;
  Parse_Function *f   = NULL; 

  if (expr->assign == Parse_Expr::Now)
    pelAssign << "now "; 
  else if ((b = dynamic_cast<Parse_Bool*>(expr->assign)) != NULL)
    pelAssign << pelBool(pc->_namesTracker, b);
  else if ((m = dynamic_cast<Parse_Math*>(expr->assign)) != NULL)
    pelAssign << pelMath(pc->_namesTracker, m);
  else if ((f = dynamic_cast<Parse_Function*>(expr->assign)) != NULL)
    pelAssign << pelFunction(pc->_namesTracker, f);
  else if ((var=dynamic_cast<Parse_Var*>(expr->assign)) != NULL && 
           pc->_namesTracker->fieldPosition(var->toString()) >= 0)                        
    pelAssign << "$" << (pc->_namesTracker->fieldPosition(var->toString())+1) << " ";
  else if ((val=dynamic_cast<Parse_Val*>(expr->assign)) != NULL) {
    if (val->v->typeCode() == Value::STR) { 
      pelAssign << "\"" << val->toString() << "\" ";
    } else {
      pelAssign << val->toString() << " ";
    }
  } else {
    std::cerr << "ASSIGN ERROR!\n";
    assert(0);
  }
   
  int pos = pc->_namesTracker->fieldPosition(a->toString());
  for (int k = 0; k < int(pc->_namesTracker->fieldNames.size()+1); k++) {
    if (k == pos) pel << pelAssign << "pop ";
    else pel << "$" << k << " pop ";
  }
  if (pos < 0) { 
    pel << pelAssign << "pop ";
    pc->_namesTracker->fieldNames.push_back(a->toString()); // the variable name
  } 

  debugRule(pc, strbuf() << "Generate assignments for " 
	    << a->toString() << " " 
	    << curRule->_ruleID << " " << pel << " " 
	    << pc->_namesTracker->toString() << "\n");
  
  ElementSpecRef assignPelTrans =
    pc->_conf->addElement(New refcounted< PelTransform >(strbuf("Assignment:") 
						     << curRule->_ruleID << ":" 
						     << assignID << ":" 
						     << pc->_nodeID, 
						     pel));

  pc->_ruleStrand->addElement(pc->_conf, assignPelTrans);  
}

#endif
