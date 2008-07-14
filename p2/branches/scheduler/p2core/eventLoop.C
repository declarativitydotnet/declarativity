// -*- c-basic-offset: 2; related-file-name: "loop.h" -*-
/*
 * @(#)$Id: loop.C 1262 2007-07-27 02:26:23Z maniatis $
 *
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 *
 * DESCRIPTION: This is the outer event loop for P2
 */

#include "eventLoop.h"

#include <sys/select.h>

/////////////////////////////////////////////////////////////////
//
// Constructor 
//
/////////////////////////////////////////////////////////////////

EventLoop::EventLoop()
{
}

/////////////////////////////////////////////////////////////////
//
// File descriptor (actually, socket descriptor) callbacks. 
//
/////////////////////////////////////////////////////////////////

//
// Helper functions to add and remove callbacks from a list.  Fairly
// standard iterator patterns. 
//
void EventLoop::add_fcb(int fd, 
			const FileCB &cb, 
			std::list<FCB> &l)
{
  for( std::list<FCB>::iterator i = l.begin(); i != l.end(); i++) {
    if (i->fd == fd) {
      i->fn = cb;
      return;
    }
  }
  l.push_back(FCB(fd, cb));
}
void EventLoop::del_fcb(int fd, std::list<FCB> &l)
{
  for( std::list<FCB>::iterator i = l.begin(); i != l.end(); i++) {
    if (i->fd == fd) {
      i = l.erase(i);
      return;
    }
  }
}  

//
// The actual methods to add and remove file callbacks from the lists
// 
void EventLoop::add_read_fcb(int f, const FileCB &cb)
{
  add_fcb(f, cb, read_fcbs);
}
void EventLoop::add_write_fcb(int f, const FileCB &cb)
{
  add_fcb(f, cb, write_fcbs);
}
void EventLoop::add_error_fcb(int f, const FileCB &cb)
{
  add_fcb(f, cb, error_fcbs);
}
void EventLoop::del_read_fcb(int f)
{
  del_fcb(f, read_fcbs);
}
 void EventLoop::del_write_fcb(int f)
{
  del_fcb(f, write_fcbs);
}
void EventLoop::del_error_fcb(int f)
{
  del_fcb(f, error_fcbs);
}

/////////////////////////////////////////////////////////////////
//
// Timer-based callbacks
//
/////////////////////////////////////////////////////////////////

EventLoop::TimerID EventLoop::enqueue_timer(const boost::posix_time::ptime &t,
					    const EventLoop::TimerCB &f)
{
  // WTF?!  Not too bad really: we construct a TCB from the time and
  // callback.  This will assign a new ID (see the constructor for
  // EventLoop::TCB), which we want to return.  "insert" returns a
  // pair, whose first value is an iterator pointing to the newly
  // inserted TCB, so we just extract its id. 
  return tcbs.insert(TCB(t, f)).first->id;
}

EventLoop::TimerID EventLoop::enqueue_timer(const double seconds,
					    const EventLoop::TimerCB &f)
{
  boost::posix_time::ptime t = 
    boost::posix_time::microsec_clock::universal_time() 
    + boost::posix_time::nanoseconds(long(seconds * 1e9));
  return EventLoop::enqueue_timer(t, f);
}

void EventLoop::cancel_timer(const TimerID id)
{
  for(std::set<TCB,TCB::Cmp>::iterator i=tcbs.begin(); i != tcbs.end(); i++) {
    if (i->id == id) {
      tcbs.erase(i);
      return;
    }
  }
}


/////////////////////////////////////////////////////////////////
//
// Main loop
//
/////////////////////////////////////////////////////////////////

//
// Timespec manipulations
//
inline void set_timespec( struct timespec &ts, 
			  const boost::posix_time::time_duration &pt)
{
  static const long ns_fac 
    = 1000000000L / boost::posix_time::time_duration::ticks_per_second();

  ts.tv_sec = pt.total_seconds();
  ts.tv_nsec = pt.fractional_seconds() / ns_fac;
}
inline void set_timespec( struct timespec &ts, long secs, long nsecs)
{
  ts.tv_sec = secs;
  ts.tv_nsec = nsecs;
}


