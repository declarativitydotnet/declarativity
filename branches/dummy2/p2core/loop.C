// -*- c-basic-offset: 2; related-file-name: "loop.h" -*-
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
 */

#include <stdexcept>
#include "loop.h"
#include "math.h"
#include "assert.h"
#include <sys/types.h>
#include <sys/socket.h>
#include "fcntl.h"
#include "val_time.h"
#include "reporting.h"

callbackQueueT callbacks;
long callbackID = 0;


std::string
timeCBHandle::toString()
{
  ostringstream o;
  o << "<timeCB ["
    << ID
    << "]"
    << owner->name()
    << "@"
    << time
    << ">";
  return o.str();
}


timeCBHandle*
delayCB(double secondDelay, b_cbv cb, Element* owner)
{
  assert(secondDelay >= 0.0);

  unsigned long secs = (unsigned long) secondDelay; 
  boost::posix_time::ptime expiration;
  boost::posix_time::
    time_duration dlay(0, 0, secs,
                       (long) ((secondDelay-secs) * PTIME_FRACTIONAL_FACTOR));

  // When will this expire?
  getTime(expiration);
  expiration += dlay;
  
  // Create handle for this request
  timeCBHandle* handle = new timeCBHandle(expiration, cb, owner);

  // Place it into the priority queue
  callbacks.insert(handle);

  // Return it
  return handle;
}


void
timeCBRemove(timeCBHandle* handle)
{
  // Do not remove this callback outside of the main loop.
  handle->active = false;
}


tcpHandle*
tcpConnect(in_addr addr, u_int16_t port, b_cbi cb)
{
  return NULL;
}


/** Go up to current time and empty out the expired elements from the
    callback queue */
void
timeCBCatchup(boost::posix_time::time_duration& waitDuration)
{
  boost::posix_time::ptime now;
  try {
    getTime(now);
  } catch (std::exception e) {
    LOOP_ERROR("timeCBCatchup exception getting current time A: '"
               << e.what()
               << "'");
  }


  ////////////////////////////////////////////////////////////
  // Empty the queue prefix that has already expired

  callbackQueueT::iterator iter = callbacks.begin();
 
  while ((iter != callbacks.end()) &&
         ((*iter)->time <= now)) {
    // Remove this callback from the queue. The iterator must be
    // incremented before its previous position is erased!
    timeCBHandle* theCallback = *iter;
    callbackQueueT::iterator toErase = iter;
    iter++;
    callbacks.erase(toErase);

    LOOP_INFO("Handling callback "
              << theCallback->toString());
    
    // Run it
    if (theCallback->active &&
        (theCallback->owner == NULL || 
         theCallback->owner->state() == Element::ACTIVE)) {
      try {
        LOOP_WORDY("Invoking timeCBCatchup callback on "
                   << theCallback->toString());
        (theCallback->callback)();
      } catch (std::exception e) {
        LOOP_ERROR("timeCBCatchup callback invocation exception on "
                   << theCallback->toString()
                   << " with message "
                   << e.what());
      }
    }
    
    // And erase it
    delete theCallback;

    // Update the time in case the previous callback took too long.
    // Due to Eric Yu-En Lu
    try {
      getTime(now);
    } catch (std::exception e) {
      LOOP_ERROR("timeCBCatchup exception getting current time B: '"
                 << e.what()
                 << "'");
    }
  }

  /** Time to clean house: remove all inactive callbacks */
  for (iter = callbacks.begin();
       iter != callbacks.end();
       ) {
    if ((*iter)->active == false) {
      timeCBHandle* theCallback = *iter;
      callbackQueueT::iterator toKill = iter;
      iter++;
      callbacks.erase(toKill);
      delete theCallback;
    } else {
      iter++;
    }
  }

  ////////////////////////////////////////////////////////////
  // Set the wait duration to be the time from now till the first
  // scheduled event
  
  // Update current time
  try {
    getTime(now);
  } catch (std::exception e) {
    LOOP_ERROR("timeCBCatchup exception getting current time C: '"
               << e.what()
               << "'");
  }

  // Get first waiting time
  if (callbacks.empty()) {
    // Nothing to worry about. Leave the wait duration alone
  } else {
    iter = callbacks.begin();
    assert(iter != callbacks.end()); // since it's not empty

    if ((*iter)->time < now) {
      // Oops, the first callback has already expired. Don't wait, just
      // poll
      waitDuration = boost::posix_time::minutes(0);
    } else {
      // Update the wait duration to be what's left until the next
      // deadline.
      waitDuration = (*iter)->time - now;
    }
  }
}



