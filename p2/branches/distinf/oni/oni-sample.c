#include "oni.h"
#include <stdio.h>
#ifdef __cplusplus
#error
#endif

// Non typesafe, verbose style
ONI_TUP_STAGE(echo) {
  // tup * t;
  int64_t i = int_Val(Val_Tup(tup,2)); //*val0;
  const char * c = str_Val(Val_Tup(tup,3)); //val1;
  //  void * v = opaque_Val(Val_Tup(tup,2),&size);

  printf("echo: %lld, %s\n",(long long)i,c);

  // alloc_Val_* allocates storage.
  val_t * iV = alloc_Val_int(i);
  //  val_t * cV = alloc_Val_str(c);

  // Tup_Push copies the allocated storage.
  Tup_Push(ret,iV);
  // It's better to use the typed versions of Tup_Push(); you don't
  // need to call free_*, and, it's more efficient.
  Tup_Push_str(ret,c);

  // free_Val_*() needs to be called on things returned from alloc_Val_*()
  free_Val_int(iV);
  //  free_Val_str(cV);

  return 0;
} END_ONI_STAGE
