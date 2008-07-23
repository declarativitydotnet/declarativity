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
 * DESCRIPTION: A stage that can save tuples to a SQLite file
 *
 */

#include "SQLiteSaveStage.h"
#include <sqlite3.h>

#include "val_str.h"
#include "val_int64.h"
#include "val_double.h"
#include "val_time.h"
#include "val_factor.h"

#include <prl/xml_oarchive.hpp>
#include <prl/factor/xml/polymorphic_factor.hpp>


SQLiteSaveStage::SQLiteSaveStage(Stage* myStage) 
  : Stage::Processor(myStage), db(NULL), stmt(NULL), initialized(false) {
}


SQLiteSaveStage::~SQLiteSaveStage() {
  if(stmt) sqlite3_finalize(stmt);
  if(db) sqlite3_close(db);
}

std::pair< TuplePtr, Stage::Status >
SQLiteSaveStage::newOutput()
{
  return std::make_pair(TuplePtr(), Stage::DONE);
}

void SQLiteSaveStage::createDatabase(TuplePtr tuple) {
  assert(db == NULL);
  if (tuple->size() < 5) {
    // Insufficient number of fields
    STAGE_WARN("newInput: tuple " 
               << tuple
               << " doesn't appear to have enough fields");
  } else {
    // Remember the location specifier
    _locationSpecifier = (*tuple)[1];
    
    // Extract the filename and the table name
    std::string fileName  = Val_Str::cast((*tuple)[2]);
    std::string tableName = Val_Str::cast((*tuple)[3]);

    // Form the list of columns and values
    num_columns = tuple->size() - 4;
    std::string columns = " (";
    std::string values = " (";
    for (size_t i = 0; i < num_columns; ++i) {
      if (i) columns += ", ";
      columns += Val_Str::cast((*tuple)[i+4]);
      if (i) values += ", ";
      values += '?';
    }
    columns += ')';
    values += ')';
    
    // Open the database
    int rc = sqlite3_open(fileName.c_str(), &db);
    if (rc) {
      STAGE_WARN("newInput: failed to open file "
                 << fileName
                 << ".");
      return;
    }

    // Create the table
    std::string cmd = "CREATE TABLE " + tableName + columns;
    rc = sqlite3_exec(db, cmd.c_str(), NULL, NULL, NULL);
    
    if (rc) {
      STAGE_WARN(sqlite3_errmsg(db));
      return;
    }

    // Prepare the insert statement
    cmd = "INSERT INTO " + tableName + " VALUES" + values;
    rc = sqlite3_prepare_v2(db, cmd.c_str(), -1, &stmt, NULL);
    if (rc) {
      STAGE_WARN(sqlite3_errmsg(db));
      return;
    }
  }
      
}

void SQLiteSaveStage::saveRow(TuplePtr tuple) {
  if (db && stmt) { // initialization succeeded
    if (tuple->size() > num_columns + 2) {
      STAGE_WARN("SQLiteSaveStage: ignoring redundant tuple entries.");
    }
    sqlite3_reset(stmt);
    sqlite3_clear_bindings(stmt);
    
    // bind the values
    for(size_t i = 2; i < tuple->size(); i++) {
      ValuePtr value = tuple->at(i);
      switch(value->typeCode()) {
      case Value::NULLV: 
        break;
      case Value::STR: {
        std::string str = Val_Str::cast(value);
        sqlite3_bind_text(stmt, i-1, str.c_str(), -1, SQLITE_TRANSIENT);
        break;
      }
      case Value::INT64:
        sqlite3_bind_int64(stmt, i-1, Val_Int64::cast(value));
        break;
      case Value::DOUBLE:
        sqlite3_bind_double(stmt, i-1, Val_Double::cast(value));
        break;
      case Value::TIME: {
        std::string time_str = value->toString();
        sqlite3_bind_text(stmt, i-1, time_str.c_str(), -1, SQLITE_TRANSIENT);
        break;
      }
      case Value::TIME_DURATION: // duration in seconds
        sqlite3_bind_double(stmt, i-1, Val_Double::cast(value));
        break;
      case Value::FACTOR: { // eventually could compress
        std::ostringstream out;
        {
          prl::xml_oarchive archive(out);
          archive << Val_Factor::cast(value);
        }
        sqlite3_bind_text(stmt, i-1, out.str().c_str(), -1, SQLITE_TRANSIENT);
        break;
      }
      default:
        STAGE_WARN("SQLiteSaveStage: Unsupported value type " 
                   << value->typeName());
        
      }
    }

    // execute the insert command
    sqlite3_step(stmt);
  }
}

void SQLiteSaveStage::newInput(TuplePtr tuple)
{
  if (!initialized) { // The first tuple specifies the table
    initialized = true;
    createDatabase(tuple);
  } else {
    saveRow(tuple);
  }
}

// ValuePtr SQLiteSaveStage::SQLiteSave = Val_Str::mk(std::string("SQLiteSave"));

// This is necessary for the class to register itself with the
// factory.
DEFINE_STAGE_INITS(SQLiteSaveStage,"SQLiteSave")
