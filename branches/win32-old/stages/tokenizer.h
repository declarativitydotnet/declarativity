// -*- c-basic-offset: 2; related-file-name: "tokenizer.C" -*-
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
 * DESCRIPTION: A tokenizing stage.
 *
 */

#ifndef _TOKENIZER_H_
#define _TOKENIZER_H_

#include "stage.h"
#include <string>
#include "val_str.h"
#include "stageRegistry.h"

class Tokenizer : public Stage::Processor {
public:
  Tokenizer(Stage* myStage);

  virtual ~Tokenizer();

  
  /** The input schema for Tokenizer is <tupleName, location specifier,
      deliminterString, contentString>. The output is <tupleName,
      location specifier, token>.*/
  void
  newInput(TuplePtr inputTuple);


  std::pair< TuplePtr, Stage::Status >
  newOutput();


  // This is necessary for the class to register itself with the
  // factory.
  DECLARE_PUBLIC_STAGE_INITS(Tokenizer)




private:

  /** Delimiter string */
  std::string _delimiter;


  /** Content string */
  std::string _content;


  /** My next string position to tokenize */
  std::string::size_type _iterator;


  /** The name of output tuples */
  static ValuePtr TOKEN;


  /** My current location specifier */
  ValuePtr _locationSpecifier;


  // This is necessary for the class to register itself with the
  // factory.
  DECLARE_PRIVATE_STAGE_INITS
};

#endif
