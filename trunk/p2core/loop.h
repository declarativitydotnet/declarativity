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
 * DESCRIPTION: Base class for P2 event loop.
 *
 */

#ifndef __LOOP_H__
#define __LOOP_H__

#include <set>
#include <list>

// For in_addr
#include <netinet/in.h>

#include <p2Time.h>
#include "boost/date_time/posix_time/posix_time.hpp"
#include "element.h"

extern "C" {
#include <rpc/rpc.h>
#include <rpc/xdr.h>
}

/** The global counter of callbacks */
extern long callbackID;



/** Reporting support */

#include <errno.h>
#define LOOP_LOG(_reportingLevel,_rest) "Event Loop, "           \
  << __FILE__                                                    \
  << ":"                                                         \
  << __LINE__                                                    \
  << ", "                                                        \
  << _reportingLevel                                             \
  << ", "                                                        \
  << errno                                                       \
  << ", "                                                        \
  << _rest


#define LOOP_ERROR(_rest) TELL_ERROR   \
  << LOOP_LOG(Reporting::ERROR,_rest)  \
    << "\n"

#define LOOP_WORDY(_rest) TELL_WORDY   \
  << LOOP_LOG(Reporting::WORDY,_rest)  \
    << "\n"

#define LOOP_INFO(_rest) TELL_INFO   \
  << LOOP_LOG(Reporting::INFO,_rest) \
    << "\n"

#define LOOP_WARN(_rest) TELL_WARN   \
  << LOOP_LOG(Reporting::WARN,_rest) \
    << "\n"




////////////////////////////////////////////////////////////
// Timed callbacks 
////////////////////////////////////////////////////////////
class Element;

/** A callback record */
struct timeCBHandle {
public:
  /** The element that owns the callback */
  Element* owner;

  /** Is this callback active? */
  bool active;

  /** What was my time target? */
  boost::posix_time::ptime time;
  
  /** What was my callback? */
  const b_cbv callback;
  
  /** My ID */
  long ID;

  /** Construct me */
  timeCBHandle(boost::posix_time::ptime& t, const b_cbv& cb, Element* e)
    : owner(e), active(true), time(t), callback(cb), ID(callbackID++)
  {}

  
  /** Print me */
  std::string toString();
};


/** Delay a callback for a given time (in seconds). It returns a handle
    that can be used to unschedule the callback. If the delay is
    non-positive, it is interpreted as a delayed callback, to be invoked
    as soon as possible. */
timeCBHandle*
delayCB(double secondDelay, b_cbv cb, Element* owner=NULL);


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
    return ((first->time < second->time) ||
            ((first->time == second->time) &&
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

  /** What is the element to be called */
  Element* owner;
  
  /** Construct me */
  fileDescriptorCBHandle(int fd,
                         b_selop op,
                         b_cbv& cb,
                         Element* o);

  
  /** Construct me without a callback */
  fileDescriptorCBHandle(int fd,
                         b_selop op);
                         

  /** Change my callback and owner. It's OK to mutate me because the 
      callback and owner do not participate in the sorting function. */
  void
  setCallback(const b_cbv& cb, Element* o) 
  { callback = cb; owner = o; }
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
                 b_cbv callback,
                 Element* owner=NULL);


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


/**A Process callback used by scheduler***/
class IProcess{
public:
  virtual void proc(boost::posix_time::time_duration*)=0;

  virtual ~IProcess();
};
typedef std::list<IProcess*> TProcesses;

TProcesses*
procs();

void registerProcess(IProcess*);
void removeProcess(IProcess*);


/** The initialization function for the event loop. Must be called
    before any code that affects the event loop (e.g., before any
    callbacks are registered, non-blocking sockets have opened, etc.) */
void
eventLoopInitialize();

/** The main loop function.  Does not return. */
void
eventLoop();

#endif /* __LOOP_H_ */
