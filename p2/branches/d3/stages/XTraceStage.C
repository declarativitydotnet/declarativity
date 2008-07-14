// -*- c-basic-offset: 2; related-file-name: "XTraceStage.h" -*-
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

#include "XTraceStage.h"



// Constructor
XTraceStage::XTraceStage(Stage* myStage)
  : Stage::Processor(myStage),
    _acc(""), _bGotIds(false)
{
STAGE_WARN("XTraceStage constructor\n");

}


std::pair< TuplePtr, Stage::Status >
XTraceStage::newOutput()
{
  TuplePtr p;
  char block[_blockSize];
  string blkstring;
	XTraceLex lexer;

	// If we have nothing enqueued from before, read more text from the input file
	// and enqueue resulting tuples
	while (_q.empty() && !_fileStream.eof()) {
		TuplePtr t;
		int parsing;
	
	memset(block, 0, _blockSize);	
	  _fileStream.read(block, _blockSize);
		// convert C string to std::string
	  blkstring = block;
	  _acc += blkstring;
	STAGE_WARN("ACC added <" << _acc << ">\n");
		// note: the last tuple we construct in this loop is unused.  We could explicitly delete it, but 
		// we'll count on boost::shared_ptr to take care of that.
	  
	  for (parsing = 1; parsing; ) {
		string sKey, sValue, sAux;
		parsing = lexer.try_to_parse_line(_acc,sKey, sValue);
		if (parsing == XTraceLex::XTraceGotBlankLine)
		{
			if (_bGotIds)
			{
				t = Tuple::mk();

				t->append(Val_Str::mk(_TaskId));
				t->append(Val_Str::mk(_OpId));
				t->append(Val_Str::mk(_ChainId));
				t->append(Val_Str::mk("task"));
				t->append(Val_Str::mk(_Host));
				t->append(Val_Str::mk(_Agent));
				t->append(Val_Str::mk(_Label));
				//t->append(Val_Str::mk(_TS));
				t->append(Val_Int64::mk(strtoll(_TS.c_str(), NULL, 0)));
				t->append(Val_Str::mk(_NextHost));
				_q.push(t);
				_Host = "";
				_Agent = "";
				_Label = "";
				_TS = "";
				_NextHost = "";
			}

			while (!_edgeQ.empty())
			{
				TuplePtr t1;
				TuplePtr t2 = Tuple::mk();

				t1 = _edgeQ.front();
				_edgeQ.pop();
		
				if (_bGotIds)
				{
					t2->append(Val_Str::mk(_TaskId));
					t2->append(Val_Str::mk(_OpId));
					t2->append(Val_Str::mk(_ChainId));
					t2->append(Val_Str::mk("edge"));
					for (unsigned int i = 0; i<t1->size(); i++) {
						t2->append((*t1)[i]);
					}	
					_q.push(t2);
				}
			}
			_bGotIds = false;
		  	
			continue;
		}
		if (parsing == XTraceLex::XTraceGotItem)
		{
			if (sKey=="X-Trace")
			{
				string sFlag;
				int nTaskIDLen;
				unsigned int uFlag;

				sFlag = sValue.substr(0, 2);
				uFlag = strtoul(sFlag.c_str(), NULL, 16);
				nTaskIDLen = 8 * ((uFlag & 0x03)+1);
				if (nTaskIDLen == 32) nTaskIDLen = 40;
		
	
				//nTaskIDLen = 8;
	
				_TaskId = sValue.substr(2, nTaskIDLen);
				_OpId = sValue.substr(2+nTaskIDLen, 8);

				_bGotIds = true;
			}
			else if (sKey=="Edge")
			{
				sAux = sValue.substr(10);
				sValue = sValue.substr(0, 8);
				t = Tuple::mk();
				t->append(Val_Str::mk(sValue));
				t->append(Val_Str::mk(sAux));
				_edgeQ.push(t);
			}
			else if (sKey=="Host") _Host = sValue;
			else if (sKey=="Agent") _Agent = sValue;
			else if (sKey=="Label") _Label = sValue;
			else if (sKey=="Timestamp") 
			{
				sValue = sValue.substr(0, 10)+sValue.substr(11, 3);
				_TS = sValue;
			}				
			else if (sKey=="NextHost") _NextHost = sValue;
		}
	  }
			
	} 
	 
	// ASSERT: Now either the _q has some tuples, or we're at EOF (fileStream.eof()), or both
  if (_q.empty()) {
		// then we're at EOF
		if (_acc != "")	
			// we had leftover stuff in the buffer
	  	STAGE_WARN("newOutput: last line of XTrace not terminate by newline, didn't get consumed: _acc = \"" << _acc <<"\"");
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
  resultTuple->append(XTraceTuple);
  resultTuple->append(_locationSpecifier);

  // iterate through tuple fields and append
  for (unsigned int i = 0; i < p->size(); i++) {
  	resultTuple->append((*p)[i]);
  }
  resultTuple->freeze();

  return std::make_pair(resultTuple, Stage::MORE);
}

  /** The input schema for XTraceStage is <tupleName, location specifier, filename>. 
      The output is <tupleName, location specifier, fields...>.  Note
      that there's no schema check here ...if the XTrace is irregular, we
      simply produce an irregular stream of tuples 
  */
void
XTraceStage::newInput(TuplePtr inputTuple)
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

  // We're here because the XTrace stage cannot produce anything. The next
  // newOutput will just conclude.
  _fileStream.close();
}

ValuePtr XTraceStage::XTraceTuple = Val_Str::mk(std::string("XTraceTuple"));



// This is necessary for the class to register itself with the
// factory.
DEFINE_STAGE_INITS(XTraceStage,"XTraceTuple")
