/*
 * Copyright (c) 2005 Intel Corporation. All rights reserved.
 *
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 */

#ifndef __AGGMKLIST_H__
#define __AGGMKLIST_H__

#include "commonTable.h"
#include "value.h"
#include "list.h"
#include "aggFactory.h"

class AggMkList :
  public CommonTable::AggFunc
{
public:
  AggMkList();
  
  
  virtual ~AggMkList();

  
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
  DECLARE_PUBLIC_INITS(AggMkList)


private:
  /** The current mkList value for this aggregate */
  ListPtr _list;


  // This is necessary for the class to register itself with the
  // factory.
  DECLARE_PRIVATE_INITS


};


#endif // AGGMKLIST_H
