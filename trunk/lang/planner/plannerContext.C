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

#include "plannerContext.h"
#include "plumber.h"
#include "reporting.h"
#include "systemTable.h"
#include "list.h"
#include "val_null.h"
#include "val_list.h"
#include "val_tuple.h"
#include "val_int64.h"
#include "oper.h"

using namespace opr;

namespace compile {
  namespace planner {

    using namespace std;
    using namespace opr;
  
    static string 
    int_list_to_str(std::vector<unsigned>& values)
    {
      ostringstream oss;
      oss << "{";
      for (std::vector<unsigned>::iterator iter = values.begin(); 
           iter != values.end(); iter++) {
        oss << *iter;
        if (iter + 1  != values.end())
          oss << ", ";
      }
      oss << "}";
      return oss.str(); 
    }
  
    DEFINE_ELEMENT_INITS_NS(Context, "PlannerContext", compile::planner)

    Context::Context(string name, string d, string isie, string isoe, string esoe)
    : compile::Context(name), 
      _mainDataflowName(d), 
      _internalStrandInputElement(isie), 
      _internalStrandOutputElement(isoe), 
      _externalStrandOutputElement(esoe),
      _nameCounter(0) { }

    Context::Context(TuplePtr args)
    : compile::Context((*args)[2]->toString()), 
      _mainDataflowName((*args)[3]->toString()), 
      _internalStrandInputElement((*args)[4]->toString()), 
      _internalStrandOutputElement((*args)[5]->toString()), 
      _externalStrandOutputElement((*args)[6]->toString()),
      _nameCounter(0) { }
  
    Context::~Context()
    {}

    int
    Context::initialize()
    {
      CommonTable::ManagerPtr catalog = Plumber::catalog();
      CommonTablePtr programTbl = catalog->table(PROGRAM);  
      CommonTable::Iterator iter = programTbl->scan();
      while (!iter->done()) {
        compile::Context::program(catalog, iter->next());
      }
      return 0;
    }

    bool 
    Context::watched(string name, string mod)
    {
      TELL_INFO << "Planner: Asked if table "
                << name
                << " is watched with modifier '"
                << mod
                << "'...";
      CommonTable::ManagerPtr catalog = Plumber::catalog();
      CommonTablePtr watchTbl = catalog->table(WATCH);
      TuplePtr lookup = Tuple::mk(WATCH);
      lookup->append(Val_Str::mk(name));
      lookup->append(Val_Null::mk());
      lookup->freeze();
      CommonTable::Key indexKey;
      indexKey.push_back(catalog->attribute(WATCH, "NAME"));

      CommonTable::Iterator i = 
        watchTbl->lookup(CommonTable::theKey(CommonTable::KEY2),
                         CommonTable::theKey(CommonTable::KEY4),
                         lookup); 
      while (!i->done()) {
        // Found something.
        if (mod == "") {
          TELL_INFO << "Yes (universal modifier)\n";
          return true;
        } else {
          // Does it contain this explicit modifier?
          TuplePtr theWatchSpec = i->next();
          string theWatchModifier =
            (*theWatchSpec)[catalog->attribute(WATCH,
                                               "MOD")]->toString();
          if (theWatchModifier == "") {
            // The watch is universal, so whatever my modifier is, it's
            // included
            TELL_INFO << "Yes (all modifiers are watched)\n";
            return true;
          } else if (theWatchModifier.find(mod) !=
                     theWatchModifier.npos) {
            // The watch is not universal, and it doesn't contain my modifier
            TELL_INFO << "No (only '"
                 << theWatchModifier
                 << "' is being watched)\n";
            return false;
          } else {
            // The watch is not universal and I found my modifier in it
            TELL_INFO << "Yes (explicitly)\n";
            return true; 
          }
        }
      }
      TELL_INFO << "No (no watch on this table)\n";
      return false;
    }

