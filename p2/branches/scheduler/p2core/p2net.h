// -*- c-basic-offset: 2; related-file-name: "p2net.C" -*-
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
 * DESCRIPTION: P2 time utilities
 *
 */

#ifndef __P2NET_H__
#define __P2NET_H__

#include <boost/function.hpp>
#include <arpa/inet.h>

class P2Net {

public:

  // 
  // Callback for TCP connections.
  //
  typedef boost::function<void (int)> ConnCB;

  //
  // Create a TCP connection, and call cb with the file descriptor (or
  // -1 if it fails somehow).   Secretly this calls the event loop. 
  //
  static void tcpConnect( in_addr_t addr, u_int16_t port, ConnCB cb);
  static void tcpConnect(ConnCB cb);

  //
  // Return a bound, but not connected, socket which has been put into
  // non-blocking mode. 
  // 
  static int inetSocket( int type, in_addr_t addr, u_int16_t port);


  ///////////////////////////////////////////////////////////
  // Low-level operations - think whether you need these individually
  // before calling them, since chances are the above do what you
  // want.  All these functions below return the socket descriptor s
  // if successful, or a negative value if they fail (and close the
  // socket in the process). 
  ///////////////////////////////////////////////////////////

  // Set TCP nodelay
  static int tcp_nodelay(int s);

  // Bind a socket
  static int ip_bind(int s, in_addr_t addr, u_int16_t port);
  
  // Set keepalives on a socket
  static int sock_keepalive(int s);
  
  // Allow address reuse on a socket
  static int sock_reuse(int s);

  // Put a socket into non-blocking mode
  static int fd_nonblock(int s);

  // Close the file descriptor on exec
  static int fd_close_on_exec(int s);

private:
  
  // Internal callback for sockets...
  static void connect_cb( int s, ConnCB cb);

};

#endif // P2NET
