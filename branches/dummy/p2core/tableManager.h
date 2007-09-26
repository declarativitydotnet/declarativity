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
 * DESCRIPTION: 
 * The manager class object creates and maintains
 * all tables defined in a running P2 instance. Tables
 * create outside the scope of the system Manager will
 * not be accessible to the system.
 * Primarily used by the compile for table resolution and
 * creation.
 * All system tables are defined at object creation.
 *
 */

#ifndef __TABLEMANAGER_H__
#define __TABLEMANAGER_H__

#include "commonTable.h"


class TableManager : public CommonTable::Manager {
public:
  class Exception {
  public:
    Exception(string m) : msg(m) {};

    string toString() const { return "TableManager Exception: " + msg; };

  private:
    string msg;
  };

  TableManager();
  ~TableManager();

  virtual string
  toString() const;

  virtual ValuePtr
  nodeid();

  virtual unsigned
  uniqueIdentifier();

  /**
   * Get table reference if it exists
   * Return: Table reference or empty pointer if not exists.
   */
  virtual CommonTablePtr 
  table(string name) const;

  virtual int
  attribute(string tablename, string attrname) const;

  virtual int
  attributes(string tablename) const;

  /**
   * Creates and registers a new Table with the system.
   * Return: A RefTable instance if table does not exist
   * Throws: TableManager::Exception if table already exists
   */
  virtual TuplePtr 
  createTable(string name, CommonTable::Key& key);

  /**
   * Creates and registers a new Table with the system.
   * Return: Table2 instance with specified maxSize and lifetime
   * Throws: TableManager::Exception if table already exists
   */
  virtual TuplePtr 
  createTable(string name, CommonTable::Key& key, uint32_t maxSize,
              boost::posix_time::time_duration& lifetime);

  /**
   * Creates and registers a new Table with the system.
   * Return: Table2 instance with specified maxSize and lifetime
   * Throws: TableManager::Exception if table already exists
   */
  virtual TuplePtr 
  createTable(string name, CommonTable::Key& key, uint32_t maxSize, 
              string lifetime);
  
  /**
   * Creates and registers a new Table with the system.
   * Return: Table2 instance with specified maxSize and infinite lifetime
   *         TableManager::Exception if table already exists
   */
  virtual TuplePtr 
  createTable(string name, CommonTable::Key& key, uint32_t maxSize);

  /**
   * Create and registers a secondary index on specified table name
   * Throws: TableManager::Exception if table does not exist.
   */
  virtual void
  createIndex(string tableName, CommonTable::Key& key);

  /**
   * Create foreign key relationship from table 'src' on 
   * key attributes 'fk', to the primaryKey of table 'dest'.
   * Throws: TableManager::Exception upon error
   */
  virtual void
  createForeignKey(string src, CommonTable::Key& fk, string dest);

  /**
   * Drops the secondary index on specified table name
   * Return: true  -- if new secondary index is dropped.
   *         false -- if index does not exists.
   */
  virtual bool
  dropIndex(string tableName, CommonTable::Key& key);

  /**
   * Drop the table from the system.
   * Return: true  -- if table was dropped or does not exist.
   *         false -- if table was not dropped. 
   *                  i.e., can't drop a system table.
   */
  virtual bool
  dropTable(string tableName);

private:
  void initialize();

  TuplePtr registerTable(string name, boost::posix_time::time_duration lifetime,
                         uint size, CommonTable::Key& primaryKey);
  void registerIndex(string tableName, CommonTable::Key& key, string type);
  void relation(string name, string table1, string table2, 
                CommonTable::Key& key);

  typedef std::map<string, CommonTablePtr> TableMap;
  TableMap _tables;

  class ForeignKeyHandler {
  public:
    ForeignKeyHandler(CommonTablePtr src, 
                      CommonTable::Key& fk, 
                      CommonTablePtr dest);
    const CommonTable::Key& key() const { return _foreignKey; };

    bool operator==(ForeignKeyHandler &other) const
    { return other._sourceTable->name() == _sourceTable->name() &&
             other._reference->name()   == _reference->name()   &&
             other._foreignKey == _foreignKey; }
  private:
    void removeListener(TuplePtr tp);
    void insertListener(TuplePtr tp);

    CommonTablePtr   _sourceTable;
    CommonTable::Key _foreignKey;
    CommonTablePtr   _reference;
  };
  typedef boost::shared_ptr<ForeignKeyHandler>        ForeignKeyHandlerPtr;
  typedef std::multimap<string, ForeignKeyHandlerPtr> ForeignKeyMap;
  ForeignKeyMap _foreignKeys;
};

#endif
