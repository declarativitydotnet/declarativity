// -*- c-basic-offset: 2; related-file-name: "hooker.C" -*-
#ifndef __HOOKER_H__
#define __HOOKER_H__
#include <task.h>
#include <inlines.h>

/** A hook function invoked by tasks */
typedef bool (*TaskHook)(Task *, void *);

class Hooker : public Task {
 public:
  /** Create a hooker given its hook and data. */
  REMOVABLE_INLINE Hooker(TaskHook, void *);

  // What's my hook?
  TaskHook hook() const			{ return _hook; }

  // Call the hook
  REMOVABLE_INLINE virtual void run();



 private:
  /** My hook */
  TaskHook _hook;

  /** My opaque data, for use whenever calling the hook */
  void * _opaque;
};

#endif