    TuplePtr 
    Context::program(CommonTable::ManagerPtr catalog, TuplePtr program) 
    {
      programEvents.clear();
      ostringstream stage_oss;

      CommonTable::Key indexKey;
      CommonTable::Iterator iter;
      ElementPtr demux = 
        Plumber::dataflow(_mainDataflowName)->find(_internalStrandInputElement)->element();

      indexKey.push_back(catalog->attribute(STAGE, "PID"));
      iter =
        catalog->table(STAGE)->lookup(CommonTable::theKey(CommonTable::KEY2), indexKey, program); 
      bool globalP2DL = !iter->done();
      if (globalP2DL) stage_oss << "edit main {\n";
      while (!iter->done()) {
        TuplePtr stage = iter->next();
        string processor = (*stage)[catalog->attribute(STAGE, "PROCESSOR")]->toString();
        string input     = (*stage)[catalog->attribute(STAGE, "INPUT")]->toString();
        string output    = (*stage)[catalog->attribute(STAGE, "OUTPUT")]->toString();

        string intStrandInputElement  = "main." + _internalStrandInputElement;
        string duplicatorName = "dup_" + input;
        for (string::size_type s = 0;
             (s = duplicatorName.find("::", s)) != string::npos; s++)
          duplicatorName.replace(s, 2, "_");

        if (demux->output(Val_Str::mk(input)) >= 0 ||
            programEvents.find(input) != programEvents.end()) {
          // TODO: Create dependency
          intStrandInputElement = "main." + duplicatorName + "[+]";
        }
        else {
          ostringstream oss;
          oss << "[+ \"" + input + "\"] -> ";
          oss << "DDuplicateConservative(\"" + duplicatorName + "\", 1)";
          intStrandInputElement += oss.str();
          programEvents.insert(std::make_pair(input, (*program)[TUPLE_ID]));
        }
        stage_oss << "\t" << intStrandInputElement; 
        if (watched(input, "c")) {
          stage_oss << " -> Print(\"RecvEvent: STAGE:" << processor << "(" << input << ")\")"; 
        }
        stage_oss << " ->  Queue(\"stageQueue\", 1000) ->\n\t"
                  << "Stage(\"stage_" << processor << "\", \"" << processor << "\") ->\n\t"
                  << "PelTransform(\"formatStage\", \"\\\"" << output << "\\\" pop swallow unbox drop popall\") -> ";
        if (watched(output, "s")) {
          stage_oss << "Print(\"SendAction: STAGE " << processor << "\") -> "; 
        }
        stage_oss << "PelTransform(\"package\", \"$1 pop swallow pop\") -> "
                  << "[+]main." << _internalStrandOutputElement << ";\n";
      }
      if (globalP2DL) stage_oss << "};\n";

      // Add all tables defined by this program
      indexKey.clear();
      indexKey.push_back(catalog->attribute(TABLE, "PID"));
      iter = catalog->table(TABLE)->
        lookup(CommonTable::theKey(CommonTable::KEY2), indexKey, program); 

      while (!iter->done()) {
        TuplePtr tbl = iter->next(); // The row in the fact table
        string name  = (*tbl)[catalog->attribute(TABLE, "TABLENAME")]->toString();
        string ttl   = (*tbl)[catalog->attribute(TABLE, "LIFETIME")]->toString();
        string size  = (*tbl)[catalog->attribute(TABLE, "SIZE")]->toString();
        string keys  = (*tbl)[catalog->attribute(TABLE, "KEY")]->toString();
        stage_oss << "createtable(" << name << ", " << ttl << ", " << size 
                  << ", keys" << keys << ");" << std::endl; 
        globalP2DL = true;
      }

      // Add all facts asserted by this program
      indexKey.clear();
      indexKey.push_back(catalog->attribute(FACT, "PID"));
      iter = catalog->table(FACT)->
        lookup(CommonTable::theKey(CommonTable::KEY2), indexKey, program); 

      while (!iter->done()) {
        TuplePtr fact = iter->next(); // The row in the fact table
        string tablename = (*fact)[catalog->attribute(FACT, "TABLENAME")]->toString();
        fact = Val_Tuple::cast((*fact)[catalog->attribute(FACT, "TUPLE")]);
        stage_oss << "fact " << fact->toConfString() << ";"; 
        globalP2DL = true;
      }

      if (globalP2DL) {
        program = program->clone();
        program->set(catalog->attribute(PROGRAM, "P2DL"), Val_Str::mk(stage_oss.str()));
      }
      return this->compile::Context::program(catalog, program);
    }

