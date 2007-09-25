/*
 * Copyright (c) 2005 Intel Corporation. All rights reserved.
 *
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 */

#include "tableManager.h"
#include "plumber.h"
#include "systemTable.h"
#include "refTable.h"
#include "table2.h"
#include "val_str.h"
#include "val_time.h"
#include "val_int64.h"
#include "val_list.h"
#include "val_null.h"
#include "list.h"

#define PRIMARY   "primary"
#define SECONDARY "secondary"

TableManager::ForeignKeyHandler::ForeignKeyHandler(CommonTablePtr src, 
                                                   CommonTable::Key& fk, 
                                                   CommonTablePtr dest)
: _sourceTable(src), _foreignKey(fk), _reference(dest)
{

}

void TableManager::ForeignKeyHandler::removeListener(TuplePtr tp)
{

}

void TableManager::ForeignKeyHandler::insertListener(TuplePtr tp)
{

}

/* Create and initialize all system tables.  */
TableManager::TableManager() 
{
  try {
    initialize();
  } catch (TableManager::Exception& e) {
    TELL_ERROR << "TableManager::TableManager exception: " 
              << e.toString() << std::endl;
  }
}

/* Clean up all tables.  */
TableManager::~TableManager()
{
}

void TableManager::initialize() 
{
#define TABLEDEF(name, key, schema) \
if (name == TABLE || name == INDEX) { \
  _tables.insert(std::make_pair(name, CommonTablePtr(new RefTable(name, key)))); \
  registerTable(name, boost::posix_time::time_duration(boost::date_time::pos_infin), 0, key); \
}
#include "systemTable.h"

#define TABLEDEF(name, key, schema) \
if (name == TABLE || name == INDEX) { \
  registerIndex(name, key, PRIMARY); \
}
#include "systemTable.h"

#define TABLEDEF(name, key, schema) \
if (name != TABLE && name != INDEX) createTable((name), (key));
#include "systemTable.h"

#define SCHEMA(name, pos) do {\
  attributes.push_back(Val_Str::mk(name)); \
  attributes.push_back(Val_Int64::mk(pos)); \
} while(0);
#define TABLEDEF(name, key, schema) do {\
  CommonTablePtr attrTbl = table(ATTRIBUTE); \
  std::deque<ValuePtr> attributes; \
  schema \
  for (std::deque<ValuePtr>::iterator i = attributes.begin(); \
       i != attributes.end(); i += 2) { \
    TuplePtr attrTp = Tuple::mk(ATTRIBUTE); \
    attrTp->append(Val_Int64::mk(uniqueIdentifier())); \
    attrTp->append(Val_Str::mk(name)); \
    attrTp->append(*i); \
    attrTp->append(*(i+1)); \
    attrTp->freeze(); \
    attrTbl->insert(attrTp); \
  } \
} while(0);
#include "systemTable.h"

#define SECONDARY_INDEX(table, key) createIndex(table, key);
#include "systemTable.h"

#define FOREIGN_KEY(src, key, dest) do { \
  createForeignKey(src, key, dest); \
  createIndex(dest, key); \
} while(0);
#include "systemTable.h"

#define FUNCTIONDEF(name, numargs, pel) do { \
  TuplePtr func = Tuple::mk(FUNCTION); \
  func->append(Val_Int64::mk(uniqueIdentifier())); \
  func->append(Val_Str::mk((name))); \
  func->append(Val_Int64::mk((numargs))); \
  func->append(Val_Str::mk((pel))); \
  func->freeze(); \
  assert(table(FUNCTION)->insert(func)); \
} while(0);
#include "systemTable.h"
}

string
TableManager::toString() const
{
  ostringstream oss;
  CommonTablePtr tableTbl = table(TABLE);
  if (tableTbl) {
    CommonTable::Iterator tIter = tableTbl->scan();
    TuplePtr tuple;
    while ((tuple = tIter->next())) {
      oss << tuple->toString() << std::endl; 
      CommonTablePtr refTbl = table((*tuple)[attribute(TABLE, "TABLENAME")]->toString());
      assert (refTbl);
      CommonTable::Iterator rIter = refTbl->scan();
      while ((tuple = rIter->next()))
        oss << "\t" << tuple->toString() << std::endl; 
    }
  }
  return oss.str();
}

