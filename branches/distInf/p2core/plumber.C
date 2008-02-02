// -*- c-basic-offset: 2; related-file-name: "plumber.h" -*-
/*
 * @(#)$Id$
 *
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 * DESCRIPTION: The plumber class, for creating dataflow graphs
 * 
 */


#include <plumber.h>
#include <set>
#include "table2.h"
#include "reporting.h"
#include "tableManager.h"
#include "commitManager.h"
#include "scheduler.h"
#include "loop.h"
#include "dot.h"


CommonTable::ManagerPtr Plumber::_catalog(new TableManager());
std::map<string, DataflowPtr > Plumber::_dataflows;
Scheduler Plumber::_scheduler(new CommitManager());





/********************************************************
 * Plumber is responsible for maintaining a set of dataflows
 * each defined by their configuration. 
 */

Plumber::Plumber()
  : _logger(0)
{
}

Scheduler* Plumber::scheduler() {
  return &_scheduler;
}

/**
 * install
 */
int Plumber::install(DataflowPtr d)
{
  if (d->validate() < 0 || d->initialize() < 0) {
    TELL_ERROR << "** Dataflow installation failure\n";
    return -1;
  }

  std::map<string, DataflowPtr>::iterator iter = _dataflows.find(d->name());
  if (iter != _dataflows.end()) {
    _dataflows.erase(iter);
  }
  _dataflows.insert(std::make_pair(d->name(), d));

  return 0;
}

#include <sstream>

void Plumber::toDot(string f) { 
  TELL_INFO << "Outputting DOT for dataflow into file "
            << f
            << "\n";
  std::set<ElementSpec::HookupPtr> hookups;
  std::set<ElementSpecPtr> elements;

  for (std::map<string, DataflowPtr>::iterator iter = _dataflows.begin();
       iter != _dataflows.end(); iter++) {
    std::vector<ElementSpec::HookupPtr> h = iter->second->hookups_;
    std::vector<ElementSpecPtr>         e = iter->second->elements_;
    for (unsigned i = 0; i < e.size(); i++) {
      elements.insert(e[i]);
    }
    for (unsigned i = 0; i < h.size(); i++) {
      hookups.insert(h[i]);
    }
  }

  std::ofstream ostr(f.c_str());
  std::ostringstream buf;

  ::toDot(&buf, elements, hookups);
  ostr<<buf.str();
}

REMOVABLE_INLINE LoggerI * Plumber::logger(LoggerI * newLogger)
{
  LoggerI * l = _logger;
  _logger = newLogger;
  return l;
}



DataflowEditPtr
Plumber::edit(string name)
{ 
  DataflowPtr dpt = dataflow(name);
  if (dpt == 0) {
    // Doesn't already exist. Create a new one.
    return DataflowEditPtr();
  }
  return DataflowEditPtr(new DataflowEdit(dpt)); 
}


DataflowPtr
Plumber::dataflow(string n)
{
  std::map<string, DataflowPtr>::const_iterator i =
    _dataflows.find(n); 
  return (i == _dataflows.end()) ? DataflowPtr() : i->second; 
}


