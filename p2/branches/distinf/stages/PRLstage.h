// -*- c-basic-offset: 2; related-file-name: "CSVstage.C" -*-
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

#ifndef _PRLSTAGE_H_
#define _PRLSTAGE_H_

#include "stage.h"
#include "val_str.h"
#include "stageRegistry.h"

#include <boost/scoped_ptr.hpp>

#include <prl/xml_iarchive.hpp>

class PRLstage : public Stage::Processor {
public:
  PRLstage(Stage* myStage);

  ~PRLstage() {};

  /** The input schema for PRLstage is <tupleName, location specifier,
      filename>. The output is <tupleName, location specifier, object>.*/
  void newInput(TuplePtr inputTuple);

  std::pair<TuplePtr, Stage::Status> newOutput();

  // This is necessary for the class to register itself with the
  // factory.
  DECLARE_PUBLIC_STAGE_INITS(PRLstage)

private:
  // The current string accumulator
  boost::scoped_ptr<prl::xml_iarchive> in_ptr;

  /** The name of output tuples */
  static ValuePtr PRLTuple;

  /** My current location specifier */
  ValuePtr _locationSpecifier;

  prl::universe u;

  // This is necessary for the class to register itself with the
  // factory.
  DECLARE_PRIVATE_STAGE_INITS
};

#endif
