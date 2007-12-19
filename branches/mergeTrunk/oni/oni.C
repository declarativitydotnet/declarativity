#include "oni.h"
#include "stage.h"
#include "value.h"
#include "val_int64.h"
#include "val_str.h"
#include "val_opaque.h"
#include "val_null.h"
#include <stdarg.h>

#define ONI_LOG(_sev,_rest)						\
  "ONI, "								\
  << Val_Time::mk(boost::posix_time::second_clock::local_time())	\
  << ", " << _sev << ", " << _rest

#define ONI_CPP_ERROR(_rest) TELL_ERROR  \
  << ONI_LOG(Reporting::ERROR, _rest)	  \
  << std::endl

void oni_log(int type, char * fmt,...) {
  va_list va_l;
  va_start(va_l, fmt);
  int len = vsnprintf(0,0,fmt,va_l);
  va_end(va_l);
  va_list va_l2;
  va_start(va_l2, fmt);
  char * ret = (char*)malloc(len+1);
  vsnprintf(ret,len+1,fmt,va_l2);
  va_end(va_l);
  switch(type) {
  case ONI_INFO_NUM: {
    TELL_INFO << ONI_LOG(Reporting::INFO, ret);
    break;
  }
  case ONI_WORDY_NUM: {
    TELL_WORDY << ONI_LOG(Reporting::WORDY, ret);
    break;
  }
  case ONI_WARN_NUM: {
    TELL_WARN << ONI_LOG(Reporting::WARN, ret);
    break;
  }
  case ONI_ERROR_NUM: {
    TELL_ERROR << ONI_LOG(Reporting::ERROR, ret);
    break;
  }
  case ONI_OUTPUT_NUM: {
    TELL_OUTPUT << ONI_LOG(Reporting::OUTPUT, ret);
    break;
  }
  default:
    abort();
  }
  free(ret);
}

/**
    A note on memory management:

    These methods take and return raw pointers that are being managed
    by boost::shared_ptr (ie: Value*'s and Tuple*'s).  This allows
    them to avoid exposing memory management issues to their callers,
    but forces callers to be careful not to keep pointers between
    stage invocations.  Otherwise, the last reference to the shared
    pointer could go away, causing the object to be freed.
    Presumably, while we're in these functions, some caller has the
    ValuePtr in question on its stack, implicitly locking the raw
    pointer against deletion.

    I considered passing ValuePtr's around, but ValuePtr's are
    objects, and C can't understand them.  Alternatively, we could
    pass ValuePtr*'s around, but we'd have to manually allocate the
    ValuePtrs on the heap, forcing callers to track and delete them.
 */


int64_t int_Val(val_t* v) {
  Value* iV = (Value*)v;
  if(iV->typeCode() != Value::INT64) {
    ONI_CPP_ERROR("int_Val() encountered unexpected value type: " <<
		iV->typeCode() << std::endl);
    abort();
  }
  return Val_Int64::raw_val(*(Val_Int64*)iV);
}
const char *  str_Val(val_t* v) {
  Value* sV = (Value*)v;
  if(sV->typeCode() != Value::STR) {
    ONI_CPP_ERROR("str_Val() encountered unexpected value type: " <<
		sV->typeCode() << std::endl);
    abort();
  }
  return Val_Str::raw_val(*(Val_Str*)sV).c_str();
}
void *  opaque_Val(val_t* v, int64_t *size) {
  Value* oV = (Value*)v;
  if(oV->typeCode() != Value::OPAQUE) {
    ONI_CPP_ERROR("opaque_Val() encountered unexpected value type: " <<
		oV->typeCode() << std::endl);
    abort();
  }
  FdbufPtr fdp = Val_Opaque::raw_val(*(Val_Opaque*)oV);
  *size = fdp->length();
  return fdp->cstr();
}

val_t* alloc_Val_int(int64_t i) {
  return (val_t*)new Val_Int64(i);
}
void free_Val_int(val_t *v) {
  delete (Val_Int64*)v;
}
val_t* alloc_Val_str(const char* c) {
  return (val_t*)new Val_Str(c);
}
void free_Val_str(val_t *v) {
  delete (Val_Str*)v;
}
val_t* alloc_Val_opaque(void * v, int64_t size) {
  FdbufPtr fdp(new Fdbuf());
  fdp->push_bytes((const char*)v,size);
  return (val_t*)new Val_Opaque(fdp);
}
void free_Val_opaque(val_t *v) {
  delete (Val_Opaque*)v;
}
val_t* Val_Tup(tup_t *t, int64_t i) {
  return (val_t*) boost::get_pointer((*(Tuple*)t)[i]);
}
void Tup_Push(tup_t *t, val_t* v) {
  // XXX this switch probably should be somewhere else, like in value.C.
  switch(((Value*)v)->typeCode()) {
  case Value::OPAQUE: {
    ((Tuple*)t)->append(Val_Opaque::mk(Val_Opaque::raw_val(*(Val_Opaque*)v)));
    break;
  }
  case Value::STR: {
    ((Tuple*)t)->append(Val_Str::mk(Val_Str::raw_val(*(Val_Str*)v)));
    break;
  }
  case Value::INT64: {
    ((Tuple*)t)->append(Val_Int64::mk(Val_Int64::raw_val(*(Val_Int64*)v)));
    break;
  }
  default: abort();
  }
}
void Tup_Push_int(tup_t* t, int i) {
  ((Tuple*)t)->append(Val_Int64::mk(i));
}
void Tup_Push_str(tup_t* t, const char* c) {
  ((Tuple*)t)->append(Val_Str::mk(c));
}
void Tup_Push_opaque(tup_t* t, void* buf, int64_t size) {
  FdbufPtr fdp(new Fdbuf());
  fdp->push_bytes((const char*)buf,size);
  ((Tuple*)t)->append(Val_Opaque::mk(fdp));
}
void Tup_Push_Null(tup_t* t) {
  ((Tuple*)t)->append(Val_Null::mk());
}
int64_t colCount_Tup(tup_t *t) {
  return ((Tuple*)t)->size();
}
