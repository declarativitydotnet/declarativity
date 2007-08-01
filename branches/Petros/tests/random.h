/*
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 *
 */
#ifndef __RANDOM_H__
#define __RANDOM_H__

#include <fstream>
#include <iostream>
#include <ctime>
#include <cstdlib>

#define INIT_RAND(SEED) srand(SEED)
#define RANDOM rand ();

class random { public:
  /** MUST CALL INITIALIZE */
  static void initialize () {
    INIT_RAND (static_cast< unsigned > (time (0)));
  }

  static u_int32_t uint32t_urand () {
    std::ifstream randomStream;
    randomStream.open("/dev/urandom");

    if (randomStream.fail ()) {
      fatal << "Failed to OPEN /dev/urandom\n";
      exit(-1);
    }

    char randomChar;
    randomStream.get(randomChar);

    randomStream.close ();

    return u_int32_t (randomChar);
  }

  static u_int32_t uint32t_rand () {
    return RANDOM;
  }

};

#undef INIT_RAND
#undef RANDOM

#endif /** !__RANDOM_H__ */