void EventLoop::outer_loop()
{
  for(;;) {
    boost::posix_time::ptime now =
      boost::posix_time::microsec_clock::universal_time();
    struct timespec ts;

    // Call all the timer callbacks, and remove them from the timer
    // heap.
    for( std::set<TCB, TCB::Cmp>::iterator i = tcbs.begin(); 
	 i != tcbs.end() && (i->t < now);
	 i++) {
      i->fn();
      tcbs.erase(i);
    }

    // If the event queue is empty, set our timeout to be the
    // closest timer event.  Otherwise, it's zero. 
    if (ecbs.empty()) {
      set_timespec(ts, 0, 0);
    } else if (tcbs.empty()) {
      // Block for 10 seconds, rather than forever...
      set_timespec(ts, 10, 0);
    } else {
      set_timespec(ts, tcbs.begin()->t - now);
    }

    // Do the select and process all the file descriptor callbacks.
    process_pselect(ts);

    // Also give the DPCs a lookin
    process_dpcs();

    // Pull an event off the queue
    if (!ecbs.empty()) {
      inner_loop( ecbs.front() );
      ecbs.pop_front();
    }
  }
}

// 
// And now the inner event loop. See the header file for a detailed
// description of what's happening here, and why.
//
void EventLoop::inner_loop( Closure &e )
{
  // Invoke the outer event...
  e();
  process_dpcs();

  // Now deal with all the repercussions.  First run the event
  // computations to completion..
  while( !inner_ecbs.empty() ) {
    
    // Run an inner event...
    inner_ecbs.front()();
    inner_ecbs.pop_front();
    process_dpcs();
  }

  // Now run all the actions. 
  while( !actions.empty() ) {
    actions.front()();
    actions.pop_front();
  }
  // And we're done for that outer event.
}


//
// Run all the DPCs
//
void EventLoop::process_dpcs()
{
  while( !dpcs.empty() ) {
    dpcs.front()();
    dpcs.pop_front();
  }
}

//
// Chunk of the outer event loop that processes file descriptors.  The
// timeout required for pselect is given as the argument.
// 
void EventLoop::process_pselect(const struct timespec &ts)
{
  fd_set read_fds;
  fd_set write_fds;
  fd_set error_fds;
  
  int max_fd = 0;
  FD_ZERO(&read_fds);
  for( std::list<FCB>::iterator i = read_fcbs.begin(); i != read_fcbs.end(); i++) {
    FD_SET(i->fd, &read_fds);
    max_fd = std::max(i->fd, max_fd);
  }
  FD_ZERO(&write_fds);
  for( std::list<FCB>::iterator i = write_fcbs.begin(); i != write_fcbs.end(); i++) {
    FD_SET(i->fd, &write_fds);
    max_fd = std::max(i->fd, max_fd);
  }
  FD_ZERO(&error_fds);
  for( std::list<FCB>::iterator i = error_fcbs.begin(); i != error_fcbs.end(); i++) {
    FD_SET(i->fd, &error_fds);
    max_fd = std::max(i->fd, max_fd);
  }
  
  // Actually do the pselect. 
  int rc = pselect(max_fd+1, &read_fds, &write_fds, &error_fds, &ts, NULL);
  
  // Errors?
  if (rc == -1) {
    // Log the error....
    return;
  } else if (rc == 0) {
    // Nothing happend...this is pretty normal.
    return;
  }
  
  // Process all the active file descriptor callbacks.
  for(std::list<FCB>::iterator i = read_fcbs.begin(); i != read_fcbs.end(); i++) {
    if (FD_ISSET(i->fd, &read_fds)) {
      i->fn();
    }
  }
  for(std::list<FCB>::iterator i = write_fcbs.begin(); i != write_fcbs.end(); i++) {
    if (FD_ISSET(i->fd, &write_fds)) {
      i->fn();
    }
  }
  for(std::list<FCB>::iterator i = error_fcbs.begin(); i != error_fcbs.end(); i++) {
    if (FD_ISSET(i->fd, &error_fds)) {
      i->fn();
    }
  }
  // And we're done..
}

// Enqueue an event.  The event can't be dequeued. 
void EventLoop::enqueue_event(const EventLoop::Closure &e) 
{
  ecbs.push_back(e);
}

/////////////////////////////////////////////////////////
// Inner computations (in P2 these correspond to events and DPCs
// inside a single fixed-point computation. 
/////////////////////////////////////////////////////////

// Deferred procedure calls within a fixed-point rule evaluation
void EventLoop::enqueue_dpc(const EventLoop::Closure &e)
{
  dpcs.push_back(e);
}

// Overlog-visible events within a fixed-point computation
void EventLoop::enqueue_inner_event(const EventLoop::Closure &e)
{
  inner_ecbs.push_back(e);
}

// Actions to be executed after the computation is done.
void EventLoop::enqueue_action(const EventLoop::Closure &e)
{
  actions.push_back(e);
}
