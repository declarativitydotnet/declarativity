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
 * DESCRIPTION: Receive buffer for file descriptors
 *
 */

#include "fdbuf.h"

#include <cerrno>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

//
// Constructor
//
Fdbuf::Fdbuf( int init_capacity )
  : capacity(init_capacity),
    len(0),
    start(0),
    err(0),
    data( new char[capacity] )
{
}

// 
// Destructor
//
Fdbuf::~Fdbuf()
{
  if (data) {
    delete data;
  }
}

//
// INPUT FUNCTIONS
//

//
// Read data in from a file descriptor
//
ssize_t Fdbuf::read(int fd, size_t max_read)
{
  ensure_additional( max_read );
  return post_read(::read(fd, data + len + start, max_read));
}
ssize_t Fdbuf::recv(int fd, size_t max_read, int flags)
{
  ensure_additional( max_read );
  return post_read(::recv(fd, data + len + start, max_read, flags));
}
ssize_t Fdbuf::recvfrom(int sd, size_t max_read, int flags,
			struct sockaddr *from, socklen_t *fromlen)
{
  ensure_additional( max_read );
  return post_read(::recvfrom(sd, data + start + len, max_read, 0, from, fromlen));
}

// Appending a string
const Fdbuf &Fdbuf::push_back(const std::string &s)
{
  ensure_additional(s.length());
  s.copy( data + start + len, s.length(), 0 );
  len += s.length();
  return *this;
}
const Fdbuf &Fdbuf::push_back(const char *buf, size_t size)
{
  ensure_additional(len);
  memcpy( data + start + len, buf, size );
  len += size;
  return *this;
}
const Fdbuf &Fdbuf::push_back(const char *str)
{
  return push_back(str,strlen(str));
}
const Fdbuf &Fdbuf::push_back(const Fdbuf &fb)
{
  return push_back( fb.data + start, fb.len );
}

//
// OUTPUT FUNCTIONS
//

//
// Write data
//
ssize_t Fdbuf::write(int fd, ssize_t max_write)
{
  return post_write(::write(fd, data + start, 
			    max_write<0 ? len : std::min(len,(size_t)max_write)));
}
ssize_t Fdbuf::send(int sd, ssize_t max_write, int flags)
{
  return post_write(::send(sd, data + start, 
			   max_write<0 ? len : std::min(len,(size_t)max_write),
			   flags ));
}
ssize_t Fdbuf::sendto(int sd, ssize_t max_write, int flags,
		      const struct sockaddr *to, socklen_t tolen )
{
  return post_write(::sendto(sd, data + start, 
			     max_write<0 ? len : std::min(len,(size_t)max_write),
			     flags, to, tolen ));
}

//
// Make sure the buffer is big enough
//
void Fdbuf::ensure(size_t new_capacity)
{
  if ( capacity < new_capacity ) {
    size_t new_size = (new_capacity + BUF_INCREMENT - 1) / BUF_INCREMENT * BUF_INCREMENT;
    char *old_buf = data;
    char *new_buf = new char[new_size];
    memcpy( new_buf, old_buf + start, len );
    data = new_buf;
    capacity = new_capacity;
    start = 0;
    delete old_buf;
  } 
  if ( capacity - start < new_capacity ) {
    memcpy( data, data + start, len );
  }
}

/*
 * End of file
 */
