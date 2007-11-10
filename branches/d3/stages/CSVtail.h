// -*- c-basic-offset: 2; related-file-name: "CSVtail.C" -*-
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
 * DESCRIPTION: A CSV tokenizing stage.
 *
 */

#ifndef _CSVTAIL_H_
#define _CSVTAIL_H_

#include <string>
#include <queue>
#include <iostream>
#include <fstream>
#include <boost/regex.hpp>
#include <sys/stat.h>
#include <fcntl.h>
#include "stage.h"
#include "val_str.h"
#include "stageRegistry.h"
#include "CSVlex.h"

class CSVtail : public Stage::Processor {
public:
  CSVtail(Stage* myStage);

  ~CSVtail() {};

  /** The input schema for CSVtail is <tupleName, location specifier,
      delimiterString, filename>. The output is <tupleName,
      location specifier, fields...>.*/
  void
  newInput(TuplePtr inputTuple);


  std::pair< TuplePtr, Stage::Status >
  newOutput();


  // This is necessary for the class to register itself with the
  // factory.
  DECLARE_PUBLIC_STAGE_INITS(CSVtail)




private:
	// a CVS file blocksize to go after
	static const int _blockSize = 8096;
	
	// a queue of tuples to process
  std::queue<TuplePtr> _q; 

  // The current string accumulator
  string	_acc;


  /** file name string */

//std::ifstream _fileStream;
    string _fileName;
    int _fd;		/* opened fd, or <= 0 if not opened		*/
    long _size;		/* size of entry last time checked		*/
    long _mtime;	/* modification time last time checked		*/

  /** The name of output tuples */
  static ValuePtr CSVTuple;


  /** My current location specifier */
  ValuePtr _locationSpecifier;


  // This is necessary for the class to register itself with the
  // factory.
  DECLARE_PRIVATE_STAGE_INITS
};

#endif
