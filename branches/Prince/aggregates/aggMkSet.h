/*
 * @(#)$Id: aggMkSet.h 1243 2007-07-16 19:05:00Z maniatis $
 *
 * Copyright (c) 2005 Intel Corporation. All rights reserved.
 *
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 * DESCRIPTION: Simple MKSET aggregate object. It mkSets matching
 * tuples.
 *
 */

#ifndef __AGGMKSET_H__
#define __AGGMKSET_H__

#include "commonTable.h"
#include "value.h"
#include "set.h"
#include "aggFactory.h"

class AggMkSet :
  public CommonTable::AggFunc
{
public:
  AggMkSet();
  
  
  virtual ~AggMkSet();

  
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
  DECLARE_PUBLIC_INITS(AggMkSet)


private:
  /** The current mkSet value for this aggregate */
  SetPtr _set;


  // This is necessary for the class to register itself with the
  // factory.
  DECLARE_PRIVATE_INITS


};


#endif // AGGMKSET_H