    void 
    Context::rule(CommonTable::ManagerPtr catalog, TuplePtr rule)
    {
      ostringstream oss;
      ostringstream event_oss;
      ostringstream condition_oss;
      ostringstream action_oss;

      TELL_INFO << "PLAN RULE: " << rule->toString() << std::endl;

      TuplePtr programTp;
      CommonTable::Key lookupField;
      lookupField.push_back(catalog->attribute(RULE, "PID"));
      CommonTablePtr programTbl = catalog->table(PROGRAM);
      CommonTable::Iterator i = 
        programTbl->lookup(lookupField, CommonTable::theKey(CommonTable::KEY2), rule); 
      if (!i->done()) {
        programTp = i->next();
      }
      else {
        throw Exception("No program tuple for rule tuple: " + rule->toString());
      }

      TuplePtrList terms;
      ruleTerms(catalog, rule, terms);
      if (terms.size() < 2) {
        CommonTable::Iterator 
          iter = catalog->table(FUNCTOR)->lookup(CommonTable::theKey(CommonTable::KEY2),
                                                 CommonTable::theKey(CommonTable::KEY3),
                                                 rule);
        TELL_ERROR << "WE SHOULD HAVE GOT FUNCTORS: " << std::endl;
        while (!iter->done()) {
           TuplePtr tp = iter->next();
           TELL_ERROR << "\t" << tp->toString() << std::endl;
           for (uint i = 0; i < tp->size(); i++) {
             TELL_ERROR << (*tp)[i]->typeCode() << ", ";
           }
           TELL_ERROR << std::endl;
        }
        TELL_ERROR << "WE ONLY GOT THE FOLLOWING TERMS: " << std::endl;
        for (TuplePtrList::iterator iter = terms.begin(); 
             iter != terms.end(); iter++) {
          TELL_ERROR << "\t" << (*iter)->toString() << std::endl;
        }
        throw Exception("FATAL ERROR: Rule must contain at least 2 terms (head + event)!: " + rule->toString());
      }
  
      Context::PortDesc epd = event(event_oss, "\t", rule, catalog, terms);
      Context::PortDesc cpd = condition(condition_oss, "\t", rule, catalog, terms);
      Context::PortDesc apd = action(action_oss, "\t", rule, catalog, terms);
  
      ValuePtr rname     = (*rule)[catalog->attribute(RULE, "NAME")];
      TuplePtr head      = terms.at(0); 
      TuplePtr event     = terms.at(1); 
      string   headName  = (*head)[catalog->attribute(FUNCTOR, "NAME")]->toString();
      string   eventName = (*event)[catalog->attribute(FUNCTOR, "NAME")]->toString();
      string   eventType = (*event)[catalog->attribute(FUNCTOR, "ECA")]->toString();
      string actionType  = (*head)[catalog->attribute(FUNCTOR, "ECA")]->toString();
      ListPtr  headArgs  = Val_List::cast((*head)[catalog->attribute(FUNCTOR, "ATTRIBUTES")]);
      ListPtr  eventArgs = Val_List::cast((*event)[catalog->attribute(FUNCTOR, "ATTRIBUTES")]); 
      int   doAggwrap  = namestracker::aggregation(headArgs) >= 0 && eventType != "INSERT";
      ostringstream aggwrap;

      if (doAggwrap) {
        string aggOper  = "";
        int    aggField = 0;
        int    starAgg  = 0;
        CommonTable::Key groupByFields;
        for (ValPtrList::const_iterator iter = headArgs->begin();
             iter != headArgs->end(); iter++) {
          TuplePtr arg = Val_Tuple::cast(*iter);
          if ((*arg)[0]->toString() == VAR || (*arg)[0]->toString() == LOC) {
            int pos = namestracker::position(eventArgs, *iter);
            if (pos < 0) {
              throw planner::Exception("Group by variables must be in event only! " + rule->toString());
            }
            groupByFields.push_back(pos+1);          
          }
          else if ((*arg)[0]->toString() == AGG) {
            aggOper  = (*arg)[3]->toString();    // Agg operator
            aggField = groupByFields.size() + 1; // Agg field position is relative to head schema
            starAgg  = (*arg)[2] == Val_Null::mk() ?  1 : 0;
          }
          else {
              throw planner::Exception("Unknown variable type in head functor! " + rule->toString());
          }
        }
        aggwrap << "aggwrap = Aggwrap2(\"aggwrap\", \"" 
                << aggOper << "\", " << aggField << ", " << starAgg 
                << ", " << int_list_to_str(groupByFields) << ", \"" << headName << "\");\n";
        
      }

      string intStrandInputElement  = "main." + _internalStrandInputElement;
      string intStrandOutputElement = "main." + _internalStrandOutputElement;
      string extStrandOutputElement = "main." + _externalStrandOutputElement;


      if (epd.inputs) {
        // Must create a duplicator, but its name can't have '::' in it.
        string duplicatorName = "dup_" + eventName;
        for (string::size_type s = 0; (s = duplicatorName.find("::", s)) != string::npos; s++) {
          duplicatorName.replace(s, 2, "_");
        }

        ElementPtr demux = 
          Plumber::dataflow(_mainDataflowName)->find(_internalStrandInputElement)->element();
        if (demux->output(Val_Str::mk(eventName)) >= 0 ||
            programEvents.find(eventName) != programEvents.end()) {
          // TODO: Create rule dependency
          intStrandInputElement = "main." + duplicatorName + "[+]";
        }
        else {
          ostringstream oss;
          oss << "[+ \"" + eventName + "\"] -> ";
          oss << "DDuplicateConservative(\"" + duplicatorName + "\", 1)";
          intStrandInputElement += oss.str();
          programEvents.insert(std::make_pair(eventName, (*rule)[TUPLE_ID]));
        }
      }
          
      string graphName = rname->toString();
      for (string::size_type s = 0; (s = graphName.find("::", s)) != string::npos; s++) {
         graphName.replace(s, 2, "_");
      }
      oss << "graph " << graphName
          << "(" << epd.inputs << ", " << apd.outputs << ", \""
          <<  epd.inProc << "/" << apd.outProc << "\", \""
          <<  epd.inFlow << "/" << apd.outFlow << "\") {\n";
      oss << event_oss.str()     << std::endl
          << condition_oss.str() << std::endl
          << action_oss.str()    << std::endl;
      if (epd.inputs && apd.outputs) {
        if (doAggwrap) {
          oss << "\t" << aggwrap.str(); // Create the aggwrap
          oss << "\tinput -> Queue(\"aggQueue\", 10000) -> PullPush(\"aggInput\", 0) ->";
          oss << "\taggwrap[1] -> event -> condition -> ";
          oss << "PullPush(\"aggpp\", 0) -> [1]aggwrap -> ";
          oss << "Queue(\"actionBuf\", 10000) -> action -> output;\n";
          oss << "\tcondition[1] -> [2]aggwrap;\n};\n"; // End signal
        }
        else {
          oss << "\tinput -> event -> condition -> action -> output;\n};\n";
        }
        oss << "edit main { " << intStrandInputElement << " -> "; 
        oss << graphName << " -> [+]" << intStrandOutputElement << "; };\n";
      }
      else if (epd.inputs) {
        if (doAggwrap) {
          TELL_ERROR << "Side affect rule should not have aggregation. RULE: " 
                     << (*rule)[catalog->attribute(RULE, "NAME")]->toString() << std::endl; 
          throw planner::Exception("Side effect rule should not have aggregate!");
        }
        else {
          oss << "\tinput -> event -> condition -> action;\n};\n";
        }
        oss << "edit main { " << intStrandInputElement << " -> " 
            << graphName << "; };\n";
      }
      else if (apd.outputs) {
        oss << "\tevent -> condition -> action -> output;\n};\n";
	if(eventType == "RECV"){
          throw planner::Exception("Receive event should have an input!");
	} else{
	  oss << "edit main { " << graphName
	      << " -> [+]" << extStrandOutputElement<<"; };\n";
	}
      }
      else if (actionType == "DROP") {
        oss << "\tevent -> condition -> action;\n};\n";
      }
      else {
        throw planner::Exception("Disconnected rule should not be planned!");
      }

 
      /** Update the rule to include the P2DL description */
      CommonTablePtr ruleTbl = catalog->table(RULE);
      TuplePtr     newRuleTp = rule->clone();
      newRuleTp->set(catalog->attribute(RULE, "P2DL"), Val_Str::mk(oss.str()));
      newRuleTp->freeze();
      ruleTbl->insert(newRuleTp);
 
      TELL_INFO << rule->toString() << std::endl;
      TELL_INFO << "=============  RULE PLAN  ===============\n" 
                << oss.str() << std::endl;
    }
    
