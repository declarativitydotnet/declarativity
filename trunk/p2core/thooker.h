// -*- c-basic-offset: 2; related-file-name: "timer.C" -*-
/*
 * @(#)$Id$
 *
 * Modified from the Click timer class by Eddie Kohler
 * 
 * Copyright (c) 1999-2000 Massachusetts Institute of Technology
 * Copyright (c) 2004 Regents of the University of California
 * Copyright (c) 2004 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software")
 * to deal in the Software without restriction, subject to the conditions
 * listed in the Click LICENSE file. These conditions include: you must
 * preserve this copyright notice, and you cannot mention the copyright
 * holders in advertising related to the Software without their permission.
 * The Software is provided WITHOUT ANY WARRANTY, EXPRESS OR IMPLIED. This
 * notice is a summary of the Click LICENSE file; the license in that file is
 * legally binding.
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 * DESCRIPTION: A timer that runs a given arbitrary function
 */
#ifndef __THOOKER_H__
#define __THOOKER_H__
#include <timer.h>

typedef void (*TimerHook)(Timer *, void *);

class THooker : public Timer {
 public:
  
  THooker(TimerHook, void *);

  /** Call the hook with the opaque data. */
  REMOVABLE_INLINE virtual void run();

 private:
  
  /** The function to call */
  TimerHook _hook;

  /** The opaque data to call the hook function with */
  void * _opaque;
};

#endif
