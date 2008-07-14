// -*- c-basic-offset: 2; related-file-name: "p2net.h" -*-
/*
 * @(#)$Id: p2Time.h 1243 2007-07-16 19:05:00Z maniatis $
 *
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 * DESCRIPTION: P2 network utilities
 *
 */ 


#include "p2net.h"

#include "eventLoop.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <boost/bind.hpp>

//
// Establish TCP connection.  ConnCB will be called with the file
// descriptor of the connected socket, or -1 if it fails.
//
void P2Net::tcpConnect( in_addr_t addr, u_int16_t port, ConnCB cb)
{
  int s;

  // Give us a socket
  if ((s=inetSocket( SOCK_STREAM, INADDR_ANY, 0 )) < 0 ) {
    EventLoop::loop()->enqueue_dpc(boost::bind(cb, -1));
    return;
  } 

  /* Connect request */
  struct sockaddr_in sa;
  memset(&sa, 0, sizeof(sa));
  sa.sin_family = PF_INET;
  sa.sin_port = htons(port);
  sa.sin_addr.s_addr = htonl(addr);
  if ( connect (s, (struct sockaddr *)&sa, sizeof(sa)) < 0 ) {
    perror("connect");
    close(s);
    EventLoop::loop()->enqueue_dpc(boost::bind(cb, -1));
  }

  // Install file descriptor callbacks
  EventLoop::loop()->add_write_fcb(s, boost::bind(&P2Net::connect_cb, s, cb));
  EventLoop::loop()->add_error_fcb(s, boost::bind(&P2Net::connect_cb, s, cb));
}

//
// (Potentially) successful connection from the event loop...
//
void P2Net::connect_cb( int s, ConnCB cb)
{
  unsigned int n; 
  socklen_t l = sizeof(n);

  EventLoop::loop()->del_write_fcb(s);
  EventLoop::loop()->del_error_fcb(s);

  if (getsockopt(s, SOL_SOCKET, SO_ERROR, &n, &l) < 0 || n != 0) { 
    perror("connecting socket");
    close(s);
    cb(-1);
    return;
  }
  // Success!
  cb(s);
}


int P2Net::inetSocket( int type, in_addr_t addr, u_int16_t port)
{
  int s;
  
  if ((s = socket(PF_INET, type, 0)) < 0 
      || ip_bind(s, addr, port) < 0 
      || sock_reuse(s) < 0 
      || fd_nonblock(s) < 0 ) 
    {
      return -1;
    } 
  return s;
}




      
int P2Net::ip_bind(int s, in_addr_t addr, u_int16_t port)
{
  /* Bind to our requested port */
  struct sockaddr_in sa;
  memset(&sa, 0, sizeof(sa));
  sa.sin_family = PF_INET;
  sa.sin_port = htons(port);
  sa.sin_addr.s_addr = htonl(addr);
  if (bind (s, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
    perror("binding to local address");
    close(s);
    return -1;
  }
  return s;
}
  
int P2Net::tcp_nodelay(int s)
{
  int n = 1;
  if (setsockopt(s, IPPROTO_TCP, TCP_NODELAY, (char *)&n, sizeof(n)) < 0) {
    perror("setsockopt for TCP nodelay");
    close(s);
    return -1;
  }
  return s;
}

int P2Net::sock_reuse(int s)
{  
  /* Socket magic: to allow us to re-run quickly */
  int n = 1;
  if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *)&n, sizeof(n)) < 0) {
    perror("setsockopt for reuse");
    close(s);
    return -1;
  }
  return s;
}

int P2Net::sock_keepalive(int s)
{  
  int n = 1;
  if (setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, (char *)&n, sizeof(n)) < 0) {
    perror("setsockopt for keepalive");
    close(s);
    return -1;
  }
  return s;
}

int P2Net::fd_nonblock(int s)
{
  int value;

  if ((value = fcntl(s, F_GETFL, 0)) < 0) {
    perror("fcntl for nonblock");
    close(s);
    return value;
  }
  value |= O_NONBLOCK;
  if (fcntl(s, F_SETFL, value) < 0) {
    perror("fcntl for nonblock");
    close(s);
    return value;
  }
  return s;
}

int P2Net::fd_close_on_exec(int s)
{
  int value;

  if ((value = fcntl(s, F_GETFD, 0)) < 0) {
    perror("fcntl for close on exec");
    close(s);
    return value;
  }
  value |= FD_CLOEXEC;
  if (fcntl(s, F_SETFD, value) < 0) {
    perror("fcntl for close-on-exec");
    close(s);
    return value;
  }
  return s;
}

