// -*- c-basic-offset: 2; related-file-name: "loop.h" -*-
/*
 * @(#)$Id: loop.C,v 1.29 2007/03/10 05:38:09 maniatis Exp $
 *
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 */

#include <winsock2.h>
#include <wchar.h>
#include <stdexcept>
#include "loop.h"
#include "math.h"
#include "assert.h"
#include <sys/types.h>
//#include <sys/socket.h>
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
tcpConnect(in_addr addr, uint16_t port, b_cbi cb)
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
    // Remove this callback from the queue
    timeCBHandle* theCallback = *iter;
	iter = callbacks.erase(iter);
    
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
  for (iter = callbacks.begin(); iter != callbacks.end(); iter++) {
    if ((*iter)->active == false) {
      timeCBHandle* theCallback = *iter;
      callbacks.erase(iter);
      delete theCallback;
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
    // Nothing to worry about. Set it to a minute
    waitDuration = boost::posix_time::minutes(1);
  } else {
    iter = callbacks.begin();
    assert(iter != callbacks.end()); // since it's not empty

    if ((*iter)->time < now) {
      // Oops, the first callback has already expired. Don't wait, just
      // poll
      waitDuration = boost::posix_time::minutes(0);
    } else {
      waitDuration = (*iter)->time - now;
    }
  }
}



////////////////////////////////////////////////////////////
// File descriptor stuff
////////////////////////////////////////////////////////////

