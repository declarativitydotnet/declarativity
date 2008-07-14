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
 * DESCRIPTION: Planner code
 *
 */

#ifndef __PLANNER_C__
#define __PLANNER_C__


#include "planner.h"
#include "planContext.h"
#include "rulePlanner.C"
#include "stageRegistry.h"

Planner::Planner(DataflowPtr conf, TableStore* tableStore, 
		 bool debug, string nodeID) 
  : _conf(conf)
{ 
  _tableStore = tableStore; 
  _debug = debug; 
  _nodeID = nodeID;
  _ruleCount = 0;

  _netPlanner = new NetPlanner(conf, nodeID); 
}

std::vector< RuleStrand* > 
Planner::generateRuleStrands(ECA_ContextPtr ectxt)
{
  std::vector<RuleStrand*> toRet;
  // go through each eca rule, form a rule strand
  for (unsigned k = 0; k < ectxt->getRules().size(); k++) {
    ECA_Rule* nextRule = ectxt->getRules().at(k);
    ostringstream b; b << _ruleCount++ << "-" << _nodeID;
    RuleStrand *rs = new RuleStrand(nextRule, b.str());
    toRet.push_back(rs);
    PlanContext* pc = new PlanContext(_conf, _tableStore, rs, 
				      _nodeID);
    compileECARule(pc);
    delete pc;    
  }
  return toRet;
}


std::vector< StageStrand* >
Planner::generateStageStrands(OL_Context* ctxt)
{
  std::vector< StageStrand* > toRet;

  OL_Context::ExternalStageSpecMap aStageSpecMap =
    ctxt->getExtStagesInfo();

  for (OL_Context::ExternalStageSpecMap::const_iterator it =
         aStageSpecMap.begin();
       it != aStageSpecMap.end();
       it++) {
    std::ostringstream strandIDoss;
    strandIDoss << _ruleCount++
                << "-"
                << _nodeID;
    std::string strandID = strandIDoss.str();

    try {
      StageStrand* ss = generateStageStrand(it->second,
                                            strandID);
      toRet.push_back(ss);
    } catch (StageRegistry::StageNotFound snf) {
      // Couldn't create this stage. Skip it and report failure.
      PLANNER_INFO_NOPC("Planner: could not find external stage "
                        << "named "
                        << snf.stageName
                        << ". Skipping.");
    }
  }

  return toRet;
}


StageStrand*
Planner::generateStageStrand(const OL_Context::ExtStageSpec* aSpec,
                             string strandID)
{
  // Stages are always pull in pull out. But rule strands start out push
  // (right after the duplicator), so we need a slot, then the stage,
  // then the name changer (PelTransform), and we're done.
  StageStrand* strand = new StageStrand(aSpec,
                                        strandID);
  strand->addElement(_conf,
                     ElementPtr(new Slot(aSpec->stageName + "!Slot")));
  strand->addElement(_conf,
                     ElementPtr(new Stage(aSpec->stageName + "!Stage",
                                          aSpec->stageName)));
  strand->addElement(_conf,
                     ElementPtr(new PelTransform(aSpec->stageName + "!Stage",
                                                 "swallow " // push
                                                            // entire
                                                            // input
                                                            // into the
                                                            // stack
                                                 "unbox "   // turn
                                                            // swallowed
                                                            // tuple
                                                            // into indiv
                                                            // fields
                                                 "drop "    // drop
                                                            // old
                                                            // name
                                                 "\""
                                                 + aSpec->outputTupleName
                                                 + "\" pop " // new name
                                                 + " popall" // rest
                                                 )));

  generateSendAction(_tableStore,
                     _nodeID,
                     strand,
                     aSpec,
                     _conf);
  
  return strand;
}


void
Planner::setupNetwork(boost::shared_ptr<Udp> udp)
{
  // call the netplanner to generate network in and out
  // mux and demux not generated
  PLANNER_INFO_NOPC("Planner: set up network elements");
  _netPlanner->generateNetworkElements(udp);
}


void
Planner::registerAllRuleStrands(std::vector<RuleStrand*> ruleStrands,
                                std::vector<StageStrand*> stageStrands)
{
  PLANNER_INFO_NOPC("Planner: register " << ruleStrands.size()
                    << " rule strands with network planner");

  // call netplanner to create the right mux/demuxes
  _netPlanner->registerAllRuleStrands(ruleStrands, stageStrands);
}



#endif
