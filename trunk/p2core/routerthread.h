// -*- c-basic-offset: 2; related-file-name: "routerthread.C" -*-
#ifndef __ROUTERTHREAD_H__
#define __ROUTERTHREAD_H__
#include <inlines.h>
#include <sync.h>
#include <vector.h>

#define CLICK_DEBUG_SCHEDULING 0

// NB: user must #include <click/task.hh> before <click/routerthread.hh>.
// We cannot #include <click/task.hh> ourselves because of circular #include
// dependency.

class RouterThread : public Task {
 public:
  
  int thread_id() const			{ return _id; }
  Master *master() const		{ return _master; }

  void driver();
  void driver_once();

  // Task list functions
  bool empty() const;

  void lock_tasks();
  bool attempt_lock_tasks();
  void unlock_tasks();

  void unschedule_all_tasks();

  REMOVABLE_INLINE void unsleep();

#if CLICK_DEBUG_SCHEDULING
  enum { S_RUNNING, S_PAUSED, S_TIMER, S_BLOCKED };
  int thread_state() const		{ return _thread_state; }
  static String thread_state_name(int);
  uint32_t driver_epoch() const		{ return _driver_epoch; }
  uint32_t driver_task_epoch() const	{ return _driver_task_epoch; }
  timeval task_epoch_time(uint32_t epoch) const;
#endif

  unsigned _tasks_per_iter;
  unsigned _iters_per_timers;
  unsigned _iters_per_os;

 private:
    
  Master *_master;
  int _id;

  Spinlock _lock;
  atomic_uint32_t _task_lock_waiting;
  atomic_uint32_t _pending;

#if CLICK_DEBUG_SCHEDULING
  int _thread_state;
  uint32_t _driver_epoch;
  uint32_t _driver_task_epoch;
  enum { TASK_EPOCH_BUFSIZ = 32 };
  uint32_t _task_epoch_first;
  timeval _task_epoch_time[TASK_EPOCH_BUFSIZ];
#endif
    
  // called by Master
  RouterThread(Master *, int);
  ~RouterThread();

  // task requests
  REMOVABLE_INLINE void add_pending();

  // task running functions
  REMOVABLE_INLINE void nice_lock_tasks();
  REMOVABLE_INLINE void run_tasks(int ntasks);
  REMOVABLE_INLINE void run_os();
    
  friend class Task;
  friend class Master;
};
#endif
