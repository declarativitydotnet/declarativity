// -*- c-basic-offset: 2; related-file-name: "ol_context.h" -*-
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
 * DESCRIPTION: P2 Catalog
 *
 */

#include "catalog.h"
#include "val_uint32.h"
#include "tuple.h"


str Catalog::TableInfo::toString() {
  strbuf t;
  t << "TABLE<" << _tableInfo->toString() << ">";

  return t;
}

str Catalog::nextQueryID() {  
  str toRet;
  strbuf sbuf;
  sbuf << _queryID;
  _queryID ++;
  return str(sbuf);
}

void Catalog::createTable(OL_Context::TableInfo* tableInfo)
{
  if (tableInfo->timeout != 0) { 
      warn << "Create table " << tableInfo->toString() << "\n";
      // if timeout is zero, table is never materialized 
      size_t tableSize;
      if (tableInfo->size != -1) {
	tableSize = tableInfo->size;
      } else {
	tableSize = UINT_MAX; // consider this infinity
      }
      TablePtr newTable(new Table(tableInfo->tableName, tableInfo->size));
      if (tableInfo->timeout != -1) {
	timespec* expiration = new timespec();
	expiration->tv_sec = tableInfo->timeout;
	expiration->tv_nsec = 0;
	newTable.reset(new Table(tableInfo->tableName, tableInfo->size, expiration));
      }

      TableInfo* newTableInfo = new TableInfo(tableInfo, newTable);
      tables->insert(std::make_pair(tableInfo->tableName, newTableInfo));
      
      // first create unique indexes
      std::vector<int> primaryKeys = tableInfo->primaryKeys;
      for (uint k = 0; k < primaryKeys.size(); k++) {
	newTable->add_unique_index(primaryKeys.at(k));
	warn << "Create index " << tableInfo->tableName 
	     << " " << primaryKeys.at(k) << "\n";
      } 
    }
}

void Catalog::initTables(OL_Context* ctxt)
{
  OL_Context::TableInfoMap::iterator theIterator;
  for (theIterator = ctxt->getTableInfos()->begin(); 
       theIterator != ctxt->getTableInfos()->end(); theIterator++) {
    OL_Context::TableInfo* tableInfo = theIterator->second;
    createTable(tableInfo);  
  }
}

Catalog::TableInfo* Catalog::getTableInfo(str tableName)
{
  Catalog::TableInfoMap::iterator theIterator;
  theIterator = tables->find(tableName);
  if (theIterator == tables->end()) {
    return NULL;
  }
  return theIterator->second;
}

void Catalog::createMultIndex(str tableName, int key) {
    TableInfo* ti = getTableInfo(tableName);
    if (ti->isSecondaryKey(key)) { return; }
    ti->_table->add_multiple_index(key);
    ti->secondaryKeys.push_back(key);
  }


bool Catalog::TableInfo::isPrimaryKey(int c) 
{
  for (unsigned k = 0; k < _tableInfo->primaryKeys.size(); k++) {
    if (c == _tableInfo->primaryKeys.at(k)) {
      return true;
    }
  }
  return false;
}

bool Catalog::TableInfo::isSecondaryKey(int c) 
{
  for (unsigned k = 0; k < secondaryKeys.size(); k++) {
    if (c == secondaryKeys.at(k)) {
      return true;
    }
  }
  return false;
}