ValuePtr
TableManager::nodeid()
{
  CommonTablePtr nodeidTbl = table(NODEID);
  CommonTable::Iterator i = nodeidTbl->scan();
  if (!i->done()) {
    TuplePtr nodeid = i->next();
    return (*nodeid)[attribute(NODEID, "ADDRESS")];
  }
  return ValuePtr();
}

unsigned
TableManager::uniqueIdentifier()
{
  static unsigned identifier = 0;
  return identifier++;
}

TuplePtr 
TableManager::createTable(string name, CommonTable::Key& key)
{
  CommonTablePtr t = table(name);
  if (t) throw TableManager::Exception("Table already exists! " + name); 
  _tables.insert(std::make_pair(
                 name, CommonTablePtr(new RefTable(name, key))));

  TuplePtr tp = 
    registerTable(name, boost::posix_time::time_duration(boost::date_time::pos_infin), 
                  0, key);
  registerIndex(name, key, PRIMARY);
  return tp;
}

TuplePtr 
TableManager::createTable(string name, CommonTable::Key& key, uint32_t maxSize,
                          boost::posix_time::time_duration& lifetime)
{
  CommonTablePtr t = table(name);
  if (t) throw TableManager::Exception("Table already exists! " + name); 
  _tables.insert(std::make_pair(
                 name, CommonTablePtr(new Table2(name, key, maxSize, lifetime))));

  TuplePtr tp = registerTable(name, lifetime, maxSize, key);
  registerIndex(name, key, PRIMARY);
  return tp;
}

TuplePtr 
TableManager::createTable(string name, CommonTable::Key& key, uint32_t maxSize, 
                          string lifetime)
{
  CommonTablePtr t = table(name);
  if (t) throw TableManager::Exception("Table already exists! " + name); 
  _tables.insert(std::make_pair(
                 name, CommonTablePtr(new Table2(name, key, maxSize, lifetime))));

  TuplePtr tp = registerTable(name, boost::posix_time::duration_from_string(lifetime), 
                              maxSize, key);
  registerIndex(name, key, PRIMARY);
  return tp;
}
  
TuplePtr 
TableManager::createTable(string name, CommonTable::Key& key, uint32_t maxSize)
{
  CommonTablePtr t = table(name);
  if (t) throw TableManager::Exception("Table already exists! " + name); 
  _tables.insert(std::make_pair(
                 name, CommonTablePtr(new Table2(name, key, maxSize))));

  TuplePtr tp = 
    registerTable(name, boost::posix_time::time_duration(boost::date_time::pos_infin), 
                  maxSize, key);
  registerIndex(name, key, PRIMARY);
  return tp;
}

void
TableManager::createIndex(string tableName, CommonTable::Key& key)
{

  /** First check if the index has already been created */
  CommonTablePtr indexTbl = table(INDEX);
  ListPtr keys = List::mk();
  for (CommonTable::Key::iterator iter = key.begin();
       iter != key.end(); iter++) 
    keys->append(Val_Int64::mk(*iter));

  TuplePtr tp = Tuple::mk(INDEX);
  tp->append(Val_Int64::mk(uniqueIdentifier()));
  tp->append(Val_Str::mk(tableName));
  tp->append(Val_List::mk(keys));

  CommonTable::Iterator iter = 
    indexTbl->lookup(CommonTable::theKey(CommonTable::KEY34), tp);
  if (iter->done()) { 
    /** Index does not exist, create one now */
    CommonTablePtr t = table(tableName);  // Get the table
    if (!t) {
      throw TableManager::Exception("Index creation failure on non existent table: " + 
                                    tableName);
    } 
    else if (t->secondaryIndex(key)) {
      registerIndex(tableName, key, SECONDARY);  // All is well, register new index
    }
  }
}

void
TableManager::createForeignKey(string src, CommonTable::Key& fk, string dest)
{
  TableManager::ForeignKeyHandlerPtr 
    handler(new TableManager::ForeignKeyHandler(table(src), fk, table(dest)));

  TableManager::ForeignKeyMap::iterator iter; 
  for (iter = _foreignKeys.lower_bound(src); 
       iter != _foreignKeys.upper_bound(src); iter++) {
      if (*iter->second == *handler) { 
        throw TableManager::Exception("Foreign Key " + src + " -> " + dest + " already exists.");
      }
  }
  _foreignKeys.insert(std::make_pair(src, handler)); 
}


/** 
 * Drops table from table map, clears out table and index entries
 * in system tables.
 */
