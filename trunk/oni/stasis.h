#ifndef _ONISTASIS_H_
#define _ONISTASIS_H_
#include "oni.h"
#include <stasis/transactional.h>
#undef end
#undef try

#ifdef __cplusplus
extern "C" {
#endif

  val_t * alloc_Val_recordid(recordid rid);
  void free_Val_recordid(val_t* v);
  void Tup_Push_recordid(tup_t * tup, recordid rid);
  recordid recordid_Val(val_t * val);
  byte * colsBytes_Tup(tup_t* tup, unsigned int startCol, unsigned int count, size_t * buf_len);
  void Tup_Push_colsBytes(tup_t* tup, const byte * buf, size_t buf_len);
#ifdef __cplusplus
}
#endif

#endif // _ONISTASIS_H_
