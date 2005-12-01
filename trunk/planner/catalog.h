// -*- c-basic-offset: 2; related-file-name: "ol_context.C" -*-
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
 * DESCRIPTION: Planner Catalog
 *
 */

#ifndef __PL_CATALOG_H__
#define __PL_CATALOG_H__

#include <vector>
#include <map>
#include <set>
#include "value.h"
#include "tuple.h"
#include "ol_context.h"
#include "table.h"

class Catalog 
{
public:
  /* Table information */
  class TableInfo {
  public:
    TableInfo(OL_Context::TableInfo* tableInfo, TableRef table) :
      _tableInfo(tableInfo), _table(table) { }

    str toString();

    OL_Context::TableInfo * _tableInfo;    
    TableRef _table;
    std::vector<int> secondaryKeys;
    bool isPrimaryKey(int c);
    bool isSecondaryKey(int c);    
  };
  
  typedef std::map<str, Catalog::TableInfo* >  TableInfoMap;
 
  /* Query information */
  class QueryInfo {
    str queryID;
    // more..
  };

  Catalog() { _queryID = 0; tables = new TableInfoMap(); };
  ~Catalog() { delete tables; }

  str nextQueryID(); 

  void setWatchTables(std::set<str> watchTables) { _watchTables = watchTables; }
  std::set<str> getWatchTables() { return _watchTables; };
  TableInfoMap* getTableInfos()  { return tables;   };
  void initTables(OL_Context* ctxt);
  void createTable(OL_Context::TableInfo* ti);
  Catalog::TableInfo* getTableInfo(str tableName);
  void createMultIndex(str tableName, int key);
 
  
private:
  TableInfoMap* tables;
  int _queryID;
  std::set<str> _watchTables;
};

#endif
