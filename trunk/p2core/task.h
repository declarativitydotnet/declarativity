// -*- c-basic-offset: 2; related-file-name: "task.C" -*-
#ifndef __TASK_H__
#define __TASK_H__
#include <inlines.h>
#include <assert.h>
#include <async.h>              /* For refs */

#define PASS_GT(a, b)	((int)(a - b) > 0)

class Task;

class Router;
typedef ref< Router > RouterRef;

class RouterThread;
class TaskList;
class Master;
class Router;

class Task {
 public:
  /** Create a task shell */
  REMOVABLE_INLINE Task();

  ~Task();

  bool initialized() const		{ return _router != 0; }

  bool scheduled() const		{ return _prev != 0; }

  Task *scheduled_next() const		{ return _next; }

  Task *scheduled_prev() const		{ return _prev; }

  RouterThread *scheduled_list() const	{ return _thread; }

  Master *master() const;
 
  void initialize(Router *, bool);
  
  void cleanup();

  void unschedule();
  
  REMOVABLE_INLINE void reschedule();

  REMOVABLE_INLINE int fast_unschedule();
  
  REMOVABLE_INLINE void fast_reschedule();

  void strong_unschedule();
  
  void strong_reschedule();

  /** Call whatever it is this task is doing.  Instances of this class
      should never be run.  They are only used as sentinel elements of
      task lists. */
  REMOVABLE_INLINE virtual void run()	 { assert(false); };

 private:

  /* if gcc keeps this ordering, we may get some cache locality on a 16
   * or 32 byte cache line: the first three fields are used in list
   * traversal */

  /** My previous task in the task list */
  Task *_prev;

  /** My next task in the task list */
  Task *_next;

  // The thread I'm running on
  RouterThread *_thread;

  // The router I'm running on
  Router * _router;

  enum { RESCHEDULE = 1 };
  
  unsigned _pending;
  
  Task *_pending_next;

  Task(const Task &);
  
  Task &operator=(const Task &);

  void add_pending(int);
  
  void process_pending(RouterThread *);
  
  /** Schedule a task assuming the task queue lock held. */
  REMOVABLE_INLINE void fast_schedule();

  /** Reschedules the task into its task queue.  If it cannot lock the
      task list, it schedules a continuation of itself for later, when
      the task list is unlocked. */
  void true_reschedule();
  
  REMOVABLE_INLINE void lock_tasks();
  
  REMOVABLE_INLINE bool attempt_lock_tasks();

  void make_list();
  
  static bool error_hook(Task *, void *);
  
  friend class RouterThread;
  friend class Master;
};

#endif
