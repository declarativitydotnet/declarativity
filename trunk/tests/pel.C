/*
 * @(#)$Id$
 *
 * Copyright (c) 2005 Intel Corporation. All rights reserved.
 *
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 * DESCRIPTION: Test suite for PEL lexer
 *
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include "pel_lexer.h"
#include "pel_program.h"

#include <iostream>


static double time_fn(cbv cb) 
{
  timespec before_ts;
  timespec after_ts;
  double elapsed;
  
  if (clock_gettime(CLOCK_REALTIME,&before_ts)) {
    fatal << "clock_gettime:" << strerror(errno) << "\n";
  }
  (cb)();
  if (clock_gettime(CLOCK_REALTIME,&after_ts)) {
    fatal << "clock_gettime:" << strerror(errno) << "\n";
  }
  
  after_ts = after_ts - before_ts;
  elapsed = after_ts.tv_sec + (1.0 / 1000 / 1000 / 1000 * after_ts.tv_nsec);
  std::cout << elapsed << " secs (";
  std::cout << after_ts.tv_sec << " secs " << (after_ts.tv_nsec/1000) << " usecs)\n";
  return elapsed;
}

struct Test {
  char *src;
  char *disassembly;
  int   num_consts;
  int	num_opcodes;
};

static const Test tests[] = {
  { "  ", "", 0, 0 },
  { "1 2 3", "1 2 3 ", 3, 3},
  { "1 1.2 \"String\" swap dup ", "1 1.2 \"String\" swap dup ", 3, 5},
  { "null\t\n pop $2 $4  ->u32", "null pop $2 $4 ->u32 ", 1, 5},
  { "1 2 /* This is a comment */ pop pop", "1 2 pop pop ", 2, 4},
  { NULL, NULL, 0, 0}
};

int main(int argc, char **argv)
{
  std::cout << "PEL\n";

  for( const Test *t = tests; t->src != NULL; t++) {
    std::cout << "Compiling: " << t->src << "\n";
    Pel_Program *prog = Pel_Lexer::compile( t->src);
    if (prog->ops.size() != t->num_opcodes) {
      std::cerr << "** Bad # opcodes for '" << t->src << "'; " << prog->ops.size() << " instead of expected " << t->num_opcodes << "\n";
    }
    if (prog->const_pool.size() != t->num_consts) {
      std::cerr << "** Bad # consts for '" << t->src << "'; " << prog->const_pool.size() << " instead of expected " << t->num_consts << "\n";
    }
    str dec = Pel_Lexer::decompile(*prog);
    if (dec != t->disassembly) {
      std::cerr << "** Bad disassembly for '" << t->src << "'; '" << dec << "' instead of expected '" << t->disassembly << "'\n";
    }
  }
  return 0;
}
  

/*
 * End of file 
 */