bool
TableManager::dropTable(string tableName)
{
  CommonTablePtr t = table(tableName);

  return true;
}

bool
TableManager::dropIndex(string tableName, CommonTable::Key& key)
{
  CommonTablePtr tbl = table(tableName);
  CommonTablePtr ind = table(INDEX);

  return true;
}

CommonTablePtr
TableManager::table(string name) const
{
  TableMap::const_iterator iter = _tables.find(name);
  return (iter == _tables.end()) ? CommonTablePtr() : iter->second;
}

int
TableManager::attribute(string tablename, string attrname) const
{
  CommonTablePtr attrTbl = table(ATTRIBUTE);
  TuplePtr tp = Tuple::mk(); 
  tp->append(Val_Str::mk(tablename));
  tp->append(Val_Str::mk(attrname));
  CommonTable::Iterator iter = 
    attrTbl->lookup(CommonTable::theKey(CommonTable::KEY01),
                    CommonTable::theKey(CommonTable::KEY34), tp);
  return iter->done() ? -1 : Val_Int64::cast((*iter->next())[5]); 
}

int
TableManager::attributes(string tablename) const
{
  CommonTablePtr attrTbl = table(ATTRIBUTE);
  TuplePtr tp = Tuple::mk(); 
  tp->append(Val_Str::mk(tablename));
  CommonTable::Iterator 
    iter = attrTbl->lookup(CommonTable::theKey(CommonTable::KEY0), 
                           CommonTable::theKey(CommonTable::KEY3), tp);
  uint size = 0;
  while (iter->next()) size++;
  return size; 
}

void
TableManager::relation(string name, string table1, 
                       string table2, CommonTable::Key& key)
{
  CommonTablePtr tp1 = table(table1);
  CommonTablePtr tp2 = table(table2);

  if (tp1 && tp2) {
    CommonTable::Key pk1 = tp1->primaryKey();
    CommonTable::Key pk2 = tp2->primaryKey();
    if (pk1.size() + pk2.size() == key.size()) {
      CommonTable::Key *fk1 = new CommonTable::Key();
      CommonTable::Key *fk2 = new CommonTable::Key();
      CommonTable::Key::iterator iter = key.begin();
      for (uint k=0; k < pk1.size(); fk1->push_back(*iter), iter++, k++);
      for (uint k=0; k < pk2.size(); fk2->push_back(*iter), iter++, k++);
      createTable(name, key);
      createForeignKey(name, *fk1, table1); 
      createForeignKey(name, *fk2, table2); 
      return;
    }
    TELL_ERROR << "RELATION 2 ERROR: KEY SIZE MISMATCH: "
               << "Primary key combination size: " << (pk1.size()+pk2.size()) 
               << ", given key size " << key.size() << std::endl;
  }
  throw TableManager::Exception("RELATION 2 ERROR: TABLE -- " + 
                                table1 + ", " + table2);
}

TuplePtr 
TableManager::registerTable(string name, boost::posix_time::time_duration lifetime,
                            uint size, CommonTable::Key& primaryKey)
{
  CommonTablePtr t = table(TABLE); assert(t);
  TuplePtr tp = Tuple::mk(TABLE);
  tp->append(Val_Int64::mk(uniqueIdentifier()));
  tp->append(Val_Str::mk(name));
  tp->append(Val_Time_Duration::mk(lifetime));
  tp->append(Val_Int64::mk(size));

  ListPtr keys = List::mk();
  for (CommonTable::Key::const_iterator iter = primaryKey.begin();
       iter != primaryKey.end(); iter++) {
    keys->append(Val_Int64::mk(*iter));
  }
  tp->append(Val_List::mk(keys));
  tp->append(Val_Int64::mk(0));
  tp->freeze();
 
  t->insert(tp);
  return tp;
}

void TableManager::registerIndex(string tableName, CommonTable::Key& key, string type)
{
  CommonTablePtr index = table(INDEX); assert(index);

  ListPtr keys = List::mk();
  for (CommonTable::Key::iterator iter = key.begin();
       iter != key.end(); iter++) 
    keys->append(Val_Int64::mk(*iter));

  TuplePtr tp = Tuple::mk(INDEX);
  tp->append(Val_Int64::mk(uniqueIdentifier()));
  tp->append(Val_Str::mk(tableName));
  tp->append(Val_List::mk(keys));
  tp->append(Val_Str::mk(type));
  tp->freeze();
  index->insert(tp);
}
