/*
 * @(#)$Id$
 *
 * Copyright (c) 2004 Intel Corporation. All rights reserved.
 *
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 * DESCRIPTION: Tests for simple element plumbing
 *
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */
#include <async.h>
#include <arpc.h>
#include <iostream>

#include "tuple.h"
#include "pullprint.h"
#include "memoryPull.h"
#include "router.h"
#include "master.h"

TupleRef create_tuple(int i) {
  TupleRef t = New refcounted< Tuple >;
  t->append(*New TupleField());
  t->append(*New TupleField((int32_t)i));
  t->append(*New TupleField((uint64_t)i));
  t->append(*New TupleField(i));
  strbuf myStringBuf;
  myStringBuf << "This is string '" << i << "'";
  str myString = myStringBuf;
  t->append(*New TupleField(myString));
  t->freeze();
  return t;
}

Router::ConfigurationRef createConfiguration(ref< vec< TupleRef > > buffer)
{
  // Create the elements
  ref< PullPrint > pullPrint = New refcounted< PullPrint >();
  ref< MemoryPull > memoryPull = New refcounted< MemoryPull >(buffer, 1);
  ref< vec< ElementRef > > elements = New refcounted< vec< ElementRef > >();
  elements->push_back(pullPrint);
  elements->push_back(memoryPull);

  // Create the hookups
  Router::HookupRef hookup =
    New refcounted< Router::Hookup >(memoryPull, 0,
                                     pullPrint, 0);
  ref < vec< Router::HookupRef > > hookups =
    New refcounted< vec< Router::HookupRef > >();
  hookups->push_back(hookup);

  // Create the configuration
  Router::ConfigurationRef configuration =
    New refcounted< Router::Configuration >(elements, hookups);

  return configuration;
}

int main(int argc, char **argv)
{
  TupleRef t = create_tuple(1);
  ref< vec< TupleRef > > tupleRefBuffer =
    New refcounted< vec< TupleRef > >();
  tupleRefBuffer->push_back(t);

  Router::ConfigurationRef configuration =
    createConfiguration(tupleRefBuffer);

  MasterRef master = New refcounted< Master >();
  RouterRef router = New refcounted< Router >(configuration, master);
  router->initialize();
  router->activate();
  master->run();

  return 0;
}
  

/*
 * End of file 
 */
