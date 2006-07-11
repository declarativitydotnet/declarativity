// -*- c-basic-offset: 2; related-file-name: "ol_context.C" -*-
/*
 * @(#)$Id$
 *
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 * DESCRIPTION: References to all tables
 *
 */

#ifndef __PL_TABLESTORE_H__
#define __PL_TABLESTORE_H__

#include <vector>
#include <map>
#include <set>
#include "value.h"
#include "tuple.h"
#include "ol_context.h"
#include "table2.h"

class TableStore
{
public:
  /* Table information */
  typedef std::map<string, Table2Ptr >  TableMap;
 
  TableStore(OL_Context* ctxt);
  ~TableStore() { delete _tables; }

  std::set<string> getWatchTables() { return _ctxt->getWatchTables(); };
  OL_Context::TableInfoMap* getTableInfos()  { return _tableInfos;   };
  void initTables();
  void createTable(OL_Context::TableInfo* ti);
  OL_Context::TableInfo* getTableInfo(string tableName);
  void addTableInfo(OL_Context::TableInfo* ti) {  
    _tableInfos->insert(std::make_pair(ti->tableName, ti));
  };
  Table2Ptr getTableByName(string tableName);
  bool checkSecondaryIndex(string uniqueStr);

private:
  std::map<string, string> _secondaryIndices;
  OL_Context::TableInfoMap* _tableInfos;
  TableMap* _tables;
  OL_Context* _ctxt;
};

#endif