    /**
     * Create a list of the rule terms, starting with the head and event,
     * and ending with the probes, selections, projections in the position 
     * order.
     */
    void
    Context::ruleTerms(CommonTable::ManagerPtr catalog, 
                       TuplePtr rule, TuplePtrList& terms)
    {
      CommonTablePtr functorTbl = catalog->table(FUNCTOR);
      CommonTablePtr assignTbl  = catalog->table(ASSIGN);
      CommonTablePtr selectTbl  = catalog->table(SELECT);
  
      CommonTable::Key functorKey;
      CommonTable::Key assignKey;
      CommonTable::Key selectKey;

      functorKey.push_back(catalog->attribute(FUNCTOR, "RID"));
      functorKey.push_back(catalog->attribute(FUNCTOR, "POSITION"));

      assignKey.push_back(catalog->attribute(ASSIGN, "RID"));
      assignKey.push_back(catalog->attribute(ASSIGN, "POSITION"));

      selectKey.push_back(catalog->attribute(SELECT, "RID"));
      selectKey.push_back(catalog->attribute(SELECT, "POSITION"));

      TELL_INFO << "PROCESS RULE: " << rule->toString() << std::endl;
      CommonTable::Iterator iter;
      for (unsigned pos = 0; true; pos++) {
        TuplePtr lookup = Tuple::mk();
        lookup->append((*rule)[TUPLE_ID]);
        lookup->append(Val_Int64::mk(pos));
        lookup->freeze();

        TuplePtr lookup2 = Tuple::mk();
        lookup2->append((*rule)[TUPLE_ID]);
        lookup2->append(Val_Int64::mk(pos));
        lookup2->freeze();

        TuplePtr lookup3 = Tuple::mk();
        lookup3->append((*rule)[TUPLE_ID]);
        lookup3->append(Val_Int64::mk(pos));
        lookup3->freeze();

        TELL_INFO << "LOOKUP TUPLE: " << lookup->toString() << std::endl;
        iter = functorTbl->lookup(CommonTable::theKey(CommonTable::KEY01), functorKey, lookup);
        if (iter->done())
          iter = functorTbl->lookup(CommonTable::theKey(CommonTable::KEY01), functorKey, lookup2);
        if (iter->done())
          iter = functorTbl->lookup(CommonTable::theKey(CommonTable::KEY01), functorKey, lookup3);

        if (!iter->done()) {
          TuplePtr term = iter->next();
          TELL_INFO << "\tFUNCTOR TERM POSITION " << pos << ": " 
                    << term->toString() << std::endl;
          terms.push_back(term);  // A condition
          continue;
        }

        iter = assignTbl->lookup(CommonTable::theKey(CommonTable::KEY01), assignKey, lookup);
        if (iter->done())
          iter = assignTbl->lookup(CommonTable::theKey(CommonTable::KEY01), assignKey, lookup2);
        if (iter->done())
          iter = assignTbl->lookup(CommonTable::theKey(CommonTable::KEY01), assignKey, lookup3);
        if (!iter->done()) {
          TuplePtr term = iter->next();
          TELL_INFO << "\tASSIGN TERM POSITION " << pos << ": " 
                    << term->toString() << std::endl;
          terms.push_back(term);  // An assignment
          continue;
        }
        iter = selectTbl->lookup(CommonTable::theKey(CommonTable::KEY01), selectKey, lookup);
        if (iter->done())
          iter = selectTbl->lookup(CommonTable::theKey(CommonTable::KEY01), selectKey, lookup2);
        if (iter->done())
          iter = selectTbl->lookup(CommonTable::theKey(CommonTable::KEY01), selectKey, lookup3);
        if (!iter->done()) {
          TuplePtr term = iter->next();
          TELL_INFO << "\tSELECT TERM POSITION " << pos << ": " 
                    << term->toString() << std::endl;
          terms.push_back(term);  // A selection
          continue;
        }
        break;  // Done.
      }
    }
  
    /**
     * Creates and returns a P2DL string reprsenting the event portion of
     * the rule terms.  */
    Context::PortDesc
    Context::event(ostringstream& oss, string indent, TuplePtr rule,
                   CommonTable::ManagerPtr catalog, TuplePtrList& terms)
    {
      assert (terms.size() >= 2);	// Must have a least a head and an event
      TuplePtr head  = terms.at(0);
      TuplePtr event = terms.at(1); 
      string   eventType = (*event)[catalog->attribute(FUNCTOR, "ECA")]->toString();
      string   eventName = (*event)[catalog->attribute(FUNCTOR, "NAME")]->toString();
      ListPtr  headArgs  = Val_List::cast((*head)[catalog->attribute(FUNCTOR, "ATTRIBUTES")]); 

      if (eventName != "periodic") {
        ListPtr  schema = Val_List::cast((*event)[catalog->attribute(FUNCTOR, "ATTRIBUTES")]); 
        schema = canonicalizeSelection(terms, schema);
        event = event->clone();
        event->set(catalog->attribute(FUNCTOR, "ATTRIBUTES"), Val_List::mk(schema));
        event->freeze();
        terms[1] = event;
      }

      if (eventType == "RECV") {
        oss << indent << "graph event(1, 1, \"h/l\", \"-/-\") {\n";
        oss << indent << "\tinput -> " << "Queue(\"" << eventName << "\", 1000) -> \n"; 
        if (watched(eventName, "c")) {
          oss << indent << "\tPrint(\"RecvEvent: RULE "
              << (*rule)[catalog->attribute(RULE, "NAME")]->toString() << "\") ->\n"; 
        }
        oss << indent << "\toutput;\n};\n";
        return (Context::PortDesc) {1, "h", "-", 1, "l", "-"};
      }
      else if (eventType == "INSERT") {
        return insertEvent(oss, indent, rule, catalog, head, event);
      } 
      else if (eventType == "REFRESH") {
        oss << indent << "graph event(0, 1, \"/l\", \"/-\") {\n";
        oss << indent << "\tRefresh(\"refresh_" << eventName << "\", \"" << eventName << "\") -> \n";
        if (watched(eventName, "r")) {
          oss << indent << "\tPrint(\"RefreshEvent: RULE "
              << (*rule)[catalog->attribute(RULE, "NAME")]->toString() << "\") ->\n"; 
        }
        oss << indent << "\toutput;\n";
        oss << indent << "};\n";
        return (Context::PortDesc) {0, "", "", 1, "l", "-"};
      }
      else if (eventType == "DELETE") {
        oss << indent << "graph event(0, 1, \"/l\", \"/-\") {\n";
        oss << indent << "\tRemoved(\"delete_" << eventName << "\", \"" << eventName << "\") -> \n";
        if (watched(eventName, "d")) {
          oss << indent << "\tPrint(\"DeleteEvent: RULE "
              << (*rule)[catalog->attribute(RULE, "NAME")]->toString() << "\") ->\n"; 
        }
        oss << indent << "\toutput;\n";
        oss << indent << "};\n";
        return (Context::PortDesc) {0, "", "", 1, "l", "-"};
      }
      else if (eventType == "DELTA_DELETE") {
        oss << indent << "graph event(0, 1, \"/l\", \"/-\") {\n";
        oss << indent << "\tRemoved(\"delete_"<<eventName<<"\",\""<<eventName<<"\") -> ";
        if (watched(eventName, "d")) {
          oss << "Print(\"DeleteEvent: RULE "
              << (*rule)[catalog->attribute(RULE, "NAME")]->toString() << "\") -> "; 
        }
        oss << indent << "output;\n};\n";
        return (Context::PortDesc) {0, "", "", 1, "l", "-"};
      }
      else if (eventType == "DELTA_INSERT") {
        oss << indent << "graph event(0, 1, \"/l\", \"/-\") {\n";
        oss << indent << "\trr = RoundRobin(\"deltaRR_" << eventName << "\", 2);\n";
        oss << indent << "\tUpdate(\"update_"<<eventName<<"\",\""<<eventName<<"\") -> ";
        if (watched(eventName, "i")) {
          oss << "Print(\"InsertEvent: RULE "
              << (*rule)[catalog->attribute(RULE, "NAME")]->toString() << "\") -> "; 
        }
        oss << "[0]rr;\n";
        oss << indent << "\tRefresh(\"refresh_"<<eventName<<"\",\""<<eventName<< "\") -> ";
        if (watched(eventName, "r")) {
          oss << "Print(\"RefreshEvent: RULE "
              << (*rule)[catalog->attribute(RULE, "NAME")]->toString() << "\") -> "; 
        }
        oss << "[1]rr;\n";
        oss << indent << "\trr -> output;\n};\n";
        return (Context::PortDesc) {0, "", "", 1, "l", "-"};
      }
      else {
        throw planner::Exception("Unknown event type: " + eventType);
      }
      return (Context::PortDesc) {0, "", "", 0, "", ""};
    }
  
