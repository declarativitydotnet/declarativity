// -*- c-basic-offset: 2; related-file-name: "master.C" -*-
#ifndef __MASTER_H__
#define __MASTER_H__
#include <inlines.h>
#include <refcnt.h>

class Master {
 public:

  Master();
  ~Master();

  /** Start up the master */
  void run();
  
 private:
};

// Handy dandy shorthand refcounted types
typedef ref< Master > MasterRef;
typedef ptr< Master > MasterPtr;

#endif
