// -*- c-basic-offset: 2; related-file-name: "loop.C" -*-
/*
 * @(#)$Id: loop.h 1262 2007-07-27 02:26:23Z maniatis $
 *
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 * DESCRIPTION: P2 event scheduler and dispatcher.
 *
 * LONGER DESCRIPTION: Please read this if you are likely to call
 *   event loop functionality, and *particularly* if you are thinking
 *   of modifying this code!
 * 
 * The goals of the new event loop are:
 *  1. Conciseness and clarity of structure
 *  2. Efficiency
 *  3. Ease of use
 *  4. Lack of dependency on other bits of P2 (such as Elements)
 *  5. Support for P2's fixed-point computational semantics
 *  6. Pave the way to multiple concurrent fixed-points
 * It is these last two goals which mainly distinguish this from a
 * canonical well-engineered select loop, such as that found in
 * libasync.  It is also the reason for having several queues for
 * events.  The previous version in P2 (loop.[Ch]) seemed lacking in
 * {1,3,4,5,6}.  
 * 
 * The best way to understand how the new loop works is to consider
 * first the outer loop, then the inner one.  
 *
 * THE OUTER LOOP
 * 
 * The outer loop processes file descriptors (sockets, in effect),
 * timers, and "events".  All these are arbitrary closures, but the
 * intention is that "events" correspond to a P2 event tuple which
 * initiates a new fixed-point computation.   
 * 
 * - File descriptor callbacks are installed and removed for read,
 *   write, or error conditions.  An event on a file descriptor does
 *   *not* deregister the file descriptor - they hang around until you
 *   explicitly remove them.  They are installed by, for example:
 *
 *   theLoop.add_read_fcb( my_fd, boost::bind(&MyClass::fd_event, this));
 *
 * - Timer callbacks are installed with an absolute time in the future
 *   at which they will fire.  Installation returns a handle which can
 *   be used to cancel the timer before it fires.  Once the
 *   timer fires it is removed.
 *
 * - Events are intended to correspond to tuples which initiate a P2
 *   fixed-point computation.  However, like everything else that the
 *   event loop deals with, they are purely closures. The intention is
 *   that the details of events (and actions, see below) are hidden
 *   from the event loop by boost::bind() invocations.  Events cannot
 *   be cancelled once enqueued.  They are removed once they have been
 *   handled. 
 * 
 * The outer loop processes all outstanding file descriptors, then all
 * expired timeouts, then a single event from the event queue, before
 * repeating.  The event itself is processed by handing it to the
 * inner loop.  The outer loop does not terminate.
 *
 * THE INNER LOOP
 * 
 * The inner loop starts by calling the single event which initiated
 * it, once and once only.  The inner loop then repeatedly invokes all
 * closures on the DPC queue, followed by a single event at the head
 * of the inner event queue, until both are empty.  It then invokes
 * all enqueue actions. 
 * 
 * - The DPC queue is for callbacks in the middle of a single
 *   computation.  For example, blocking and unblock dataflows is
 *   typically performed by queueing DPCs which restart elements. 
 * 
 * - The inner event queue is for intermediate tuples which result
 *   in the middle of a fixed-point computation.
 * 
 * - Actions are operations such as table insertions and deletions,
 *   plus potentially enqueuing further outer events. 
 * 
 * CONCURRENCY
 *
 * The outer loop is a classic single-threaded event loop.  The inner
 * loop is also implicitly single threaded, but the reason for
 * splitting them up at this stage is that In The Future(tm) we could
 * have multiple inner loops running concurrently, as long as we could
 * establish (by queue analysis on the outer queue) that they did not
 * have any data dependencies, and that actions could be applied
 * atomically with respect to each other.  
 * 
 * INITIALIZATION (or, what happended to IRunnable?)
 * 
 * The previous loop used the concept of "active" elements which were
 * (sometimes) runnable, in that they would be called regardless of
 * the state of the dataflow.  The new loop does not support such a
 * concept - an element is implicitly "runnable" IFF there is a
 * callback queued somewhere which will directly or indirectly invoke
 * it.  
 * 
 * This has the benefit of centralizing execution in the event loop /
 * scheduler, but leads to an initialization problem with elements
 * like pullPush which must be notified of available tuples in order
 * to start up, but at start-of-day have had no chance to install a
 * callback with their upstream element(s) to be activated.  
 * 
 * Instead, under the new regime such elements install a DPC at
 * initialization time, which will typically point to their "pull
 * unblock" handler, or somesuch.  This will ensure the initial
 * callback is then installed (even if there are no tuples at that
 * stage).  It might be regarded as an additional initialization
 * stage, which compensates for the fact that unblock callbacks are
 * dynamically bound, but that's a wider design question...
 */

