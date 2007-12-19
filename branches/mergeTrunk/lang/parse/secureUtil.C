#include "secureUtil.h"
#include "val_uint32.h"

namespace compile {
  namespace parse{

      /**
       * Returns a list of tables used in says: for authentication algebra generation
       */
      void Secure::initializeRule(Rule *r, StatementList *s)
      {
	int newVariable = 1;
	Says *sh;
	std::list<TermList*> newRuleComparators;
	sh = dynamic_cast<Says*>(r->_head);
	string headTableName = ((sh!=NULL)?sh->name():"");
	if(sh != NULL)
	  {
	    // first rule generates the proof assuming that the node 
	    // executing the rule has the key
	    TermList *newTermsGen = normalizeGenerate(sh, newVariable);
	    if(newTermsGen != NULL){
	      newRuleComparators.push_back(newTermsGen);
	      r->_head = new Functor(sh);
	      
	    }
	  }
	
	for (TermList::iterator iter = r->_body->begin(); 
	     iter != r->_body->end(); iter++) {
	  Says *s;
	  if ((s = dynamic_cast<Says*>(*iter)) != NULL)
	    {
	      string saysTableName = s->name();
	      normalizeVerify(s, newVariable);
	      
	      if(sh != NULL && (saysTableName.compare(headTableName)==0))
		{
		  ExpressionList *head =  const_cast<ExpressionList*>(r->_head->arguments());
		  
		  TermList *comparisonTerms = generateEqTerms(head->begin(),
								    head->end()-TableEntry::numSecureFields,
								    s->saysArgs()->begin(), 
								    s->saysArgs()->end()-TableEntry::numSecureFields);
		  generateAlgebraLT(head->end()-TableEntry::numSecureFields, 
					  s->saysArgs()->end()-TableEntry::numSecureFields, 
					  comparisonTerms);
		  generateEqTerms(head->end()-1, head->end(), 
					s->saysArgs()->end()-1, s->saysArgs()->end(), 
					comparisonTerms);
		  newRuleComparators.push_back(comparisonTerms);
		}
	  
	      
	      r->_body->erase(iter);
	      // make a deep copy even though we can live with shallow one 
	      // to ensure that the following delete can be executed safely
	      r->_body->push_back(new Functor(*s));
	      delete s;
	      
	      
	      
	    }
	}
	
	if(sh != NULL)
	  {
	    //	delete sh;
	    // copy this rule into a new rule and insert the new rule into the list as well 
	    // as modify the existing rule
	    int size = newRuleComparators.size();
	    Rule *newr;
	    for(int i = 0; i < size; i++){
	      // if i < size - 1, then create a copy for next stage
	      // else use the current copy
	      if(i < size - 1){
		newr = new Rule(*r);
		newr->resetName();
		s->push_back(newr);
	      }
	      else{
		newr = r;
	      }
	      
	      TermList *newTermsUse = newRuleComparators.front();
	      newRuleComparators.pop_front();
	      if(newTermsUse != NULL){
		
		for (TermList::iterator it = newTermsUse->begin(); 
		     it != newTermsUse->end(); it++) {
		  newr->_body->push_back(*it);
		}
		delete newTermsUse;
	      }
	      
	    }
	  }
	
      }
      
      
      void 
      Secure::program(StatementList *s, bool parserCall) 
      {
	static bool materialized = false;
	if(parserCall)
	  {
	    if(!materialized){
	      StatementList *mat = generateMaterialize();
	      for (StatementList::iterator iter = mat->begin();
		   iter != mat->end(); iter++) { 
		s->push_front(*iter);
	      }
	      delete mat;
	      materialized = true;
	    }
	  }
	
	Rule* r;
	Namespace *nmSpc;
	Table *tab;
	SetPtr materializedSaysTables(new Set());

	for (StatementList::iterator iter = s->begin();
             iter != s->end(); iter++) { 
	  
	  if ((r = dynamic_cast<Rule*>(*iter)) != NULL) 
	    {
	      initializeRule(r, s);
	    }
	  else if((nmSpc = dynamic_cast<Namespace*>(*iter)) != NULL) 
	    {
	      if(nmSpc->statements() != NULL)
		program(nmSpc->statements(), false);
	    }
	  else if((tab = dynamic_cast<Table*>(*iter)) != NULL)
	    {
	      //	  tab->initialize();
	    }
	  else
	    {
	      // do nothing
	    }
	  
	}
	
	
      }

