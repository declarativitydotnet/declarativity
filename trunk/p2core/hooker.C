// -*- c-basic-offset: 2; related-file-name: "hooker.h" -*-
/*
 * @(#)$Id$
 *
 * Modified from the Click Task class by Eddie Kohler and Benjie Chen
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
 * DESCRIPTION: A special task for calling arbitrary functions (TaskHooks)
 */

#include <hooker.h>

// - Changes to _thread are protected by _thread->lock.
// - Changes to _thread_preference are protected by
//   _router->master()->task_lock.
// - If _pending is nonzero, then _pending_next is nonnull.
// - Either _thread_preference == _thread->thread_id(), or
//   _thread->thread_id() == -1.


REMOVABLE_INLINE Hooker::Hooker(TaskHook hook, void *opaque)
  : Task(),
    _hook(hook),
    _opaque(opaque)
{
  assert(hook != 0);
}

REMOVABLE_INLINE void Hooker::run()
{
  (void) _hook(this, _opaque);
}

