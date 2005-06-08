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
 * DESCRIPTION: Receive buffer for file descriptors.
 *
 */

#ifndef __FDBUF_H__
#define __FDBUF_H__

#include <cstdlib>
#include <string>
#include <sstream>
#include <errno.h>

/*
  A brief implementation discussion is in order.  Unlike (say) DM's
  suio class, this doesn't do (a) scatter gather using iovecs or
  similar, or (b) refcounted buffer management.   It's going to copy
  things rather more, in other words.  It does, however, have the
  benefit of being easy to understand. 

  Moving forward [cheer], we should benchmark alternatives.  One
  interesting compromise is replace the malloc'ed 'data' with a
  std::basic_string, and rely on the latter's implementation.  With a
  good string implementation, we should minimise copy in and out. 
*/

class Fdbuf {

private:
  size_t capacity;	// Capacity of the buffer so far.
  size_t len;		// Number of bytes actually held.
  size_t start;		// Offset of first valid byte
  int	 err;		// Last value of errno. 
  char	*data;		// Data itself

  // Used by write, send, sendto...
  inline ssize_t post_write(ssize_t w) {
    err = errno;
    if (w > 0) { start += w; len =- w; }
    return w;
  };
  
  // Used by read, recv, recvfrom...
  inline ssize_t post_read(ssize_t r) {
    err = errno;
    if (r > 0) { len += r; }
    return r;
  };


public:

  // Size constants.
  enum { 
    BUF_DFLT_CAP = 1500,	// Default capacity of the buffer.
    BUF_DFLT_READ = 1500,	// Default quantity to read.
    BUF_UNLIMITED = -1,		// Unlimited capacity.
    BUF_INCREMENT = 0x80	// Granularity of buffer growing.
  };

  Fdbuf( int init_capacity = BUF_DFLT_CAP );
  ~Fdbuf();
  
  //
  // INPUT FUNCTIONS: stuff that adds data to the tail of the buffer.
  //
  
  // Read some data into the buffer from a file descriptor.  As with
  // read(1), return the number of bytes read this time, 0 if EOF, or
  // -1 if there was some "real" error (including EAGAIN).
  ssize_t read(int fd, size_t max_read = BUF_DFLT_READ);
  ssize_t recv(int sd, size_t max_read = BUF_DFLT_READ, int flags=0);
  ssize_t recvfrom(int sd, size_t max_read = BUF_DFLT_READ, int flags=0,
		   struct sockaddr *from=NULL, socklen_t *fromlen=0);
  
  // Member functions to append values to the buffer.
  const Fdbuf &push_back(const Fdbuf &fb );
  const Fdbuf &push_back(const std::string &s);
  const Fdbuf &push_back(const char *buf, size_t len);
  const Fdbuf &push_back(const char *str);
  // Could probably be more efficient...
  template <class T> const Fdbuf &push_back(const T &t) { 
    std::ostringstream os; 
    os << t; 
    return push_back(os.str()); 
  };

  // Operators
  template <class T> const Fdbuf &operator<< (const T &t) { return push_back(t); }
 
  //
  // OUTPUT FUNCTIONS: remove stuff from the head of the buffer. 
  //

  // Write data to a file descriptor.  Write max_write bytes at most,
  // or else the whole valid chunk of data.  As with write(1), return
  // the number of bytes written (which will be subtracted from the
  // buffer), or 0 if EAGAIN, or -1 in the event of some other error. 
  ssize_t write(int fd, ssize_t max_write = BUF_UNLIMITED);
  ssize_t send(int sd, ssize_t max_write = BUF_UNLIMITED, int flags=0);
	       
  ssize_t sendto(int sd, ssize_t max_write = BUF_UNLIMITED, int flags=0,
		 const struct sockaddr *to=NULL, socklen_t tolen=0 );

  //
  // ACCESS FUNCTIONS: those that aren't quite INPUT or OUTPUT. 
  //
  
  // Return the last value of errno. 
  int last_errno() { return err; };
  
  // Remove all data in the buffer.
  void clear() { len = 0; start = 0; };
  
  // How much valid data is in the buffer?
  size_t length() { return len; };
  
  // Return the buffer as a C++ string - the whole length contents of
  // the buffer.
  std::string str() { return std::string(data, len); };
  
  // Return the buffer as a C string - terminate the string with a
  // null character.  Embedded nulls in the string are your own
  // problem.  The pointer is good until you call any other method.  
  char *cstr() { ensure_additional(1); data[len+start] = '\0'; return data; };

  // Make sure the buffer has sufficient capacity
  void ensure(size_t new_capacity);
  void ensure_additional(size_t extra) { ensure(extra + len); };
};


#endif /* __FDBUF_H_ */
