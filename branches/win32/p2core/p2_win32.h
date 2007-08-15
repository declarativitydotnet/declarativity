#ifndef P2_WIN32_H
#define P2_WIN32_H

#include <stdlib.h>
#ifdef VISUAL_LEAK_DETECTOR
// #include "vld.h"
#endif // VISUAL_LEAK_DETECTOR

typedef __int64 int64_t;
typedef __int32 int32_t;
typedef __int16 int16_t;
typedef unsigned __int64 uint64_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int16 uint16_t;
typedef unsigned int uint;
typedef unsigned __int64 u_int64_t;
typedef unsigned __int32 u_int32_t;
typedef unsigned __int16 u_int16_t;
typedef unsigned int u_int;
typedef long ssize_t;
typedef int bool_t;
typedef int socklen_t;
#ifndef caddr_t
typedef char* caddr_t;
#endif /* caddr_t */

#ifndef HAVE_RANDOM
inline int32_t random() { return (int32_t) rand(); }
inline void srandom(unsigned int seed) { srand(seed); }
#endif

#ifndef strtoull
#define strtoull _strtoui64
#endif

#ifndef strtoll
#define strtoll _strtoi64
#endif


#ifndef llabs
#define llabs _abs64
#endif

struct timespec {
	long tv_sec;
    long tv_nsec;
};

inline int32_t drand48() { unsigned int retval; (void) rand_s(&retval); return (int32_t) retval; }
#define LONG_LONG_MAX _I64_MAX
#define LONG_LONG_MIN _I64_MIN
#define ULONG_LONG_MAX _UI64_MAX
#define ULONG_LONG_MIN _UI64_MIN

#define NOMINMAX // don't let windef.h put in macros for min and max, screwing up std::min

// VC++ is being over-careful with this one.  See 
// http://groups.google.com/group/boost-list/browse_thread/thread/79b5a5bb13f37576/7773443825609bb1%237773443825609bb1
#pragma warning(disable: 4244) 
#endif /* P2_WIN32_H */