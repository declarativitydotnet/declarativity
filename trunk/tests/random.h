
#ifndef __RANDOM_H__
#define __RANDOM_H__

#include <fstream>
#include <iostream>
#include <ctime>
#include <cstdlib>
#include <async.h>

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

#undef INIT_RAND(SEED)
#undef RANDOM

#endif /** !__RANDOM_H__ */

