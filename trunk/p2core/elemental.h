// -*- c-basic-offset: 2; related-file-name: "elemental.C" -*-
#ifndef __ELEMENTAL_H__
#define __ELEMENTAL_H__
#include <task.h>
#include <inlines.h>
#include <element.h>

class Elemental : public Task {
 public:
  /** Create a task for a specific element */
  REMOVABLE_INLINE Elemental(ElementRef element);
  ~Elemental();

  ElementRef element() const		{ return _element; }

  void initialize(Element *, bool scheduled);

  // Call the elemental task
  REMOVABLE_INLINE virtual void run();

 private:

  // My associated element
  ElementRef _element;
};

#endif
