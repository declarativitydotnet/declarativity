// -*- c-basic-offset: 2; related-file-name: "" -*-
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
 * DESCRIPTION: Overlog based on new planner
 *
 */

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "ol_lexer.h"
#include "ol_context.h"
#include "eca_context.h"
#include "localize_context.h"
#include "plmb_confgen.h"
#include "tableStore.h"
#include "udp.h"
#include "planner.h"
#include "ruleStrand.h"
#include "reporting.h"

#include "dot.h"

extern int ol_parser_debug;

int main(int argc, char **argv)
{
  PlumberPtr plumber(new Plumber());
  TELL_INFO << "TestPlanner\n";
  boost::shared_ptr< OL_Context > ctxt(new OL_Context());
  bool route = false;
  bool builtin = false;
  string filename("");

  //PlumberPtr plumber(new Plumber());




  // Parse command line options
  int c;
  while ((c = getopt(argc, argv, "r:dhcpf:g")) != -1) {
    switch (c) {
    case 'f':
      {
        if (optarg == "-") {
          ctxt->parse_stream(&std::cin);
        } else {
          filename = optarg;
          std::ifstream istr(filename.c_str());
          ctxt->parse_stream(&istr);
        }
      }
      break;
    case 'r':
      {
        // My minimum reporting level is optarg
        std::string levelName(optarg);
        Reporting::Level level =
          Reporting::levelFromName[levelName];
        Reporting::setLevel(level);
      }
      break;
    case 'd':
      {
        ol_parser_debug = 1;
      }
      break;
    case 'h':
      TELL_ERROR << "Usage: " << argv[0] << "{option}*\n"
                << "\t-c: canonical form (used for builtin tests)\n"
                << "\t-d: turn on parser debugging\n"
                << "\t-p: try to instantiate a plumber config\n"
                << "\t-g: produce a DOT graph spec\n"
                << "\t-h: print this help text\n"
                << "\t-r <reporting level> : Set reporting level\n"
                << "\t-f <filename> : "-" means read from stdin\n";
      exit(0);
      break;
    case 'c':
      builtin = true;
      break;
    case 'p':
      route = true;
      builtin = false;
      break;
    case 'g':
      {
      Plumber::DataflowPtr conf(new Plumber::Dataflow("overlog"));
      filename = optarg;
      std::ifstream istr(filename.c_str());
      ctxt->parse_stream(&istr);

      boost::shared_ptr< TableStore > tableStore(new TableStore(ctxt.get()));
      tableStore->initTables();

      boost::shared_ptr< Localize_Context > lctxt(new Localize_Context());
      lctxt->rewrite(ctxt.get(), tableStore.get());

      boost::shared_ptr< ECA_Context > ectxt(new ECA_Context());
      ectxt->rewrite(lctxt.get(), tableStore.get());

      boost::shared_ptr< Planner > planner(new Planner(conf,
                                                       tableStore.get(),
                                                       false,
                                                       "127.0.0.1:10000"));
      boost::shared_ptr< Udp > udp(new Udp("Udp", 12345));

      std::vector<RuleStrand*>
      ruleStrands = planner->generateRuleStrands(ectxt);

      planner->setupNetwork(udp);
      planner->registerAllRuleStrands(ruleStrands);

      if (plumber->install(conf) == 0) {
        TELL_INFO << "Correctly initialized network of reachability flows.\n";
      } else {
        TELL_ERROR << "** Failed to initialize correct spec\n";
      }
      plumber->toDot("overlog.dot");
      exit (0);
      }
      break;
    default:
      std::cerr << "Unrecognized option.\n";
      break;
    }
  }



  if (builtin) {
    TELL_INFO << ctxt->toString();
    TELL_INFO << "OK\n";
  } 

  TELL_INFO << "Finish parsing (functors / tableInfos) " 
	    << ctxt->getRules()->size() 
	    << " " << ctxt->getTableInfos()->size() << "\n";

  TELL_INFO << ctxt->toString() << "\n";

  if (route) {
    Plumber::DataflowPtr conf(new Plumber::Dataflow("overlog"));
    boost::shared_ptr< TableStore > tableStore(new TableStore(ctxt.get()));  
    tableStore->initTables(); 

    FILE* strandOutput = fopen((filename + ".strand").c_str(), "w");
    FILE* ecaOutput = fopen((filename + ".eca").c_str(), "w");
    FILE* localOutput = fopen((filename + ".local").c_str(), "w");

    boost::shared_ptr< Localize_Context > lctxt(new Localize_Context()); 
    lctxt->rewrite(ctxt.get(), tableStore.get());
    fprintf(localOutput, lctxt->toString().c_str());
    fclose(localOutput);
    
    boost::shared_ptr< ECA_Context > ectxt(new ECA_Context()); 
    ectxt->rewrite(lctxt.get(), tableStore.get());
    fprintf(ecaOutput, ectxt->toString().c_str());
    fclose(ecaOutput);
    
    boost::shared_ptr< Planner > planner(new Planner(conf,
                                                     tableStore.get(),
                                                     false,
                                                     "127.0.0.1:10000"));
    boost::shared_ptr< Udp > udp(new Udp("Udp", 12345));
    std::vector<RuleStrand*> ruleStrands = planner->generateRuleStrands(ectxt);
    
    for (unsigned k = 0; k < ruleStrands.size(); k++) {
      TELL_INFO << ruleStrands.at(k)->toString();
      fprintf(strandOutput, ruleStrands.at(k)->toString().c_str());
    }
    fclose(strandOutput);
    
    planner->setupNetwork(udp);
    planner->registerAllRuleStrands(ruleStrands);
    TELL_INFO << planner->getNetPlanner()->toString() << "\n";

    if (plumber->install(conf) == 0) {
      TELL_INFO << "Correctly initialized network of reachability flows.\n";
    } else {
      TELL_ERROR << "** Failed to initialize correct spec\n";
    }    
    TELL_INFO << "Done.\n";

  }

  return 0;
}
