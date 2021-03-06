// -*- c-basic-offset: 2; related-file-name: "fdbuf.h" -*-
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
 * DESCRIPTION: Receive buffer for file descriptors
 *
 */

#include "fdbuf.h"

#include <cerrno>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <assert.h>

//
// Constructor
//
Fdbuf::Fdbuf(int init_capacity, bool is_safe)
  : capacity(align(init_capacity)),
    len(0),
    start(0),
    err(0),
    data(new char[capacity]),
    safe(is_safe)
{
}

// 
// Destructor
//
Fdbuf::~Fdbuf()
{
  if (data) {
    if (safe) {
      memset(data, 0, capacity);
    }
    delete[] data;
  }
}

//
// INPUT FUNCTIONS
//

//
// Read data in from a file descriptor
//
ssize_t Fdbuf::read(int fd, uint32_t max_read)
{
  ensure_additional(max_read);
  return post_read(::read(fd, data + start + len, max_read));
}
ssize_t Fdbuf::recv(int fd, uint32_t max_read, int flags)
{
  ensure_additional(max_read);
  return post_read(::recv(fd, data + start + len, max_read, flags));
}
ssize_t Fdbuf::recvfrom(int sd, uint32_t max_read, int flags,
			struct sockaddr *from, socklen_t *fromlen)
{
  ensure_additional(max_read);
  ssize_t result = ::recvfrom(sd, data + start + len, max_read,
                              0, from, fromlen);
  return post_read(result);
}

// Appending a string
Fdbuf &Fdbuf::pushString(const std::string &s)
{
  ensure_additional(s.length());
  s.copy(data + start + len, s.length(), 0);
  len += s.length();
  return *this;
}

Fdbuf &Fdbuf::pushString(const char *str)
{
  return push_bytes(str, strlen(str));
}

Fdbuf &Fdbuf::pushFdbuf(const Fdbuf &fb, uint32_t max_size)
{
  return push_bytes(fb.data + fb.start, std::min(fb.len, max_size));
}

//
// OUTPUT FUNCTIONS
//

//
// Write data
//
ssize_t
Fdbuf::write(int fd, ssize_t max_write)
{
  return post_write(::write(fd,
                            data + start, 
			    (max_write < 0) ? len
                            : std::min(len, (uint32_t) max_write)));
}

ssize_t Fdbuf::send(int sd, ssize_t max_write, int flags)
{
  return post_write(::send(sd, data + start, 
			   (max_write < 0) ? len
                           : std::min(len, (uint32_t) max_write),
			   flags));
}
ssize_t Fdbuf::sendto(int sd, ssize_t max_write, int flags,
		      const struct sockaddr *to, socklen_t tolen)
{
  return post_write(::sendto(sd, data + start, 
			     ((max_write < 0) ? len
                              : std::min(len, (uint32_t) max_write)),
			     flags, to, tolen));
}

//
// Extracting values
//
u_int32_t
Fdbuf::pop_uint32()
{
  u_int32_t v = 0;
  if (len >= sizeof(u_int32_t)) {
    v = *((u_int32_t *)(data + start));
    len -= sizeof(u_int32_t);
    start += sizeof(u_int32_t);
  }
  return v;
}


Fdbuf&
Fdbuf::push_uint32(const u_int32_t l)
{
  ensure_additional(sizeof(u_int32_t));
  *((u_int32_t *)(data + start + len)) = l;
  len += sizeof(u_int32_t);
  return *this;
}


bool Fdbuf::pop_bytes(char *buf, uint32_t sz)
{
  if (len >= sz) {
    memcpy(buf, data + start, sz);
    len -= sz;
    start += sz;
    return true;
  } else {
    return false;
  }
}


Fdbuf&
Fdbuf::push_bytes(const char *buf, uint32_t sz)
{
  ensure_additional(sz);
  memcpy(data + start + len, buf, sz);
  len += sz;
  return *this;
}


uint32_t
Fdbuf::pop_to_fdbuf(Fdbuf &fb, uint32_t to_write)
{
  to_write = std::min(to_write, len);
  fb.pushFdbuf(*this, to_write);
  len -= to_write;
  start += to_write;
  return to_write;
}

//
// Make sure the buffer is big enough
//
void Fdbuf::ensure(uint32_t new_capacity)
{
  // Always give us aligned headroom.
  new_capacity = align(new_capacity);
  if (capacity < new_capacity) {
    uint32_t new_size = (new_capacity + BUF_INCREMENT - 1) / BUF_INCREMENT * BUF_INCREMENT;
    char *old_buf = data;
    char *new_buf = new char[new_size];
    memcpy(new_buf, old_buf + start, len);
    if (safe) {
      memset(new_buf + len, 0, new_size -len);
      memset(old_buf, 0, capacity);
    }
    data = new_buf;
    capacity = new_size;
    start = 0;
    delete[] old_buf;
  } 
  if (capacity - start < new_capacity) {
    memcpy(data, data + start, len);
  }
  assert(capacity >= start + len);
}


/*
 * End of file
 */
