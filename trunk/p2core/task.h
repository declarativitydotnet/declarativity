// -*- c-basic-offset: 2; related-file-name: "task.C" -*-
#ifndef __TASK_H__
#define __TASK_H__
#include <element.h>
#include <inlines.h>

#define PASS_GT(a, b)	((int)(a - b) > 0)

class Task;
typedef bool (*TaskHook)(Task *, void *);
class RouterThread;
class TaskList;
class Master;
class Router;

class Task {
 public:

  REMOVABLE_INLINE Task(TaskHook, void *);
  REMOVABLE_INLINE Task(Element *);		// call element->run_task()
  ~Task();

  bool initialized() const		{ return _router != 0; }
  bool scheduled() const		{ return _prev != 0; }

  TaskHook hook() const			{ return _hook; }
  void *thunk() const			{ return _thunk; }
  REMOVABLE_INLINE Element *element() const;

  Task *scheduled_next() const	{ return _next; }
  Task *scheduled_prev() const	{ return _prev; }
  RouterThread *scheduled_list() const { return _thread; }
  Master *master() const;
 
  void initialize(Router *, bool scheduled);
  void initialize(Element *, bool scheduled);
  void cleanup();

  void unschedule();
  REMOVABLE_INLINE void reschedule();

  REMOVABLE_INLINE int fast_unschedule();
  REMOVABLE_INLINE void fast_reschedule();

  void strong_unschedule();
  void strong_reschedule();

  int thread_preference() const	{ return _thread_preference; }

  REMOVABLE_INLINE void call_hook();



 private:

  /* if gcc keeps this ordering, we may get some cache locality on a 16 or 32
   * byte cache line: the first three fields are used in list traversal */

  Task *_prev;
  Task *_next;
  
  TaskHook _hook;
  void *_thunk;
  
  RouterThread *_thread;
  int _thread_preference;
  
  Router *_router;

  enum { RESCHEDULE = 1 };
  unsigned _pending;
  Task *_pending_next;

  Task(const Task &);
  Task &operator=(const Task &);

  void add_pending(int);
  void process_pending(RouterThread *);
  REMOVABLE_INLINE void fast_schedule();
  void true_reschedule();
  REMOVABLE_INLINE void lock_tasks();
  REMOVABLE_INLINE bool attempt_lock_tasks();

  void make_list();
  static bool error_hook(Task *, void *);
  
  friend class RouterThread;
  friend class Master;
  
};

#endif
