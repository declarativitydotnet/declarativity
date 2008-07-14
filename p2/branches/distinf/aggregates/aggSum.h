/*
 * @(#)$Id: aggSum.h 1243 2007-07-16 19:05:00Z maniatis $
 *
 * Copyright (c) 2005 Intel Corporation. All rights reserved.
 *
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 * DESCRIPTION: Simple COUNT aggregate object. It counts matching
 * tuples.
 *
 */

#ifndef __AGGSUM_H__
#define __AGGSUM_H__

#include "commonTable.h"
#include "value.h"
#include "aggFactory.h"
#include "val_double.h"

class AggSum :
  public CommonTable::AggFunc
{
public:
  AggSum();
  
  
  virtual ~AggSum();

  
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
  DECLARE_PUBLIC_INITS(AggSum)


private:
  /** The current count value for this aggregate */
//  Val_Double _currentSum;
  double _currentSum;



  // This is necessary for the class to register itself with the
  // factory.
  DECLARE_PRIVATE_INITS


};


#endif // AGGSUM_H