    /**
     * Creates and returns a P2DL string representing the probe, selection, and
     * assignment terms in a rule.
     */
    Context::PortDesc
    Context::condition(ostringstream& oss, string indent, TuplePtr rule,
                       CommonTable::ManagerPtr catalog, TuplePtrList& terms)
    {
      TuplePtr head       = terms.at(0); 
      TuplePtr event      = terms.at(1); 
      ListPtr headSchema  = Val_List::cast((*head)[catalog->attribute(FUNCTOR, "ATTRIBUTES")]);
      int     aggPosition = namestracker::aggregation(headSchema);
      bool    filter      = false;
  
      if (aggPosition >= 0 &&
          (*event)[catalog->attribute(FUNCTOR, "ECA")]->toString() == "RECV") {
        oss << indent << "graph condition(1, 2, \"l/lh\", \"-/--\") {\n";  
        filter = true;
      }
      else {
        oss << indent << "graph condition(1, 1, \"l/l\", \"-/-\") {\n";  
      }

      ostringstream termName;
      termName << "terms_" << _nameCounter++;
      // The tuple schema begins with the event schema... Compute it.
      if (filter) {
        oss << indent << "\tgraph " << termName.str() << "(1, 2, \"l/lh\", \"-/--\") {\n";  
      }
      else {
        oss << indent << "\tgraph " << termName.str() << "(1, 1, \"l/l\", \"-/-\") {\n";  
      }

      ListPtr tupleSchema = Val_List::cast((*event)[catalog->attribute(FUNCTOR, "ATTRIBUTES")])->clone();
      /** Get rid of any non-variable attributes associated with periodic. */
      if ((*event)[catalog->attribute(FUNCTOR, "NAME")]->toString() == "periodic") {
        while ((*Val_Tuple::cast(tupleSchema->back()))[TNAME]->toString() != VAR &&
               (*Val_Tuple::cast(tupleSchema->back()))[TNAME]->toString() != LOC) {
          tupleSchema->pop_back();
        }
      }

      headSchema = canonicalizeAssign(terms, headSchema);
      head = head->clone();
      head->set(catalog->attribute(FUNCTOR, "ATTRIBUTES"), Val_List::mk(headSchema));

      std::deque<string> graphNames;
      string filterGraphName = "";
      for (uint i = 2; i < terms.size(); i++) {
        TuplePtr term = terms.at(i);
        string graphName;
        if ((*term)[0]->toString() == FUNCTOR) {
          ListPtr schema = Val_List::cast((*term)[catalog->attribute(FUNCTOR, "ATTRIBUTES")]);
          schema = canonicalizeSelection(terms, schema);
          term = term->clone();
          term->set(catalog->attribute(FUNCTOR, "ATTRIBUTES"), Val_List::mk(schema));
          graphName = probe(oss, indent + "\t\t", catalog, term, tupleSchema, filter);
          if ((*term)[catalog->attribute(FUNCTOR, "NOTIN")] == Val_Int64::mk(false)) {
              tupleSchema = namestracker::merge(tupleSchema, schema);
          }
          if (filter) 
            filterGraphName = graphName;
          filter  = false;
        }
        else if ((*term)[0]->toString() == ASSIGN) {
          graphName = assign(oss, indent + "\t\t", catalog, term, tupleSchema);
        }
        else if ((*term)[0]->toString() == SELECT) {
          graphName = select(oss, indent + "\t\t", catalog, term, tupleSchema);
        }
        else
          throw planner::Exception("Unknown term type: " + (*term)[1]->toString());
        graphNames.push_back(graphName);
      }

      if (aggPosition >= 0 && filterGraphName != "") {
        oss << indent << "\t\t" << filterGraphName << "[1] -> [1]output;\n";
      } 

      oss << indent << "\t\tinput";
      for (std::deque<string>::const_iterator i = graphNames.begin();
           i != graphNames.end(); i++) {
        oss << " -> " << *i; 
      }
      oss << indent << " -> output;\n";
      oss << indent << "\t};\n";
  
      // Ensure that the tuple schema conforms to the head schema
      TELL_INFO << "HEAD SCHEMA FOR HEAD "
                << (*head)[catalog->attribute(FUNCTOR, "NAME")]->toString() << std::endl
                << "\t" << headSchema->toString() << std::endl
                << "\tTUPLE SCHEMA IS " << tupleSchema->toString() << std::endl;
      ostringstream pel;
      pel << "\\\"" << (*head)[catalog->attribute(FUNCTOR, "NAME")]->toString() << "\\\" pop ";
      for (ValPtrList::const_iterator iter = headSchema->begin(); 
           iter != headSchema->end(); iter++) {
        ValuePtr arg = *iter;
        TuplePtr argTp = Val_Tuple::cast(arg);

        if ((*argTp)[0]->toString() == AGG &&
            (*event)[catalog->attribute(FUNCTOR, "ECA")]->toString() == "INSERT") {
          /* An Aggregation element is planned in this case, which will always place
             the aggregate value in the last position. */
          pel << "$" << (tupleSchema->size() + 1) << " pop ";
        }
        else {
          int pos = namestracker::position(tupleSchema, arg);
          if (pos >= 0) {
            pel << "$" << (pos + 1) << " pop ";
          }
          else if ((*argTp)[0]->toString() == AGG && (*argTp)[2] == Val_Null::mk()) {
            /* This field is handled by the Aggwrap element. */
          }
          else {
            throw planner::Exception("ERROR Rule: " +
                                     (*rule)[catalog->attribute(RULE, "NAME")]->toString() +
                                     "\n\tUnknown variable: " + 
                                      arg->toString() + "\n\tSCHEMA: " + 
                                      tupleSchema->toString());
          }
        }
      }
      TELL_INFO << "\tPEL: " << pel.str() << std::endl;

      oss << indent << "\tgraph project(1, 1, \"a/a\", \"x/x\") {\n";
      oss << indent << "\t\tinput -> "
                    << "PelTransform(\"project\", \"" << pel.str() << "\") -> ";
      if (watched((*head)[catalog->attribute(FUNCTOR, "NAME")]->toString(), "h")) {
          oss << "Print(\"HeadProjection: RULE "
              << (*rule)[catalog->attribute(RULE, "NAME")]->toString() << "\") -> "; 
      }
      oss << "output;\n";
      oss << indent << "\t};\n";
  
      oss << indent << "\tinput -> " << termName.str() << " -> project -> output;\n";
      if (aggPosition >= 0 && filterGraphName != "") {
        oss << indent << "\t\t" << termName.str() << "[1] -> [1]output;\n";
      } 
      oss << indent << "};\n";
      return (Context::PortDesc) {1, "a", "x", 1, "a", "x"};
    }
  
