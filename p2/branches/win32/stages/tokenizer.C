// -*- c-basic-offset: 2; related-file-name: "tokenizer.h" -*-
/*
 * @(#)$Id$
 *
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 * DESCRIPTION: A tokenizerping stage.
 *
 */

#ifdef WIN32
#include "p2_win32.h"
#endif // WIN32
#include "tokenizer.h"
#include "val_str.h"


Tokenizer::Tokenizer(Stage* myStage)
  : Stage::Processor(myStage)
{
}


Tokenizer::~Tokenizer()
{
}


std::pair< TuplePtr, Stage::Status >
Tokenizer::newOutput()
{
  // Iterator has the first non-delimiter. Find the first delimiter.
  std::string::size_type endOfToken =
    _content.find_first_of(_delimiter, _iterator);

  ValuePtr token;
  if (endOfToken == _content.npos) {
    // We reached the end of the string. Do we have a non-empty content?
    if (_iterator != endOfToken) {
      // We have content. From here to the end
      token = Val_Str::mk(_content.substr(_iterator));
      _iterator = _content.npos;
    } else {
      // We have no content, zilch.  This is the end of the tokens in
      // this content string.
      return std::make_pair(TuplePtr(), Stage::DONE);
    }
  } else {
    // We did find something and haven't reached the end yet
    token =
      Val_Str::mk(_content.substr(_iterator, endOfToken - _iterator));

    // Skip any delimiters
    _iterator = _content.find_first_not_of(_delimiter, endOfToken);
  }

  // We're here because we have a result to give
  TuplePtr resultTuple =
    Tuple::mk();
  resultTuple->append(TOKEN);
  resultTuple->append(_locationSpecifier);
  resultTuple->append(token);
  resultTuple->freeze();
  
  return std::make_pair(resultTuple, Stage::MORE);
}


void
Tokenizer::newInput(TuplePtr inputTuple)
{
  // Fetch the two strings
  if (inputTuple->size() < 4) {
    // Insufficient number of fields
    STAGE_WARN("newInput: tuple "
              << inputTuple
              << " doesn't appear to have enough "
              << "fields to be tokenized");
  } else {
    // Remember the location specifier
    _locationSpecifier = (*inputTuple)[1];

    // Fetch the 3rd and 4th values
    ValuePtr delimiterVal = (*inputTuple)[2];
    ValuePtr contentVal = (*inputTuple)[3];

    // Are they both strings?
    if ((delimiterVal->typeCode() != Value::STR) ||
        (contentVal->typeCode() != Value::STR)) {
      // Inappropriate input values
      STAGE_WARN("newInput: tuple "
                << inputTuple
                << " doesn't appear to have strings "
                << "as its 2nd and 3rd values.");
    } else {
      // Turn them into strings
      _delimiter = Val_Str::cast(delimiterVal);
      _content = Val_Str::cast(contentVal);

      // Place the iterator right after the front delimiters
      _iterator = _content.find_first_not_of(_delimiter, 0);

      return;
    }
  }

  // We're here because the tokenizer cannot produce anything. The next
  // newOutput will just conclude.
  _delimiter = "";
  _content = "";
  _iterator = 0;
}


ValuePtr Tokenizer::TOKEN = Val_Str::mk(std::string("token"));



// This is necessary for the class to register itself with the
// stage registry.
DEFINE_STAGE_INITS(Tokenizer,"TOKENIZER")


