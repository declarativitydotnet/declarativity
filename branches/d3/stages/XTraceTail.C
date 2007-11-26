// -*- c-basic-offset: 2; related-file-name: "XTraceTail.h" -*-
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
 * DESCRIPTION: A csv tail reader stage.
 *
 */

#include "XTraceTail.h"
#include "val_int32.h"
#include "loop.h"

// Constructor
XTraceTail::XTraceTail(Stage* myStage)
  : Stage::Processor(myStage),
    _acc("")
{
STAGE_WARN("XTraceTail constructor\n");

_fd = 0;
_size = 0;
_mtime = 0;

}


std::pair< TuplePtr, Stage::Status >
XTraceTail::newOutput()
{
//  STAGE_ERROR("call newOutput()!!!!");
  TuplePtr p;
  char block[_blockSize];
  string blkstring;
	XTraceLex lexer;

	// If we have nothing enqueued from before, read more text from the input file
	// and enqueue resulting tuples
	while (_q.empty() && _fd) {


		struct stat sbuf;
	    	int status = fstat(_fd,&sbuf);
/*
		STAGE_ERROR("while loop!!!! status [" << status
					<< "] sbuf.st_size[" << sbuf.st_size
					<< "] _size [" << _size
					<< "] sbuf.st_mtime [" << sbuf.st_mtime 
					<< "] _mtime [" << _mtime << "]");
*/

		if (sbuf.st_size == _size && sbuf.st_mtime == _mtime) break;

		_mtime = sbuf.st_mtime;

//		STAGE_ERROR("read more!!!!");

		TuplePtr t;
		int parsing;

		int nb;
		memset(block, 0, _blockSize);		
		nb = read(_fd, block, _blockSize);
		if (nb==0) break;
		
//		STAGE_ERROR("loaded  "<< nb << " now size is " << _size);

		_size += nb;
		
		// convert C string to std::string
	  blkstring = block;
	  _acc += blkstring;
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
				t->append(Val_Str::mk(_TS));
				t->append(Val_Str::mk(_Next_host));
				_q.push(t);
				_Host = "";
				_Agent = "";
				_Label = "";
				_TS = "";
				_Next_host = "";
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
			else if (sKey=="Timestamp") _TS = sValue;
			else if (sKey=="NextHost") _Next_host = sValue;
		}
	  }
			
	} 
	 
	// ASSERT: Now either the _q has some tuples, or we're at EOF (fileStream.eof()), or both
  if (_q.empty()) {
		// then we're at EOF
		if (_acc != "")	
			// we had leftover stuff in the buffer
	  	STAGE_WARN("newOutput: last line of CSV not terminate by newline, didn't get consumed: _acc = \"" << _acc <<"\"");


//		return std::make_pair(TuplePtr(), Stage::DONE);

		// register delayed callback to resume
		delayCB(2,boost::bind(&Stage::resume, _stage), _stage);
		return std::make_pair(TuplePtr(), Stage::MORE);

	}
	else {
		// there's a good tuple in the queue
   	p = _q.front();
   	_q.pop();
  }

  // Prepare result
  TuplePtr resultTuple =
    Tuple::mk();
  resultTuple->append(XTraceTailVal);
  resultTuple->append(_locationSpecifier);
  // iterate through tuple fields and append
  for (unsigned int i = 0; i < p->size(); i++) {
  	resultTuple->append((*p)[i]);
  }
  resultTuple->freeze();

  return std::make_pair(resultTuple, Stage::MORE);
}

  /** The input schema for XTraceTail is <tupleName, location specifier, filename>. 
      The output is <tupleName, location specifier, fields...>.  Note
      that there's no schema check here ...if the CSV is irregular, we
      simply produce an irregular stream of tuples 
  */
void
XTraceTail::newInput(TuplePtr inputTuple)
{
//  STAGE_ERROR("call newInput()!!!!");


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
		if (_fd<=0 || fileNameStr!=_fileName)
		{
			_fileName = fileNameStr;
			_fd = open(_fileName.c_str(), O_RDONLY);
			if (_fd>0)
			{
//				STAGE_ERROR("open!!!!!!!!!!!!!!" << _nPassed);
				// SUCCESSFUL INITIALIZATION!
			}
			else
			{
				STAGE_ERROR("Failed to open file " << _fileName <<"\n" );

			    // Inappropriate input values
			      STAGE_WARN("newInput: failed to open file "
			                << fileName
			                << ".");
			} 
		}


    }
  }
}

/*
			struct stat sbuf;
		    	int status = fstat(_fd,&sbuf);
			if (sbuf.st_size == _size && sbuf.st_mtime == _mtime)
*/

ValuePtr XTraceTail::XTraceTailVal = Val_Str::mk(std::string("XTraceTail"));



// This is necessary for the class to register itself with the
// factory.
DEFINE_STAGE_INITS(XTraceTail,"XTraceTail")
