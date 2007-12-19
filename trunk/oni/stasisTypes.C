#include "oni.h"
#include "stasis.h"
#include "stage.h"
#include "value.h"
#include "val_int64.h"
#include "val_opaque.h"
#include "fdbuf.h"
#include "xdrbuf.h"
val_t * alloc_Val_recordid(recordid rid) {
  FdbufPtr fdp(new Fdbuf());
  fdp->push_uint64(rid.page);
  fdp->push_uint32(rid.slot);
  fdp->push_uint32(rid.size); /// XXX no signed ints w/ fdbuf
  //  std::cout << "rid{"<<rid.page<<","<<rid.slot<<","<<rid.size<<"}->value"<<std::endl;
  return (val_t*) new Val_Opaque(fdp);
}
void free_Val_recordid(val_t* v) {
  delete (Val_Opaque*)v;
}
void Tup_Push_recordid(tup_t * t, recordid rid) {
  FdbufPtr fdp(new Fdbuf());
  fdp->push_uint64(rid.page);
  fdp->push_uint32(rid.slot);
  fdp->push_uint32(rid.size); /// XXX no signed ints w/ fdbuf
  ((Tuple*)t)->append(Val_Opaque::mk(fdp));
}
recordid recordid_Val(val_t * val) {
  recordid rid;
  FdbufPtr fdp = Val_Opaque::raw_val(*(Val_Opaque*)val);
  Fdbuf fd(fdp);
  rid.page = fd.pop_uint64();
  rid.slot = fd.pop_uint32();
  rid.size = fd.pop_uint32();
  //std::cout << "rid{"<<rid.page<<","<<rid.slot<<","<<rid.size<<"}<-value"<<std::endl;
  return rid;
}
byte * colsBytes_Tup(tup_t* tup, unsigned int startCol, unsigned int count, size_t * buf_len) {
  Tuple t;

  for(int i = 0; i < count; i++) {
    t.append((*(Tuple*)tup)[i+startCol]);
  }
  t.freeze();
  Fdbuf f; // XXX should just be an Fdbuf...

  XDR xe;
  xdrfdbuf_create(&xe, &f, false, XDR_ENCODE);
  t.xdr_marshal(&xe);
  xdr_destroy(&xe);

  *buf_len = f.length();

  byte * ret = (byte*)malloc(*buf_len);
  bool r = f.pop_bytes((char*)ret,*buf_len);

  assert(r);
  return ret;
}
void Tup_Push_colsBytes(tup_t* tup, const byte * buf, size_t buf_len) {
  Fdbuf f;

  f.push_bytes((const char*)buf, buf_len);

  XDR xd;
  xdrfdbuf_create(&xd,&f,false,XDR_DECODE);
  TuplePtr unmarshal = Tuple::xdr_unmarshal(&xd);
  xdr_destroy(&xd);
  for(unsigned int i = 0; i < unmarshal->size(); i++) {
    ((Tuple*)tup)->append((*unmarshal)[i]);
  }
}