    /**
     * Creates and returns a P2DL string represeting the final projection
     * onto the head schema and the head action.
     * Invariant: The schema of the input tuple conforms to the head.
     *            The input is a single pull input.
     */
    Context::PortDesc
    Context::action(ostringstream& oss, string indent, TuplePtr rule,
                    CommonTable::ManagerPtr catalog, TuplePtrList& terms)
    {
      TuplePtr head       = terms.at(0); 
      string   funcName   = (*head)[catalog->attribute(FUNCTOR, "NAME")]->toString();
      string   actionType = (*head)[catalog->attribute(FUNCTOR, "ECA")]->toString();
  
      if (actionType == "DROP") {
        oss << indent << "graph action(1, 0, \"l/\", \"-/\") {\n";
        oss << indent << "\tinput -> PullPush(\"actionPull\", 0) -> Discard(\"drop\");\n";
        oss << indent << "};\n";
        return (Context::PortDesc) {1, "l", "-", 0, "", ""};
      }
      else if (actionType == "ADD") {
        oss << indent << "graph action(1, 0, \"l/\", \"-/\") {\n";
        oss << indent << "\tinput -> PullPush(\"actionPull\", 0) ->\n";
        if (watched(funcName, "a")) {
          oss << indent << "\tPrint(\"AddAction: RULE "
              << (*rule)[catalog->attribute(RULE, "NAME")]->toString() << "\") ->\n"; 
        }
        oss << indent << "\tInsert2(\"actionInsert\", \"" << funcName << "\");\n";
        oss << indent << "};\n";
        return (Context::PortDesc) {1, "l", "-", 0, "", ""};
      }
      else if (actionType == "DELETE") {
        oss << indent << "graph action(1, 0, \"l/\", \"-/\") {\n";
        oss << indent << "\tinput -> PullPush(\"actionPull\", 0) ->\n";
        if (watched(funcName, "z")) {
          oss << indent << "\tPrint(\"DeleteAction: RULE "
              << (*rule)[catalog->attribute(RULE, "NAME")]->toString() << "\") ->\n"; 
        }
        oss << indent << "\tDelete2(\"actionDelete\", \"" << funcName << "\");\n";
        oss << indent << "};\n";
        return (Context::PortDesc) {1, "l", "-", 0, "", ""};
      }
      else if (actionType == "SEND") {
        ostringstream package;

        /** Find where the location attribute falls in the tuple.
         *  The locaction will be extracted from the tuple, via pel, and placed in
         *  position 0 followed by an encapsulation of the orginal tuple. */ 
        ListPtr headSchema = Val_List::cast((*head)[catalog->attribute(FUNCTOR, "ATTRIBUTES")]);
        int pos = namestracker::position(headSchema, namestracker::location(headSchema));
        if (pos < 0)
          throw planner::Exception("No location variable in schema: " + headSchema->toString());

        package << "$" << (pos + 1);   // Location attribute position
        package << " pop swallow pop"; // Encapsulate the tuple in a value type field.
         
        oss << indent << "graph action(1, 1, \"a/a\", \"x/x\") {\n";
        oss << indent << "\tinput -> ";
        if (watched(funcName, "s")) {
          oss << indent << "\tPrint(\"SendAction: RULE "
              << (*rule)[catalog->attribute(RULE, "NAME")]->toString() << "\") ->\n"; 
        }
        oss << indent << "PelTransform(\"actionPel\", \"" << package.str() << "\") -> output;\n};\n";
        return (Context::PortDesc) {1, "l", "-", 1, "l", "-"};
      }
      else
        throw planner::Exception("Unknown head action type: " + head->toString());
      return (Context::PortDesc) {0, "", "", 0, "", ""};
    }
  
