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
 * DESCRIPTION: Simple MAX aggregate object. It decides max according to
 * the value pointer compareTo method.
 *
 */

#ifndef __AGGMAX_H__
#define __AGGMAX_H__

#include "commonTable.h"
#include "value.h"
#include "aggFactory.h"

class AggMax :
  public CommonTable::AggFunc
{
public:
  AggMax();


  virtual ~AggMax();

  
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
  DECLARE_PUBLIC_INITS(AggMax)


private:
  /** The current max value for this aggregate */
  ValuePtr _currentMax;


  // This is necessary for the class to register itself with the
  // factory.
  DECLARE_PRIVATE_INITS

};


#endif // AGGMAX_H
