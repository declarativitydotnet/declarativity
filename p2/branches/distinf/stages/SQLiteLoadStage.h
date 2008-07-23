/*
 * @(#)$$
 *
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 * Author: Stanislav Funiak
 * DESCRIPTION: A stage that can load items from a PRL XML archive
 *
 */

#ifndef _SQLITE_LOAD_STAGE_H_
#define _SQLITE_LOAD_STAGE_H_

#include "stage.h"
#include "val_str.h"
#include "stageRegistry.h"

// forward declaration
struct sqlite3;
struct sqlite3_stmt;

/** 
 * A class that reads tuples from an SQLite table.
 */
class SQLiteLoadStage : public Stage::Processor {
public:
  SQLiteLoadStage(Stage* myStage);

  ~SQLiteLoadStage();

  /** 
   * The input schema for SQLiteLoadStage is as follows:
   * The first tuple is 
   *    <tupleName, location specifier, filename, tablename [, columns...]>
   * If no columns are specified, the stage reads all columns in their
   *    natural order
   */
  void newInput(TuplePtr inputTuple);

  /** 
   * The output schema is <tupleName, location specifier, [object ...]>
   */
  std::pair<TuplePtr, Stage::Status> newOutput();

  // This is necessary for the class to register itself with the
  // factory.
  DECLARE_PUBLIC_STAGE_INITS(SQLiteLoadStage)

private:
  TuplePtr loadRow();

  //! A pointer to the database (initially NULL).
  sqlite3* db;

  //! A prepared statement for the query
  sqlite3_stmt* stmt;

  //! The current location specifier
  ValuePtr _locationSpecifier;

  //! The name of output tuples
  // static ValuePtr SQLiteSave;

  // This is necessary for the class to register itself with the
  // factory.
  DECLARE_PRIVATE_STAGE_INITS
};

#endif