    Context::PortDesc
    Context::insertEvent(ostringstream& oss, string indent, TuplePtr rule,
                         CommonTable::ManagerPtr catalog, TuplePtr head, TuplePtr event) 
    {
      ListPtr headArgs  = Val_List::cast((*head)[catalog->attribute(FUNCTOR, "ATTRIBUTES")]);
      ListPtr eventArgs = Val_List::cast((*event)[catalog->attribute(FUNCTOR, "ATTRIBUTES")]);
      string  eventName = (*event)[catalog->attribute(FUNCTOR, "NAME")]->toString();
      int   aggPosition = namestracker::aggregation(headArgs);
  
      if (aggPosition >= 0) {
        string aggTable = eventName;
        string aggOper  = "";
        int    aggField = -1;
        CommonTable::Key groupByFields;
        for (ValPtrList::const_iterator iter = headArgs->begin();
             iter != headArgs->end(); iter++) {
          TuplePtr arg = Val_Tuple::cast(*iter);
          if ((*arg)[0]->toString() == VAR || (*arg)[0]->toString() == LOC) {
            int pos = namestracker::position(eventArgs, *iter);
            if (pos < 0) continue;
            groupByFields.push_back(pos+1);          
          }
          else {
            aggOper  = (*arg)[3]->toString();
            if ((*arg)[2] == Val_Null::mk()) {
              aggField = 0; 
            }
            else {
              aggField = namestracker::position(eventArgs, *iter) + 1;
              if (!aggField)
                throw Exception("Aggregation field must be over table predicate!\nRule Head: " +
                                head->toString() + ". Rule Event: " + event->toString());
            }
          }
        }
          
        oss << indent << "graph event(0, 1, \"/l\", \"/-\") {\n";
        oss << indent << "\t"
            << "agg = Aggregate(\"" << aggOper << "_" << aggTable 
            << "\", \"" << aggTable << "\", \"" << aggOper << "\", " << aggField 
            << ", " << int_list_to_str(groupByFields) << ");\n";
        oss << indent << "\tagg -> ";
        oss << "PelTransform(\"tableName\", \"\\\"" << aggTable
            << "\\\" pop swallow unbox popall\") -> "; 
        if (watched(eventName, "i")) {
          oss << indent << "\tPrint(\"InsertEvent: RULE "
              << (*rule)[catalog->attribute(RULE, "NAME")]->toString() << "\") ->\n"; 
        }
        oss << indent << "\toutput;\n};\n";
        return (Context::PortDesc) {0, "", "", 1, "l", "-"};
      }
      else if (eventName == "periodic") {
        ValPtrList::const_iterator i = eventArgs->begin();
        i++; i++;  // Move to the period argument
        TuplePtr ptp  = Val_Tuple::cast(*i);
        TuplePtr ctp  = eventArgs->size() > 3 ? Val_Tuple::cast(*(++i)) : TuplePtr();
        string period = (*ptp)[2]->toString();
        string count  = ctp ? (*ctp)[2]->toString() : "0";
  
        oss << indent << "graph event(0, 1, \"/l\", \"/-\") {\n";
        oss << indent << "\tsource = StaticTupleSource(\"periodicSource\", periodic<\""
                      << Plumber::catalog()->nodeid()->toString() << "\">);\n";
        oss << indent << "\tpel = PelTransform(\"periodicPel\", \"$0 pop $1 pop rand pop\");\n";
        oss << indent << "\ttimer = TimedPullPush(\"periodicTimer\", ";
        oss << period << ", " << count << ");\n";
        oss << indent << "\tsource -> pel -> timer -> Slot(\"periodicSlot\") -> output;\n";
        oss << indent << "};\n";
        return (Context::PortDesc) {0, "", "", 1, "l", "-"};
      }
      else {
        oss << indent << "graph event(0, 1, \"/l\", \"/-\") {\n";
        oss << indent << "\tUpdate(\"update_" << eventName << "\", \"" 
                      << eventName << "\") -> \n";
        if (watched(eventName, "i")) {
          oss << indent << "\tPrint(\"InsertEvent: RULE "
              << (*rule)[catalog->attribute(RULE, "NAME")]->toString() << "\") ->\n"; 
        }
        oss << indent << "\toutput;\n};\n";
        return (Context::PortDesc) {0, "", "", 1, "l", "-"};
      }
      return (Context::PortDesc) {0, "", "", 0, "", ""};
    }
  
    ListPtr
    Context::canonicalizeAssign(TuplePtrList& assigns, ListPtr schema)
    {
      static long fictNum = 0;

      ListPtr cschema = List::mk();
      for (ValPtrList::const_iterator iter = schema->begin(); 
           iter != schema->end(); iter++) {
        TuplePtr attr = Val_Tuple::cast(*iter);
        if ((*attr)[TNAME]->toString() != VAR &&
            (*attr)[TNAME]->toString() != LOC &&
            (*attr)[TNAME]->toString() != AGG) {
          ostringstream name;
          name <<"$CSELECT_" << fictNum++;
          TuplePtr varAttr = Tuple::mk(VAR);
          varAttr->append(Val_Str::mk(name.str()));
          varAttr->freeze();
          
          TuplePtr assignTp = Tuple::mk(ASSIGN, true);
          assignTp->append(Val_Null::mk());
          assignTp->append(Val_Tuple::mk(varAttr));
          assignTp->append(Val_Tuple::mk(attr));
          assignTp->append(Val_Null::mk());
          assignTp->freeze();
 
          assigns.push_back(assignTp);
          cschema->append(Val_Tuple::mk(varAttr));
        }
        else {
          cschema->append(Val_Tuple::mk(attr));
        }
      } 
      return cschema;
    }

