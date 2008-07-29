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

#include "SQLiteLoadStage.h"
#include <sqlite3.h>

#include "val_null.h"
#include "val_str.h"
#include "val_int64.h"
#include "val_double.h"
#include "val_factor.h"
#include "val_set.h"

#include <prl/xml_oarchive.hpp>
#include <prl/factor/xml/polymorphic_factor.hpp>
#include <prl/factor/xml/constant_factor.hpp>
#include <prl/factor/xml/table_factor.hpp>
#include <prl/factor/xml/gaussian_factors.hpp>
#include <prl/factor/xml/mixture.hpp>
#include <prl/factor/xml/decomposable_fragment.hpp>
#include <prl/math/bindings/lapack.hpp>

//! The matrix representation
typedef prl::math::bindings::lapack::double_matrix matrix_type;

//! The vector representation
typedef prl::math::bindings::lapack::double_vector vector_type;

//! A constant factor
typedef prl::constant_factor<> constant_factor;

//! A table factor
typedef prl::table_factor<> table_factor;

//! The type of factors stored in this value
typedef prl::canonical_gaussian<matrix_type, vector_type> canonical_gaussian;

//! The moment Gaussian factor type (used for mean, covariance)
typedef prl::moment_gaussian<matrix_type, vector_type> moment_gaussian;

//! A decomposable fragment over Gaussians
typedef prl::decomposable_fragment<canonical_gaussian> fragment_gaussian;

namespace prl {
  map<std::string, xml_iarchive::deserializer*> xml_iarchive::deserializers;
}

struct archive_registration {

  archive_registration() {
    using namespace std;
    using prl::xml_iarchive;
    cerr << "Registering the types with archive" << endl;
    xml_iarchive::register_type<prl::domain>("domain");
    xml_iarchive::register_type<constant_factor>("constant_factor");
    xml_iarchive::register_type<table_factor>("table_factor");
    xml_iarchive::register_type<canonical_gaussian>("canonical_gaussian");
    //xml_iarchive::register_type<mixture_gaussian>("mixture_gaussian");
    xml_iarchive::register_type<fragment_gaussian>("decomposable_fragment");
  }

} register_types;



SQLiteLoadStage::SQLiteLoadStage(Stage* myStage) 
  : Stage::Processor(myStage), db(NULL), stmt(NULL) {
  using prl::xml_iarchive;
}

SQLiteLoadStage::~SQLiteLoadStage() {
  if(stmt) sqlite3_finalize(stmt);
  if(db) sqlite3_close(db);
}

void SQLiteLoadStage::newInput(TuplePtr tuple)
{
  assert(db == NULL);

  // Check if sufficient number of fields
  if (tuple->size() < 4) {
    STAGE_WARN("newInput: tuple " 
               << tuple
               << " doesn't appear to have enough fields");
    return;
  }

  // Remember the location specifier
  _locationSpecifier = (*tuple)[1];
    
  // Extract the filename and the table name
  std::string fileName  = Val_Str::cast((*tuple)[2]);
  std::string tableName = Val_Str::cast((*tuple)[3]);

  // Form the list of columns 
  std::string columns;
  if (tuple->size() > 4) {
    size_t num_columns = tuple->size() - 4;
    for (size_t i = 0; i < num_columns; ++i) {
      if (i) columns += ", ";
      columns += Val_Str::cast((*tuple)[i+4]);
    }
  } else {
    columns = "*";
  }

  // Open the database
  int rc = sqlite3_open(fileName.c_str(), &db);
  if (rc) {
    STAGE_WARN("newInput: failed to open file "
               << fileName
               << ".");
    return;
  }

  // Execute the query
  std::string cmd = "SELECT " + columns + " FROM " + tableName;
  rc = sqlite3_prepare_v2(db, cmd.c_str(), -1, &stmt, NULL);
  if (rc) {
    STAGE_WARN(sqlite3_errmsg(db));
    return;
  }
}


std::pair< TuplePtr, Stage::Status >
SQLiteLoadStage::newOutput() {    
  if (db && stmt) { // initialization succeeded
    int rc = sqlite3_step(stmt);
    switch (rc) {
    case SQLITE_ROW:
      return std::make_pair(loadRow(), Stage::MORE);
    case SQLITE_DONE:
      return std::make_pair(TuplePtr(), Stage::DONE);
    default:
      STAGE_WARN(sqlite3_errmsg(db));
      return std::make_pair(TuplePtr(), Stage::DONE);
    }
  } else {
    return std::make_pair(TuplePtr(), Stage::DONE);
  }
}

TuplePtr SQLiteLoadStage::loadRow() {
  size_t num_columns = sqlite3_column_count(stmt);

  // Initialize the tuple
  TuplePtr result = Tuple::mk();
  result->append(Val_Str::mk("SQLiteLoad"));
  result->append(_locationSpecifier);
  
  // Iterate through tuple fields and append the column values
  for (size_t i = 0; i < num_columns; i++) {
    int type = sqlite3_column_type(stmt, i);
    switch(type) {
    case SQLITE_INTEGER: 
      result->append(Val_Int64::mk(sqlite3_column_int64(stmt, i)));
      break;
    case SQLITE_FLOAT:
      result->append(Val_Double::mk(sqlite3_column_double(stmt, i)));
      break;
    case SQLITE_TEXT:
      result->append(Val_Str::mk((const char*)sqlite3_column_text(stmt, i)));
      break;
    case SQLITE_BLOB: {
      using namespace prl;
      const unsigned char* blob = sqlite3_column_text(stmt, i);
      xml_iarchive in(blob, sqlite3_column_bytes(stmt, i), Val_Factor::u);
      serializable_object* obj = in.read();
      if (typeid(*obj) == typeid(domain)) {
        SetPtr s(Val_Set::cast(Val_Factor::names(*dynamic_cast<domain*>(obj))));
        result->append(Val_Set::mk(s));
      } else {
        prl::factor* f = dynamic_cast<prl::factor*>(obj);
        assert(f != NULL);
        result->append(Val_Factor::mk(*f));
      }
      delete obj;
      break;
    }
    case SQLITE_NULL:
      result->append(Val_Null::mk());
      break;
    default:
      STAGE_WARN("SQLiteLoadStage: unrecognized column type " << type);
      result->append(Val_Null::mk());
      break;
    }
  }
  result->freeze();
  return result;
}


// This is necessary for the class to register itself with the
// factory.
DEFINE_STAGE_INITS(SQLiteLoadStage,"SQLiteLoad")