      TermList* Secure::generateAlgebraLT(ExpressionList::iterator start1, 
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
	generateSelectTerms(start1, start1 + (TableEntry::numSecureFields - 1), 
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


      TermList* Secure::generateSelectTerms(ExpressionList::iterator start1, ExpressionList::iterator end1, 
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


      TermList* Secure::generateEqTerms(ExpressionList::iterator start1, ExpressionList::iterator end1, 
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


      StatementList* Secure::generateMaterialize()
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
      
	Table *encHint = new Table(encHintName, new compile::parse::Value(minusOne), new compile::parse::Value(minusOne), t, false, false);
      
	mat->push_back(encHint);

	//generate materialize for verTable
	Value *verTableName = new Value(Val_Str::mk(Says::verTable));
	t = new ExpressionList();
	t->push_back(new compile::parse::Value(one)); 
	t->push_back(new compile::parse::Value(two)); 
      
	Table *verTable = new Table(verTableName, new compile::parse::Value(minusOne), new compile::parse::Value(minusOne), t, false, false);
      
	mat->push_back(verTable);

	//generate materialize for genTable
	Value *genTableName = new Value(Val_Str::mk(Says::genTable));
	t = new ExpressionList();
	t->push_back(new compile::parse::Value(one)); 
	t->push_back(new compile::parse::Value(two)); 
      
	Table *genTable = new Table(genTableName, new compile::parse::Value(minusOne), new compile::parse::Value(minusOne), t, false, false);
      
	mat->push_back(genTable);

	return mat;
      }

      // modify the says term into the saysTable form with the proof and P, R, k, V fields
      void Secure::normalizeVerify(Functor* f, int& newVariable)
      {
	Says *s;
	if ((s = dynamic_cast<Says*>(f)) != NULL)
	  {
	    // now modify the says tuple
	    //TODO: need to ensure that the generated says tuple has the right form: also the caller expects the last field to contain the proof so need to modify that code too
	    ExpressionList::iterator iter;
	    ExpressionList *arg = s->saysArgs();
	    ExpressionList *saysList =  const_cast<ExpressionList*>(s->saysParams());
	
	    for (iter = saysList->begin(); 
		 iter != saysList->end(); iter++){
	      arg->push_back(*iter);
	    }

	    ostringstream var5;
	    var5 << Says::varPrefix << newVariable++;
	    Variable *proof = new Variable(Val_Str::mk(var5.str()));
	    arg->push_back(proof);
	
	    s->changeName(s->name() + Says::saysPrefix);

	  }
      }

      // return a list of terms that needs to be added to the rule on converting the 
      // securelog term f into overlog.
      // Also converts f into the appropriate overlog form
      TermList* Secure::normalizeGenerate(Functor* f, int& newVariable)
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
	    for(uint32_t i = 0; i< TableEntry::numSecureFields -1; i++)
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
	    Functor *genHint = new Functor(new Value(Val_Str::mk(Says::encHint)), t);

	    //create key generation table
	    ExpressionList *genT = new ExpressionList();
	    genT->push_back(Variable::getLocalLocationSpecifier());
	    genT->push_back(encId);

	    ostringstream var2;
	    var2 << Says::varPrefix << newVariable++;
	    Variable *genKey = new Variable(Val_Str::mk(var2.str()));
	    genT->push_back(genKey);
	    Functor *genTable = new Functor(new Value(Val_Str::mk(Says::genTable)), genT);


	    newTerms->push_back(genHint);
	    newTerms->push_back(genTable);

	    // add constraints between saysParams and rhsSaysParams
	    generateAlgebraLT(saysList->begin(), 
			      rhsSaysParams->begin(), newTerms);

	
	    // now modify the says tuple
	    //TODO: changed code here: need to ensure that the generated says tuple has the right form: also the caller expects the last field to contain the proof so need to modify that code too
	    ExpressionList *arg = s->saysArgs();
	    for (iter = saysList->begin(); 
		 iter != saysList->end(); iter++){
	      arg->push_back(*iter);
	    }
	
	    s->changeName(s->name() + Says::saysPrefix);

	    // finally return the new terms
	    return newTerms;
	  }
	return NULL;
      }
      
  }
}
