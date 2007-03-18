// -*- c-basic-offset: 2; related-file-name: "CSVstage.h" -*-
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
 * DESCRIPTION: A csv reader stage.
 *
 */

#include "CSVstage.h"

// Constructor
CSVstage::CSVstage(Stage* myStage)
  : Stage::Processor(myStage),
    _acc("")
{
}


std::pair< TuplePtr, Stage::Status >
CSVstage::newOutput()
{
  TuplePtr p;
  char block[_blockSize];
  string blkstring;
	CSVlex lexer;

	// If we have nothing enqueued from before, read more text from the input file
	// and enqueue resulting tuples
	while (_q.empty() && !_fileStream.eof()) {
		TuplePtr t;
		int parsing;
		
	  _fileStream.read(block, _blockSize);
		// convert C string to std::string
	  blkstring = block;
	  _acc += blkstring;
		// note: the last tuple we construct in this loop is unused.  We could explicitly delete it, but 
		// we'll count on boost::shared_ptr to take care of that.
	  
	  for (t = Tuple::mk(), parsing = 1; parsing; t = Tuple::mk()) {
		parsing = lexer.try_to_parse_line(_acc,t);
		if (parsing == CSVlex::CSVGotComment)
		  continue;
		if (parsing == CSVlex::CSVGotLine)
			_q.push(t);
	  }
			
	} 
	 
	// ASSERT: Now either the _q has some tuples, or we're at EOF (fileStream.eof()), or both
  if (_q.empty()) {
		// then we're at EOF
		if (_acc != "")	
			// we had leftover stuff in the buffer
	  	STAGE_WARN("newOutput: last line of CSV not terminate by newline, didn't get consumed: _acc = \"" << _acc <<"\"");
		_fileStream.close(); // clean up this file handle
		return std::make_pair(TuplePtr(), Stage::DONE);
  } 
	else {
		// there's a good tuple in the queue
   	p = _q.front();
   	_q.pop();
  }

  // Prepare result
  TuplePtr resultTuple =
    Tuple::mk();
  resultTuple->append(CSVTuple);
  resultTuple->append(_locationSpecifier);
  // iterate through tuple fields and append
  for (unsigned int i = 0; i < p->size(); i++) {
  	resultTuple->append((*p)[i]);
  }
  resultTuple->freeze();

  return std::make_pair(resultTuple, Stage::MORE);
}

  /** The input schema for CSVstage is <tupleName, location specifier, filename>. 
      The output is <tupleName, location specifier, fields...>.  Note
      that there's no schema check here ...if the CSV is irregular, we
      simply produce an irregular stream of tuples 
  */
void
CSVstage::newInput(TuplePtr inputTuple)
{
	// Fetch the input tuple
  if (inputTuple->size() < 3) {
    // Insufficient number of fields
    STAGE_WARN("newInput: tuple "
              << inputTuple
              << " doesn't appear to have enough "
              << "fields to be tokenized");
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
                << " doesn't appear to have a string "
                << "for a filename in position 2.");
    } else {
			string fileNameStr = Val_Str::cast(fileName);

			// attempt to open file
		  _fileStream.open(fileNameStr.c_str(), std::ios::in);
		  if (!_fileStream.is_open())
		  {
		    // Inappropriate input values
	      STAGE_WARN("newInput: failed to open file "
	                << fileName
	                << ".");
		  } else
			{
				// SUCCESSFUL INITIALIZATION!
      	return;
			}
    }
  }

  // We're here because the CSV stage cannot produce anything. The next
  // newOutput will just conclude.
  _fileStream.close();
}

ValuePtr CSVstage::CSVTuple = Val_Str::mk(std::string("CSVtuple"));



// This is necessary for the class to register itself with the
// factory.
DEFINE_STAGE_INITS(CSVstage,"CSVtuple")
