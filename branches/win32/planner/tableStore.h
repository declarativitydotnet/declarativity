// -*- c-basic-offset: 2; related-file-name: "tableStore.C" -*-
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
#include "commonTable.h"
#include "table2.h"
#include "refTable.h"

class TableStore
{
public:
  /* Table information */
  typedef std::map<string, CommonTablePtr >  TableMap;
 
  TableStore(OL_Context* ctxt);
  ~TableStore() { delete _tables; }

  OL_Context::WatchTableType getWatchTables() { return _ctxt->getWatchTables(); };
  OL_Context::TableInfoMap* getTableInfos()  { return _tableInfos;   };


  /** Creates an empty table for a given table definition */
  void
  createTable(OL_Context::TableInfo* ti);


  /** Creates empty tables for all tables definitions recorded so far */
  void
  initTables();


  OL_Context::TableInfo* getTableInfo(string tableName);
  void addTableInfo(OL_Context::TableInfo* ti) {  
    _tableInfos->insert(std::make_pair(ti->tableName, ti));
  };
  CommonTablePtr getTableByName(string tableName);
  bool checkSecondaryIndex(string uniqueStr);

private:
  std::map<string, string> _secondaryIndices;
  OL_Context::TableInfoMap* _tableInfos;
  TableMap* _tables;
  OL_Context* _ctxt;
};

#endif
