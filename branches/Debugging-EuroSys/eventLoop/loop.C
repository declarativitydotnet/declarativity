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

#include "loop.h"
#include <time.h>
#include "math.h"
#include "assert.h"
#include <sys/types.h>
#include <sys/socket.h>
#include "fcntl.h"

callbackQueueT callbacks;
long callbackID = 0;

timeCBHandle*
delayCB(double secondDelay, b_cbv cb)
{
  assert(secondDelay >= 0.0);

  // When will this expire?
  struct timespec expiration;
  getTime(expiration);          // now
  increment_timespec(expiration, secondDelay);
  
  // Create handle for this request
  timeCBHandle* handle = new timeCBHandle(expiration, cb);

  // Place it into the priority queue
  callbacks.insert(handle);

  // Return it
  return handle;
}

void
timeCBRemove(timeCBHandle* handle)
{
  callbacks.erase(handle);
}

tcpHandle*
tcpConnect(in_addr addr, u_int16_t port, b_cbi cb)
{
  return NULL;
}


/** Go up to current time and empty out the expired elements from the
    callback queue */
void
timeCBCatchup(struct timespec& waitDuration)
{
  struct timespec now;
  getTime(now);

  ////////////////////////////////////////////////////////////
  // Empty the queue prefix that has already expired

  callbackQueueT::iterator iter = callbacks.begin();
  while ((iter != callbacks.end()) &&
         (compare_timespec((*iter)->time, now) <= 0)) {
    // Remove this callback from the queue
    timeCBHandle* theCallback = *iter;
    callbacks.erase(iter);
    iter++;
    
    // Run it
    (theCallback->callback)();
    
    // And erase it
    delete theCallback;
  }

  ////////////////////////////////////////////////////////////
  // Set the wait duration to be the time from now till the first
  // scheduled event
  
  // Update current time
  getTime(now);

  // Get first waiting time
  if (callbacks.empty()) {
    // Nothing to worry about. Set it to a minute
    waitDuration.tv_sec = 60;
    waitDuration.tv_nsec = 0;

  } else {
    iter = callbacks.begin();
    assert(iter != callbacks.end()); // since it's not empty

    if (compare_timespec((*iter)->time, now) < 0) {
      // Oops, the first callback has already expired. Don't wait, just
      // poll
      waitDuration.tv_sec = 0;
      waitDuration.tv_nsec = 0;
    } else {
      subtract_timespec(waitDuration, (*iter)->time, now);
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
                 b_cbv callback)
{
  assert(fileDescriptor >= 0);
  assert(callback);

  // Do I have the entry already?
  fileDescriptorCBHandle handle(fileDescriptor, op, callback);
  fileDescriptorCallbackDirectoryT::iterator iter =
    fileDescriptorCallbacks.find(&handle);
  if (iter == fileDescriptorCallbacks.end()) {
    // Nope, none exists. Just create and insert it
    fileDescriptorCBHandle* newHandle =
      new fileDescriptorCBHandle(fileDescriptor, op, callback);
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
    (*iter)->setCallback(callback);
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
    fileDescriptorCallbacks.erase(iter);
    delete (*iter);
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
fileDescriptorCatchup(struct timespec& waitDuration)
{
  // Copy the bit sets over
  static fd_set readResultBits;
  static fd_set writeResultBits;
  memcpy(&readResultBits, &readBits, sizeof(fd_set));
  memcpy(&writeResultBits, &writeBits, sizeof(fd_set));

  int result = pselect(nextFD, &readResultBits, &writeResultBits,
                       NULL, &waitDuration, NULL);
  if (result == -1) {
    // Ooops, error
    fatal << "pselect failed";
    exit(-1);
  } else if (result == 0) {
    // Nothing happened
    return;
  } else {
    // Go through and call all requisite callbacks, first all writes,
    // then all reads
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
        ((*iter)->callback)();
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
        ((*iter)->callback)();
      }
    }
  }
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
  struct timespec waitDuration;

  while (1) {
    timeCBCatchup(waitDuration);
    fileDescriptorCatchup(waitDuration);
  }
}