    ListPtr
    Context::canonicalizeSelection(TuplePtrList& selections, ListPtr schema)
    {
      static long fictNum = 0;

      ListPtr cschema = List::mk();
      for (ValPtrList::const_iterator iter = schema->begin(); 
           iter != schema->end(); iter++) {
        TuplePtr attr = Val_Tuple::cast(*iter);
        if ((*attr)[TNAME]->toString() != VAR &&
            (*attr)[TNAME]->toString() != LOC) {
          ostringstream name;
          name << "$PVEVENTGEN_" << fictNum++;
          TuplePtr varAttr = Tuple::mk(VAR);
          varAttr->append(Val_Str::mk(name.str()));
          varAttr->freeze();
          
          TuplePtr boolTp = Tuple::mk(BOOL);
          boolTp->append(Val_Str::mk("=="));
          boolTp->append(Val_Tuple::mk(varAttr));
          boolTp->append(Val_Tuple::mk(attr));
          boolTp->freeze();

          TuplePtr selectTp = Tuple::mk(SELECT, true);
          selectTp->append(Val_Null::mk());
          selectTp->append(Val_Tuple::mk(boolTp));
          selectTp->append(Val_Null::mk());
          selectTp->freeze();
          selections.push_back(selectTp);
 
          cschema->append(Val_Tuple::mk(varAttr));
        }
        else {
          cschema->append(Val_Tuple::mk(attr));
        }
      } 

      return cschema;
    }

    string
    Context::probe(ostringstream& oss, string indent,
                   CommonTable::ManagerPtr catalog, 
                   TuplePtr probeTp, ListPtr tupleSchema, bool filter)
    {
      ListPtr probeSchema  = Val_List::cast((*probeTp)[catalog->attribute(FUNCTOR, "ATTRIBUTES")]);
      string    tableName  = (*probeTp)[catalog->attribute(FUNCTOR, "NAME")]->toString();
      bool    notin = (*probeTp)[catalog->attribute(FUNCTOR, "NOTIN")] == Val_Int64::mk(true);
      CommonTable::Key joinKey;
      CommonTable::Key indexKey;
      CommonTable::Key baseKey;
      namestracker::joinKeys(tupleSchema, probeSchema, joinKey, indexKey, baseKey);
      catalog->createIndex(tableName, HASH_INDEX, indexKey);

      // Create a unique graph name for this probe
      ostringstream graphName;
      graphName << "probe_" << _nameCounter++;
 
      ostringstream pelProject;
      pelProject << "$0 0 field pop "; // Pop the original tuple name.
      for (uint k = 0; k < tupleSchema->size(); k++)
        pelProject << "$0 " << (k+1) << " field pop ";
 
      if (!notin) {
    	  /* Okay add the probe schema to the output schema */
    	  for (CommonTable::Key::iterator iter = baseKey.begin();
    	  		iter != baseKey.end(); iter++)
    		  pelProject << "$1 " << *iter << " field pop "; 
      }
  
      oss << std::endl;
      if (filter) {
        oss << indent << "graph " << graphName.str() << "(1, 2, \"l/lh\", \"-/--\") {\n";
        oss << indent << "\tnoNullSignal = NoNullSignal(\"nns\");\n";
        oss << indent << "\tnoNullSignal[1] -> [1]output;\n";
        oss << indent << "\tlookup = Lookup2(\"probeLookup\", \"" << tableName << "\", "
            << int_list_to_str(joinKey) << ", " << int_list_to_str(indexKey) << ");\n"; 
        oss << indent << "\tinput -> PullPush(\"tpp\", 0) -> lookup -> noNullSignal ->\n";
        if (notin) {
        	oss << "OnlyNullField(\"onlyNull\", 1) -> ";
        }
        else {
        	oss << "NoNullField(\"noNull\", 1) -> ";
        }
        oss << indent << "\tPelTransform(\"probeProject\", \"" << pelProject.str() << "\") ->";
        oss << "output;\n";
        oss << indent << "};\n";
      }
      else {
        oss << indent << "graph " << graphName.str() << "(1, 1, \"l/l\", \"-/-\") {\n";
        oss << indent << "\tlookup = Lookup2(\"probeLookup\", \"" << tableName << "\", "
            << int_list_to_str(joinKey) << ", " << int_list_to_str(indexKey) << ");\n"; 
        oss << indent << "\tinput -> PullPush(\"tpp\", 0) -> lookup -> NoNull(\"filter\") -> ";
        if (notin) {
        	oss << "OnlyNullField(\"onlyNull\", 1) -> ";
        }
        else {
        	oss << "NoNullField(\"noNull\", 1) -> ";
        }

        oss << indent << "\tPelTransform(\"probeProject\", \"" << pelProject.str() << "\") -> ";
        oss << "output;\n";
        oss << indent << "};\n";
      }

      return graphName.str();
    } 
  
    string
    Context::assign(ostringstream& oss, string indent,
                    CommonTable::ManagerPtr catalog, 
                    TuplePtr asmt, ListPtr &tupleSchema)
    {
      // Create a unique graph name for this assignment
      ostringstream graphName;
      ValuePtr var = (*asmt)[catalog->attribute(ASSIGN, "VAR")];
      if (namestracker::position(tupleSchema, var) < 0) {
        tupleSchema->append(var);
      }
      graphName << "assign_" << _nameCounter++;

      oss << std::endl;
      oss << indent << "graph " << graphName.str() << "(1, 1, \"a/a\", \"-/-\") {\n";
      oss << indent << "\tinput -> PelTransform(\"assignment\", \"$0 pop "
                    << pel::gen(tupleSchema, asmt) << "\") -> output;\n";
      oss << indent << "};\n";

      return graphName.str();
    } 
  
    string
    Context::select(ostringstream& oss, string indent,
                    CommonTable::ManagerPtr catalog, 
                    TuplePtr selection, ListPtr tupleSchema)
    {
      // Create a unique graph name for this selection
      ostringstream graphName;
      graphName << "select_" << _nameCounter++;

      oss << std::endl;
      oss << indent << "graph " << graphName.str() << "(1, 1, \"a/a\", \"-/-\") {\n";
      oss << indent << "\tinput -> "
                    << "PelTransform(\"selection\", \""
                    << pel::gen(tupleSchema, selection) << "\") -> "
                    << "output;\n";
      oss << indent << "};\n";

      return graphName.str();
    } 

  }
}
