// -*- c-basic-offset: 2; related-file-name: "loop.C" -*-
/*
 * @(#)$Id$
 *
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 * DESCRIPTION: Base class for P2 event loop
 *
 */

#ifndef __LOOP_H__
#define __LOOP_H__

#include <string>
#include <sstream>
#include <iostream>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <set>

// For in_addr
#include <netinet/in.h>

#include <p2Time.h>
#include <time.h>

using std::string;
using std::ostringstream;

#undef warn
#define warn std::cerr
#undef fatal
#define fatal std::cerr



////////////////////////////////////////////////////////////
// Callbacks
////////////////////////////////////////////////////////////

typedef boost::function<void (void)>        b_cbv;
typedef boost::function<void (int)>         b_cbi;
typedef boost::function<void (std::string)> b_cbs;
typedef boost::function<void (bool)>        b_cbb;


/** The global counter of callbacks */
extern long callbackID;



////////////////////////////////////////////////////////////
// Timed callbacks 
////////////////////////////////////////////////////////////

/** A callback record */
struct timeCBHandle {
public:
  /** What was my time target? */
  struct timespec time;
  
  /** What was my callback? */
  const b_cbv callback;
  
  /** My ID */
  long ID;

  /** Construct me */
  timeCBHandle(struct timespec& t, const b_cbv& cb)
    : time(t), callback(cb), ID(callbackID++)
  {
    assert((t.tv_nsec >=0) && (t.tv_nsec < 1000 * 1000 * 1000));
  }
};


/** Delay a callback for a given time (in seconds). It returns a handle
    that can be used to unschedule the callback. If the delay is
    non-positive, it is interpreted as a delayed callback, to be invoked
    as soon as possible. */
timeCBHandle*
delayCB(double secondDelay, b_cbv cb);


/** Unschedule a scheduled callback */
void
timeCBRemove(timeCBHandle *);


/** My ordering predicate for callback handles. Returns true if the
    first time is less than the second, or if they're equal but the
    first ID is less than the second. */
struct timeCBHandleLess
{
  bool operator()(const timeCBHandle* first,
                  const timeCBHandle* second) const
  {
    return ((compare_timespec(first->time, second->time) < 0) ||
            ((compare_timespec(first->time, second->time) == 0) &&
             (first->ID < second->ID)));
  }
};


/** The timed callbacks sorted set type */
typedef std::set<timeCBHandle*, timeCBHandleLess> callbackQueueT;


/** The timed callback queue. */
extern callbackQueueT callbacks;






////////////////////////////////////////////////////////////
// File descriptor callbacks 
////////////////////////////////////////////////////////////

/** Operation type for file descriptor operations. */
enum
b_selop {
  b_selread = 0,
  b_selwrite = 1
};


/** A file descriptor callback record */
struct fileDescriptorCBHandle {
public:
  /** What's my file descriptor? */
  int fileDescriptor;


  /** What's my operation? */
  b_selop operation;


  /** What's my callback? */
  b_cbv callback;

  
  /** Construct me */
  fileDescriptorCBHandle(int fd,
                         b_selop op,
                         b_cbv& cb)
    : fileDescriptor(fd), operation(op), callback(cb)
  { 
    assert(fd > 0);
  }

  
  /** Construct me without a callback */
  fileDescriptorCBHandle(int fd,
                         b_selop op)
    : fileDescriptor(fd), operation(op), callback(0)
  {
    assert(fd > 0);
  }
                         

  /** Change my callback. It's OK to mutate me because the callback does
      not participate in the sorting function. */
  void
  setCallback(const b_cbv& cb) { callback = cb; }
};


/** My ordering predicate for file descriptor callbacks. Returns true if
    the first FD is less than the second, or if they're equal but the
    first operation is less than the second. */
struct fileDescriptorCBHandleLess
{
  bool operator()(const fileDescriptorCBHandle* first,
                  const fileDescriptorCBHandle* second) const
  {
    return ((first->fileDescriptor < second->fileDescriptor) ||
            ((first->fileDescriptor == second->fileDescriptor) &&
             (first->operation < second->operation)));
  }
};


/** The file descriptor callbacks sorted set type */
typedef std::set<fileDescriptorCBHandle*,
                 fileDescriptorCBHandleLess> fileDescriptorCallbackDirectoryT;


/** The file descriptor callback directory. */
extern fileDescriptorCallbackDirectoryT fileDescriptorCallbacks;


/** Create a non-blocking network socket, given its type */
int
networkSocket(int type, u_int16_t port, u_int32_t addr);


/** Set a callback for asynchronous operations of a given type
    (read/write) on a file descriptor.  If such a callback has already
    been set, replace it. Return true if the callback replaced another,
    false otherwise. */
bool
fileDescriptorCB(int fileDescriptor,
                 b_selop operation,
                 b_cbv callback);


/** Remove a callback from asynchronous operations of a given type
    (read/write) on a file descriptor.  Do nothing if no such callback
    has been set.  Return true if such a callback existed, false
    otherwise. */
bool
removeFileDescriptorCB(int fileDescriptor,
                       b_selop operation);

















/** A TCP control block */
struct tcpHandle {
};


tcpHandle*
tcpConnect(in_addr addr, u_int16_t port, b_cbi cb);






/** The main loop function.  Does not return. */
void
eventLoop();


/** The initialization function for the event loop. Must be called
    before any code that affect the event loop (e.g., before any
    callbacks are registered, non-blocking sockets have opened, etc.) */
void
eventLoopInitialize();


#endif /* __LOOP_H_ */
