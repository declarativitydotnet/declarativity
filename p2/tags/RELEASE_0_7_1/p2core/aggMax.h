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

#include "table2.h"
#include "value.h"

class AggMax :
  public Table2::AggFunc
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


  static AggMax*
  mk();




private:
  /** The current max value for this aggregate */
  ValuePtr _currentMax;
};


#endif // AGGMAX_H
