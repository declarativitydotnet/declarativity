// -*- c-basic-offset: 2;
/*
 * @(#)$Id$
 *
 * Modified from the Click synchronization primitives by Eddie Kohler
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
 * DESCRIPTION: Synchronization primitives
 */
#ifndef __SYNC_H__
#define __SYNC_H__
#include <atomic.h>

// loop-in-cache spinlock implementation: 8 bytes. if the size of this class
// changes, change size of padding in ReadWriteLock below.

class Spinlock { public:

    Spinlock()			{ }

    void acquire()		{ }
    void release()		{ }
    bool attempt()		{ return true; }
    bool nested() const		{ return false; }
  
};

class SpinlockIRQ { public:

    SpinlockIRQ()		{ }

    typedef int flags_t;
    
    flags_t acquire()		{ return 0; }
    void release(flags_t)	{ }
  
};


// read-write lock:
//
// on read: acquire local read lock
// on write: acquire every read lock
//
// alternatively, we could use a read counter and a write lock. we don't do
// that because we'd like to avoid a cache miss for read acquires. this makes
// reads very fast, and writes more expensive

class ReadWriteLock { public:
  
    ReadWriteLock()				{ }
  
    void acquire_read()				{ }
    bool attempt_read()				{ return true; }
    void release_read()				{ }
    void acquire_write()			{ }
    bool attempt_write()			{ return true; }
    void release_write()			{ }

};

#endif
