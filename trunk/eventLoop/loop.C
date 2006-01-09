// -*- c-basic-offset: 2; related-file-name: "loop.h" -*-
/*
 * @(#)$Id$
 *
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

/** Wait for any pending file descriptor actions for the given time
    period. */
void
fileDescriptorCatchup(struct timespec& waitDuration)
{
  
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


bool
fileDescriptorCB(int fileDescriptor,
                 b_selop op,
                 b_cbv callback)
{
  assert(fileDescriptor >= 0);

  // Do I have the entry already?
  static fileDescriptorCBHandle handle(fileDescriptor, op, callback);
  fileDescriptorCallbackDirectoryT::iterator iter =
    fileDescriptorCallbacks.find(&handle);
  if (iter == fileDescriptorCallbacks.end()) {
    // Nope, none exists. Just create and insert it
    fileDescriptorCBHandle* newHandle =
      new fileDescriptorCBHandle(fileDescriptor, op, callback);
    fileDescriptorCallbacks.insert(newHandle);
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

  static fileDescriptorCBHandle handle(fileDescriptor, operation);
  int removed = fileDescriptorCallbacks.erase(&handle);
  return (removed > 0);
}