#ifndef __EVENTLOOP_H__
#define __EVENTLOOP_H__

#include <list>
#include <set>
#include <boost/function.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

////////////////////////////////////////////////////////////
// Outer event loop class
////////////////////////////////////////////////////////////



class EventLoop {
  
public:

  EventLoop();

  /////////////////////////////////////////////////////////
  // File descriptor-based callbacks
  /////////////////////////////////////////////////////////

  // What it looks like. 
  typedef boost::function<void (void)>        FileCB;

  // Add a file callback.  Only one callback for a particular fd 
  // pair can be installed at a time for each operation. 
  void add_read_fcb(int f, const FileCB &cb); 
  void add_write_fcb(int f, const FileCB &cb); 
  void add_error_fcb(int f, const FileCB &cb); 

  // Remove a callback.  Anything matching this (op,fd) pair will be
  // removed. 
  void del_read_fcb(int f); 
  void del_write_fcb(int f); 
  void del_error_fcb(int f);

  /////////////////////////////////////////////////////////
  // Timer-based callbacks
  /////////////////////////////////////////////////////////

  // What it looks like
  typedef boost::function<void (void)>	TimerCB;
  typedef long long TimerID;

  // Add a timer callback.  The ID of the timer will be returned. 
  TimerID enqueue_timer(const boost::posix_time::ptime &t, const TimerCB &f );
  TimerID enqueue_timer(const double seconds, const TimerCB &f );

  // Remove (cancel) a timer callback. 
  void cancel_timer(const TimerID id);

  /////////////////////////////////////////////////////////
  // Outer events (in P2 these correspond to distinct fixed-point
  // computations. 
  /////////////////////////////////////////////////////////

  // What an event looks like
  typedef boost::function<void (void)>	Closure;
  
  // Enqueue an event.  The event can't be dequeued. 
  void enqueue_event(const Closure &e);

  /////////////////////////////////////////////////////////
  // Inner computations (in P2 these correspond to events and DPCs
  // inside a single fixed-point computation. 
  /////////////////////////////////////////////////////////

  // Deferred procedure calls within a fixed-point rule evaluation
  void enqueue_dpc(const Closure &e);

  // Overlog-visible events within a fixed-point computation
  void enqueue_inner_event(const Closure &e);

  // Actions to be executed after the computation is done.
  void enqueue_action(const Closure &e);

  /////////////////////////////////////////////////////////
  // Singleton for the event loop itself.
  /////////////////////////////////////////////////////////

  static EventLoop *loop() {
    static EventLoop lp;
    return &lp;
  }

  /////////////////////////////////////////////////////////
  // Kick things off: this function does not return.
  /////////////////////////////////////////////////////////
  void start() { outer_loop(); };

private:

  // File descriptor callbacks
  struct FCB {
    FCB( int s, FileCB f ) : fd(s), fn(f) {};

    int	fd;
    FileCB fn;
  };
  
  // File descriptors to watch
  std::list<FCB> read_fcbs;
  std::list<FCB> write_fcbs;
  std::list<FCB> error_fcbs;

  static void add_fcb(int fd, const FileCB &cb, std::list<EventLoop::FCB> &l);
  static void del_fcb(int fd, std::list<EventLoop::FCB> &l);

  // Timer queue
  struct TCB {

    boost::posix_time::ptime	t;
    TimerCB			fn;
    TimerID			id;

    TCB(const boost::posix_time::ptime &tm, const TimerCB f) : t(tm), fn(f) {
      static long long nextId = 1;
      id = nextId++;
    };

    struct Cmp {
      bool operator()(const TCB &t1, const TCB &t2) const {
	return (t1.t == t2.t) ? (t1.id < t2.id) : (t1.t < t2.t); 
      };
    };
  };
  std::set<TCB, TCB::Cmp> tcbs;
  
  // Closure queues
  std::list<Closure> ecbs;
  std::list<Closure> inner_ecbs;
  std::list<Closure> actions;
  std::list<Closure> dpcs;

  void process_pselect(const struct timespec &ts);
  void process_dpcs();
  void outer_loop();
  void inner_loop( Closure &e );


};


#endif /* __EVENTLOOP_H_ */
