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

#ifndef _SQLITE_SAVE_STAGE_H_
#define _SQLITE_SAVE_STAGE_H_

#include "stage.h"
#include "val_str.h"
#include "stageRegistry.h"

// forward declaration
struct sqlite3;
struct sqlite3_stmt;

/** 
 * A class that allows simple logging using SQLite.
 */
class SQLiteSaveStage : public Stage::Processor {
public:
  SQLiteSaveStage(Stage* myStage);

  ~SQLiteSaveStage();

  /** 
   * The input schema for SQLiteSaveStage is as follows:
   * The first tuple is 
   *    <tupleName, location specifier, filename, tablename, columns...>
   * The following tuples are
   *    <tupleName, location_specifier, column values...>
   */
  void newInput(TuplePtr inputTuple);

  /** 
   * The stage does not produce any output.
   */
  std::pair<TuplePtr, Stage::Status> newOutput();

  // This is necessary for the class to register itself with the
  // factory.
  DECLARE_PUBLIC_STAGE_INITS(SQLiteSaveStage)

private:
  void createDatabase(TuplePtr tuple);
  void saveRow(TuplePtr tuple);

  //! A pointer to the database (initially NULL).
  sqlite3* db;

  //! A prepared statement for tuple insertion
  sqlite3_stmt* stmt;

  //! True if the the first initialization tuple has been received
  bool initialized;

  //! The number of entries
  size_t num_columns;

  //! The current location specifier
  ValuePtr _locationSpecifier;

  //! The name of output tuples
  // static ValuePtr SQLiteSave;

  // This is necessary for the class to register itself with the
  // factory.
  DECLARE_PRIVATE_STAGE_INITS
};

#endif
