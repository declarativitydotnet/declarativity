// -*- c-basic-offset: 2; related-file-name: "XTraceStage.C" -*-
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
 * DESCRIPTION: A XTrace data tokenizing stage.
 *
 */

#ifndef _XTRACESTAGE_H_
#define _XTRACESTAGE_H_

#include <string>
#include <queue>
#include <iostream>
#include <fstream>
#include <boost/regex.hpp>
#include "stage.h"
#include "val_str.h"
#include "val_double.h"
#include "val_int64.h"
#include "stageRegistry.h"
#include "XTraceLex.h"

class XTraceStage : public Stage::Processor {
public:
  XTraceStage(Stage* myStage);

  ~XTraceStage() {};

  /** The input schema for CSVstage is <tupleName, location specifier,
      delimiterString, filename>. The output is <tupleName,
      location specifier, fields...>.*/
  void
  newInput(TuplePtr inputTuple);


  std::pair< TuplePtr, Stage::Status >
  newOutput();


  // This is necessary for the class to register itself with the
  // factory.
  DECLARE_PUBLIC_STAGE_INITS(XTraceStage)




private:
	// a file blocksize to go after
	static const int _blockSize = 8096;

 // (TaskId, OpId, ChainId, Key, Value)
  std::queue<TuplePtr> _q; // queue for emitting output
  //std::queue<TuplePtr> _tempQ; // temporary queue to store values which are not ready to be emitted 
  std::queue<TuplePtr> _edgeQ;

  string 	_TaskId, _OpId, _ChainId;
  string	_Host, _Agent, _Label, _TS, _NextHost;

  bool	_bGotIds;

	
  // The current string accumulator
  string	_acc;

  /** file name string */
  std::ifstream _fileStream;


  /** The name of output tuples */
  static ValuePtr XTraceTuple;


  /** My current location specifier */
  ValuePtr _locationSpecifier;


  // This is necessary for the class to register itself with the
  // factory.
  DECLARE_PRIVATE_STAGE_INITS
};

#endif