////////////////////////////////////////////////////////////
// File descriptor stuff
////////////////////////////////////////////////////////////

int
networkSocket(int type, u_int16_t port, u_int32_t addr)
{
  int s;
  // Create it
  s = socket(AF_INET, type, 0);
  if (s < 0) {
    // Ooops, couldn't allocate it. No can do.
    return -1;
  }

  
  // No bind the socket to the given address
  struct sockaddr_in sin;

  // Setup the address sturctures
  bzero(&sin, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_port = htons(port);
  sin.sin_addr.s_addr = htonl(addr);

  // And bind
  if (bind(s, (struct sockaddr *) &sin, sizeof(sin)) < 0) {
    // Hmm, couldn't bind this socket. Close it and return failure.
    goto errorAfterOpen;
  } else {
    // Now enable keep alives
    int value = true;
    if (setsockopt(s, SOL_SOCKET, SO_KEEPALIVE,
                   &value, sizeof(value)) == -1) {
      // Hmm, couldn't set the socket option
      goto errorAfterOpen;
    }
    
    // And make the file descriptor non-blocking
    value = fcntl(s, F_GETFL, 0);
    if (value < 0) {
      // Couldn't read the file descriptor flags
      goto errorAfterOpen;
    }
    value |= O_NONBLOCK;
    if (fcntl(s, F_SETFL, value) < 0) {
      // Couldn't set the file descriptor flags
      goto errorAfterOpen;
    }
    return s;
  }

 errorAfterOpen:
  close(s);
  return -1;
}

fileDescriptorCallbackDirectoryT fileDescriptorCallbacks;

/** The read bitvector keeping track of set callbacks */
static fd_set readBits;

/** The write bitvector keeping track of set callbacks */
static fd_set writeBits;

/** The first untouched filedescriptor */
static int nextFD = 0;

bool
fileDescriptorCB(int fileDescriptor,
                 b_selop op,
                 b_cbv callback,
                 Element* owner)
{
  assert(fileDescriptor >= 0);
  assert(callback);

  // Do I have the entry already?
  fileDescriptorCBHandle handle(fileDescriptor, op, callback, owner);
  fileDescriptorCallbackDirectoryT::iterator iter =
    fileDescriptorCallbacks.find(&handle);
  if (iter == fileDescriptorCallbacks.end()) {
    // Nope, none exists. Just create and insert it
    fileDescriptorCBHandle* newHandle =
      new fileDescriptorCBHandle(fileDescriptor, op, callback, owner);
    fileDescriptorCallbacks.insert(newHandle);

    // And turn on the appropriate bit
    switch(op) {
    case b_selread:
      FD_SET(fileDescriptor, &readBits);
      break;
    case b_selwrite:
      FD_SET(fileDescriptor, &writeBits);
      break;
    default:
      // No such enum exists
      break;
    }

    // Finally, check if the next untouched file descriptor must be
    // updated
    nextFD = std::max(fileDescriptor + 1, nextFD);

    return true;
  } else {
    // It already exists. Just replace its callback
    (*iter)->setCallback(callback, owner);
    return false;
  }
}

bool
removeFileDescriptorCB(int fileDescriptor,
                       b_selop operation)
{
  assert(fileDescriptor >= 0);

  fileDescriptorCBHandle handle(fileDescriptor, operation);
  // Must find it so that we can delete the element

  fileDescriptorCallbackDirectoryT::iterator iter =
    fileDescriptorCallbacks.find(&handle);
  bool found;
  if (iter == fileDescriptorCallbacks.end()) {
    // Didn't find anything
    found = false;
  } else {
    found = true;
    delete (*iter);
    fileDescriptorCallbacks.erase(iter);
  }

  // And turn off the appropriate bit
  switch(operation) {
  case b_selread:
    FD_CLR(fileDescriptor, &readBits);
    break;
  case b_selwrite:
    FD_CLR(fileDescriptor, &writeBits);
    break;
  default:
    // No such enum exists
    break;
  }

  return found;
}


/** Wait for any pending file descriptor actions for the given time
    period. */
void
fileDescriptorCatchup(boost::posix_time::time_duration& waitDuration)
{
  // Copy the bit sets over
  static fd_set readResultBits;
  static fd_set writeResultBits;
  memcpy(&readResultBits, &readBits, sizeof(fd_set));
  memcpy(&writeResultBits, &writeBits, sizeof(fd_set));
  timespec td_ts;

  td_ts.tv_sec = waitDuration.total_seconds();
  // ensure we compute nanosecs (1/(10^9) sec) even if boost is compiled to lower 
  // precision 
  td_ts.tv_nsec = waitDuration.fractional_seconds() * PTIME_SECS_FACTOR;
  assert(td_ts.tv_nsec >= 0);

  int result = pselect(nextFD, &readResultBits, &writeResultBits,
                       NULL, &td_ts, NULL);
  if (result == -1) {
    // Ooops, error
    LOOP_ERROR("pselect failed");
    exit(-1);
  } else if (result == 0) {
    // Nothing happened
    LOOP_WORDY("pselect says nothing happened");
    return;
  } else {
    // Go through and call all requisite callbacks, first all writes,
    // then all reads
    LOOP_WORDY("pselect says something happened");
    for (int i = 0;
         i < nextFD;
         i++) {
      if (FD_ISSET(i, &writeResultBits)) {
        // Fetch the callback
        fileDescriptorCBHandle handle(i, b_selwrite);
        static fileDescriptorCallbackDirectoryT::iterator iter;
        iter = fileDescriptorCallbacks.find(&handle);

        // It'd better be there and be the right thing
        assert(iter != fileDescriptorCallbacks.end());
        assert((*iter)->fileDescriptor == i);
        assert((*iter)->operation == b_selwrite);

        // Call the callback
        if ((*iter)->owner == NULL || 
            (*iter)->owner->state() == Element::ACTIVE) {
          ((*iter)->callback)();
        } else {
          LOOP_INFO("NOT RUNNING CALLBACK: element not active");
        }
      }
    }
    for (int i = 0;
         i < nextFD;
         i++) {
      if (FD_ISSET(i, &readResultBits)) {
        // Fetch the callback
        fileDescriptorCBHandle handle(i, b_selread);
        static fileDescriptorCallbackDirectoryT::iterator iter;
        iter = fileDescriptorCallbacks.find(&handle);

        // It'd better be there and be the right thing
        assert(iter != fileDescriptorCallbacks.end());
        assert((*iter)->fileDescriptor == i);
        assert((*iter)->operation == b_selread);

        // Call the callback
        if ((*iter)->owner == NULL || 
            (*iter)->owner->state() == Element::ACTIVE) {
          ((*iter)->callback)();
        } else {
          LOOP_INFO("NOT RUNNING CALLBACK: element not active");
        }
      }
    }
  }
}


////////////////////////////////////////
//Process callbacks...
///////////////////////////////////////
TProcesses*
procs()
{
  static TProcesses* theProcs = new TProcesses();
  return theProcs;
}

void
registerProcess(IProcess* aProc)
{
  procs()->push_back(aProc);
}

void
removeProcess(IProcess* aProc)
{
  procs()->remove(aProc);
}

void
processCatchup(boost::posix_time::time_duration& waitDuration)
{
  boost::posix_time::time_duration w2 = waitDuration;
  TProcesses::iterator it = procs()->begin();
  for(;
      it != procs()->end();
      it++) {
    (*it)->proc(&w2);
    if(w2 < waitDuration) {
      waitDuration = w2;
    }
  }
}


IProcess::~IProcess()
{
}



////////////////////////////////////////////////////////////
// Main loop
////////////////////////////////////////////////////////////

void
eventLoopInitialize()
{
  FD_ZERO(&readBits);
  FD_ZERO(&writeBits);
}


void
eventLoop()
{
  // The wait duration for file descriptor waits. It is set by
  // timeCBCatchup and used by fileDescriptorCatchup.  Equivalent to
  // selwait in libasync
  boost::posix_time::time_duration waitDuration =
    boost::posix_time::seconds(1);

  while (1) {
    try {
      timeCBCatchup(waitDuration);
    } catch (std::exception e) {
      LOOP_ERROR("timeCBCatchup exception in loop: '"
                 << e.what()
                 << "'");
    }

    processCatchup(waitDuration);
    
    try {
      fileDescriptorCatchup(waitDuration);
    } catch (std::exception e) {
      LOOP_ERROR("fileDescriptorCatchup exception in loop: '"
                 << e.what()
                 << "'");
    }
    
  }
}


fileDescriptorCBHandle::fileDescriptorCBHandle(int fd,
                                               b_selop op,
                                               b_cbv& cb,
                                               Element* o)
  : fileDescriptor(fd), operation(op), callback(cb), owner(o)
{ 
  assert(fd > 0);
}


fileDescriptorCBHandle::fileDescriptorCBHandle(int fd,
                                               b_selop op)
  : fileDescriptor(fd), operation(op), callback(0)
{
  assert(fd > 0);
}

