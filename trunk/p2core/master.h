// -*- c-basic-offset: 2; related-file-name: "master.C" -*-
#ifndef __MASTER_H__
#define __MASTER_H__
#include <click/vector.hh>
#include <timer.h>
#include <task.h>
#include <sync.h>
#include <inlines.h>

#include <unistd.h>
#if HAVE_POLL_H
# include <poll.h>
#endif

class Element;

class Master {
 public:

  Master(int nthreads);
  ~Master();
  
  void use();
  void unuse();

  int nthreads() const				{ return _threads.size() - 1; }

  /** Return the router thread by that index. Currently, there is only
      one thread */
  REMOVABLE_INLINE RouterThread *thread(int id) const;

  const volatile int *runcount_ptr() const	{ return &_runcount; }
  
  int timer_delay(struct timeval *);
  void run_timers();
  
  void remove_router(Router *);
  

 private:
  
  Spinlock _master_lock;
  volatile int _master_paused;
  Router *_routers;
  int _refcount;
  
  Vector<RouterThread *> _threads;
  
  Spinlock _runcount_lock;
  volatile int _runcount;
  
  Task _task_list;
  SpinlockIRQ _task_lock;
  
  Timer _timer_list;
  Spinlock _timer_lock;
  
  Master(const Master &);
  Master &operator=(const Master &);
  
  void register_router(Router *);
  void run_router(Router *, bool foreground);
  
  void remove_pollfd(int);
  
  bool check_driver();
  void process_pending(RouterThread *);
  
  friend class Task;
  friend class Timer;
  friend class RouterThread;
  friend class Router;
};

#endif
