// -*- c-basic-offset: 2; related-file-name: "ol_context.h" -*-
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
 * DESCRIPTION: P2 Catalog
 *
 */

#include "tableStore.h"
#include "val_uint32.h"
#include "tuple.h"
#include "loop.h"

TableStore::TableStore(OL_Context* ctxt) { 
  _ctxt = ctxt;
  _tables = new TableMap(); 
  _tableInfos = ctxt->getTableInfos();
};


CommonTablePtr TableStore::getTableByName(string tableName)
{
  TableMap::iterator _iterator = _tables->find(tableName);
  if (_iterator == _tables->end()) {
    std::cerr << "ERROR: table " << tableName << " not found.\n";
    exit(-1) ;
  }
  return _iterator->second;
}

void TableStore::createTable(OL_Context::TableInfo* tableInfo)
{
  // What's my expiration? -1 in the inputs means no expiration.
  boost::posix_time::time_duration expiration = tableInfo->timeout;
  
  // What's my size? -1 in the inputs means no size
  uint32_t tableSize = tableInfo->size;
  
  // What's my primary key?
  Table2::Key key = tableInfo->primaryKeys;
  
  // What's the table name. This is unique across nodes
  string newTableName = tableInfo->tableName;
  
  // Create the table. 
  CommonTablePtr newTable;
  if (expiration == Table2::NO_EXPIRATION) {
    newTable.reset(new RefTable(tableInfo->tableName,
				key));
    std::cout << "Create ref counted table " << tableInfo->toString() << "\n";
  } else {
    newTable.reset(new Table2(tableInfo->tableName,
			      key,
			      tableSize,
			      expiration));
    std::cout << "Create table " << tableInfo->toString() << "\n";
  }

  _tables->insert(std::make_pair(tableInfo->tableName, newTable));
  
  // Now handle facts
  for (unsigned int k = 0; k < _ctxt->getFacts().size(); k++) {
    TuplePtr tr = _ctxt->getFacts().at(k);
    ValuePtr vr = (*tr)[0];
    std::cout << "Insert tuple " << tr->toString()
              << " into table "  << vr->toString()
              << " " << tr->size() << "\n";
    CommonTablePtr tableToInsert = getTableByName(vr->toString());     
    tableToInsert->insert(tr);
    std::cout << "Tuple inserted: " << tr->toString() 
	      << " into table " << vr->toString() 
	      << " " << tr->size() << "\n";
  }
}

void TableStore::initTables()
{
  OL_Context::TableInfoMap::iterator theIterator;
  for (theIterator = _ctxt->getTableInfos()->begin(); 
       theIterator != _ctxt->getTableInfos()->end(); theIterator++) {
    OL_Context::TableInfo* tableInfo = theIterator->second;
    createTable(tableInfo);  
  }
}

bool TableStore::checkSecondaryIndex(string uniqStr) {
  if (_secondaryIndices.find(uniqStr) == _secondaryIndices.end()) {
    _secondaryIndices.insert(std::make_pair(uniqStr, uniqStr));
    return false;
  }
  return true;
}

OL_Context::TableInfo* TableStore::getTableInfo(string tableName)
{
  OL_Context::TableInfoMap::iterator theIterator;
  theIterator = _tableInfos->find(tableName);
  if (theIterator == _tableInfos->end()) {
    return NULL;
  }
  return theIterator->second;
}

