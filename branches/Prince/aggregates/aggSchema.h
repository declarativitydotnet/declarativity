/*
 * Copyright (c) 2005 Intel Corporation. All rights reserved.
 *
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 */

#ifndef __AGGSCHEMA_H__
#define __AGGSCHEMA_H__

#include "commonTable.h"
#include "value.h"
#include "list.h"
#include "aggFactory.h"

class AggSchema :
  public CommonTable::AggFunc
{
public:
  AggSchema();
  
  
  virtual ~AggSchema();

  
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
  DECLARE_PUBLIC_INITS(AggSchema)


private:
  /** The current schema value for this aggregate */
  ListPtr _schema;


  // This is necessary for the class to register itself with the
  // factory.
  DECLARE_PRIVATE_INITS


};


#endif // AGGSCHEMA_H