int
networkSocket(int type, uint16_t port, uint32_t addr, int proto)
{
  int s;
  // Create it
  s = WSASocket(AF_INET, type, proto, NULL, 0, WSA_FLAG_OVERLAPPED);
  if (s == INVALID_SOCKET) {
    // Ooops, couldn't allocate it. No can do.
    int errcode = WSAGetLastError();
    TELL_ERROR << "Socket creation failure, error code " << errcode << "\n";
    return -1;
  }
  TELL_INFO "Created socket " << s << "(" << type << "," << proto << ",NULL,0,WSA_FLAG_OVERLAPPED\n";

  
  // Now bind the socket to the given address
  SOCKADDR_IN  sin;

  // Setup the address sturctures
  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_port = htons(port);
  sin.sin_addr.s_addr = htonl(addr);

  // And bind
  if (bind(s, (LPSOCKADDR) &sin, sizeof(struct sockaddr)) < 0) {
    // Hmm, couldn't bind this socket. Close it and return failure.
    goto errorAfterOpen;
  } else {
	TELL_INFO "Bound socket " << s << "to port " << port << ", addr " << addr << "\n";

	  // Now enable keep alives
    BOOL value = true;
	int errcode = WSAGetLastError();
	if (type != SOCK_DGRAM)
	    if (setsockopt(s, SOL_SOCKET, SO_KEEPALIVE,
            (const char *)&value, sizeof(value)) == -1) {
          // Hmm, couldn't set the socket option
          TELL_ERROR << "Failed setsockopt\n";
          goto errorAfterOpen;
        }
    
    // And make the file descriptor non-blocking
#ifdef JOE_NOTWINDOWS
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
#else
	unsigned long tmpval = 1;
	value = ioctlsocket(s,FIONBIO, &tmpval);
    TELL_INFO "ioctlsocket(" << s << ",FIONBIO, &1)\n";

	if (value < 0)
			// problem with socket
			goto errorAfterOpen;
#endif
	return s;
  }

 errorAfterOpen:
  int errcode = WSAGetLastError();
  closesocket(s);
  TELL_ERROR << "error in setting up open socket.  error code " << errcode << "\n"; 
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
    TELL_INFO << "creating filedesc " << fileDescriptor << "\n";
    fileDescriptorCBHandle* newHandle =
      new fileDescriptorCBHandle(fileDescriptor, op, callback, owner);
    fileDescriptorCallbacks.insert(newHandle);

    // And turn on the appropriate bit
    switch(op) {
    case b_selread:
      TELL_INFO "Setting read bit " << fileDescriptor << "\n";
      FD_SET(fileDescriptor, &readBits);
      break;
    case b_selwrite:
      TELL_INFO "Setting write bit " << fileDescriptor << "\n";
      FD_SET(fileDescriptor, &writeBits);
      break;
    default:
      // No such enum exists
      break;
    }

    // Finally, check if the next untouched file descriptor must be
    // updated
    nextFD = max(fileDescriptor + 1, nextFD);

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
  TELL_INFO << "removing filedesc " << fileDescriptor << "\n";

  fileDescriptorCBHandle handle(fileDescriptor, operation);
  // Must find it so that we can delete the element

  fileDescriptorCallbackDirectoryT::iterator iter =
    fileDescriptorCallbacks.find(&handle);
  bool found;
  if (iter == fileDescriptorCallbacks.end()) {
    // Didn't find anything
    found = false;
  } else {
    fileDescriptorCBHandle *delme = *iter;
    found = true;
    fileDescriptorCallbacks.erase(iter);
    delete (delme);
  }

  // And turn off the appropriate bit
  switch(operation) {
  case b_selread:
    TELL_INFO "Clearing read bit " << fileDescriptor << "\n";
    FD_CLR(fileDescriptor, &readBits);
    break;
  case b_selwrite:
    TELL_INFO "Clearing write bit " << fileDescriptor << "\n";
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
//  timespec td_ts;
  timeval td_ts;

  TELL_INFO << "entering fileDescriptorCatchup\n";

  td_ts.tv_sec = waitDuration.total_seconds();
  // ensure we compute nanosecs (1/(10^9) sec) even if boost is compiled to lower 
  // precision 
//  td_ts.tv_nsec = waitDuration.fractional_seconds() * PTIME_SECS_FACTOR;
//  assert(td_ts.tv_nsec >= 0);
  // Microseconds are nanoseconds / 1000
  td_ts.tv_usec = (long)(waitDuration.fractional_seconds() * PTIME_SECS_FACTOR) / 1000;

  timeval tmptv = td_ts;
  int errcode_pre = WSAGetLastError();
  int result = SOCKET_ERROR;

  result = select(nextFD, &readResultBits, &writeResultBits,
                       NULL, (const timeval *)&tmptv);

//  int result = pselect(nextFD, &readResultBits, &writeResultBits,
//                       NULL, &td_ts, NULL);
  if (result == SOCKET_ERROR) {
    // Ooops, error
	int errcode = WSAGetLastError();
	if (errcode == WSAEINVAL) {
		// very likely no bits are set, let's check.
		int foundw=0;
	    int foundr=0;
		for (int i = 0; i < nextFD; i++) {
			if (FD_ISSET(i,&writeResultBits)) {
				TELL_ERROR << "writeResultBit "<<i<<" was set.\n";
				foundw = 1;
			}
			if (FD_ISSET(i,&readResultBits)) {
				TELL_ERROR << "readResultBit "<<i<<" was set.\n";
				foundr = 1;
			}
		}
		if (!foundw && !foundr) {
			TELL_INFO << "no read or write resultBits set\n";
			return;
		}
		// else drop through
	}
    LOOP_ERROR("select failed with errcode " << errcode);
  } else if (result == 0) {
    // Nothing happened
	  TELL_INFO << "select successful with 0 result\n"; 
	  return;
  } else {
    // Go through and call all requisite callbacks, first all writes,
    // then all reads
    for (int i = 0;
         i < nextFD;
         i++) {
      if (FD_ISSET(i, &writeResultBits)) {
        // Fetch the callback
		  TELL_INFO << "select succeeded with write on bit " << i << "\n";
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
		TELL_INFO << "select succeeded with read on bit " << i << "\n";
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






////////////////////////////////////////////////////////////
// Main loop
////////////////////////////////////////////////////////////

void
eventLoopInitialize()
{
  FD_ZERO(&readBits);
  FD_ZERO(&writeBits);

  // BEGIN WINSOCK INITIALIZATION
  // Winsock initialization taken from http://msdn2.microsoft.com/en-us/library/ms742213.aspx
  WORD wVersionRequested;
  WSADATA wsaData;
  int err;
 
  wVersionRequested = MAKEWORD( 2, 0 );
  err = WSAStartup( wVersionRequested, &wsaData );
  if ( err != 0 ) {
    /* Tell the user that we could not find a usable */
    /* WinSock DLL.                                  */
    TELL_ERROR << "Could not initialize Windows Sockets (2.2)\n";
  }
 
  /* Confirm that the WinSock DLL supports 2.2.*/
  /* Note that if the DLL supports versions greater    */
  /* than 2.2 in addition to 2.2, it will still return */
  /* 2.2 in wVersion since that is the version we      */
  /* requested.                                        */
 
  if ( LOBYTE( wsaData.wVersion ) != 2 ||
       HIBYTE( wsaData.wVersion ) != 0 ) {
    /* Tell the user that we could not find a usable */
    /* WinSock DLL.                                  */
    WSACleanup( );
   TELL_ERROR << "Could not initialize Windows Sockets (2.0)\n"; 
  }
 
  /* The WinSock DLL is acceptable. Proceed. */
  TELL_INFO << "Initialized Winsock 2.0\n";
  // END WINSOCK INITIALIZATION

}


void
eventLoop()
{
  // The wait duration for file descriptor waits. It is set by
  // timeCBCatchup and used by fileDescriptorCatchup.  Equivalent to
  // selwait in libasync
  boost::posix_time::time_duration waitDuration;

  while (1) {
    try {
      timeCBCatchup(waitDuration);
    } catch (std::exception e) {
      LOOP_ERROR("timeCBCatchup exception in loop: '"
                 << e.what()
                 << "'");
    }
    
    try {
      fileDescriptorCatchup(waitDuration);
    } catch (std::exception e) {
      LOOP_ERROR("fileDescriptorCatchup exception in loop: '"
                 << e.what()
                 << "'");
    }
  }
}



