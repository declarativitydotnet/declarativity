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

#include "PRLstage.h"

#include "val_factor.h"
#include <prl/factor/xml/polymorphic_factor.hpp>
#include <prl/factor/xml/table_factor.hpp>
#include <prl/factor/xml/constant_factor.hpp>

// Constructor
PRLstage::PRLstage(Stage* myStage) : Stage::Processor(myStage) { 
  prl::xml_iarchive::register_type< prl::constant_factor<> >("constant_factor");
  prl::xml_iarchive::register_type< prl::table_factor<> >("table_factor");
}

std::pair< TuplePtr, Stage::Status >
PRLstage::newOutput()
{
  TuplePtr p;

  if (in_ptr && in_ptr->has_next()) {
    polymorphic_factor f;
    (*in_ptr) >> f;

    // Prepare the result
    TuplePtr resultTuple = Tuple::mk();
    resultTuple->append(PRLTuple);
    resultTuple->append(_locationSpecifier);
    resultTuple->append(Val_Factor::mk(f));
    resultTuple->freeze();
    return std::make_pair(resultTuple, Stage::MORE);
  } else {
    return std::make_pair(TuplePtr(), Stage::DONE);
  }

}

/** The input schema for PRLstage is <tupleName, location specifier, filename>. 
    The output is <tupleName, location specifier, fields...>.  Note
    that there's no schema check here ... (?)
  */
void
PRLstage::newInput(TuplePtr inputTuple)
{
  // Fetch the input tuple
  if (inputTuple->size() < 3) {
    // Insufficient number of fields
    STAGE_WARN("newInput: tuple " 
            << inputTuple
            << " doesn't appear to have enough fields to be tokenized");
  } else {
    // Remember the location specifier
    _locationSpecifier = (*inputTuple)[1];

    // Fetch the 3rd value
    ValuePtr fileName = (*inputTuple)[2];

    // Is the filename a string?
    if ((fileName->typeCode() != Value::STR)) {
      // Inappropriate input values
      STAGE_WARN("newInput: tuple "
                << inputTuple
                << " doesn't have a string for a filename in position 2.");
    } else {
      string fileNameStr = Val_Str::cast(fileName);
      in_ptr.reset(new prl::xml_iarchive(fileNameStr.c_str(), u));
      return; // successful initialization

      /*
      // TODO: check if open
      // attempt to open file
      in.open(fileNameStr.c_str(), std::ios::in);
      if (in.is_open()) {
        // Inappropriate input values
        STAGE_WARN("newInput: failed to open file "
                   << fileNameStr
                   << ".");
      } else {
      */

    }
  }
}

ValuePtr PRLstage::PRLTuple = Val_Str::mk(std::string("PRLtuple"));

// This is necessary for the class to register itself with the
// factory.
DEFINE_STAGE_INITS(PRLstage,"PRLtuple")

namespace prl {
  map<std::string, xml_iarchive::deserializer*> xml_iarchive::deserializers;
}
