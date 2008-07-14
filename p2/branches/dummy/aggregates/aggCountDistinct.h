/*
 * @(#)$Id$
 *
 * Copyright (c) 2005 Intel Corporation. All rights reserved.
 *
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 * DESCRIPTION: Simple COUNTDISTINCT aggregate object. It counts
 * matching tuples with distinct values.
 *
 */

#ifndef __AGGCOUNTDISTINCT_H__
#define __AGGCOUNTDISTINCT_H__

#include "commonTable.h"
#include "value.h"
#include "aggFactory.h"

class AggCountDistinct :
  public CommonTable::AggFunc
{
public:
  AggCountDistinct();
  
  
  virtual ~AggCountDistinct();

  
  void
  reset();
  
  
  void
  first(ValuePtr);
  

  void
  process(ValuePtr);
  
  
  ValuePtr
  result();


  // This is necessary for the class to register itself with the
  // factory.
  DECLARE_PUBLIC_INITS(AggCountDistinct)


private:
  /** The current set of values for this aggregate */
  ValueSet _valueSet;


  // This is necessary for the class to register itself with the
  // factory.
  DECLARE_PRIVATE_INITS
};


#endif // AGGCOUNTDISTINCT_H
