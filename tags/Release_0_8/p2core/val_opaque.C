/*
 * @(#)$Id$
 *
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 * DESCRIPTION: P2's concrete type system: String type.
 *
 */

#include "val_opaque.h"
#include "val_str.h"

const opr::Oper* Val_Opaque::oper_ = new opr::OperCompare<Val_Opaque>();


//
// Marshal an opaque
// 
void Val_Opaque::xdr_marshal_subtype( XDR *x ) 
{
  uint32_t l = b->length();
  xdr_uint32_t(x, &l);
  xdr_opaque(x, b->raw_inline(l), l);
}

ValuePtr Val_Opaque::xdr_unmarshal( XDR *x )
{
  FdbufPtr fb(new Fdbuf());
  uint32_t l;
  xdr_uint32_t(x, &l);
  static const unsigned STATIC_BYTE_BUFFER = 10000;

  if (l < STATIC_BYTE_BUFFER) {
    // We can use the static buffer
    static char byteBuffer[STATIC_BYTE_BUFFER];
    static char* bb = &(byteBuffer[0]);
    memset(bb, 0, STATIC_BYTE_BUFFER);
    xdr_opaque(x, bb, l);
    fb->push_bytes(bb, l);
  }  else {
    // We can't use the static buffer. We must allocate a one-shot
    // buffer
    char * localBuffer = new char[l+1];
    memset(localBuffer, 0, STATIC_BYTE_BUFFER);
    xdr_opaque(x, localBuffer, l);
    fb->push_bytes(localBuffer, l);
    delete localBuffer;
  }

  return mk(fb);
}

string Val_Opaque::toConfString() const
{
  warn << "Cannot get conf string for an OPAQUE value\n";
  return "";
}

//
// Casting
//
FdbufPtr Val_Opaque::cast(ValuePtr v)
{
  switch (v->typeCode()) {
  case Value::OPAQUE:
    return (static_cast<Val_Opaque *>(v.get()))->b;
  case Value::STR:
    {
      FdbufPtr fb(new Fdbuf());
      fb->pushBack(Val_Str::cast(v));
      return fb;
    }
  default:
    throw Value::TypeError(v->typeCode(), Value::OPAQUE );
  }
}
  
int Val_Opaque::compareTo(ValuePtr other) const
{
  if (other->typeCode() != Value::OPAQUE) {
    if (Value::OPAQUE < other->typeCode()) {
      return -1;
    } else if (Value::OPAQUE > other->typeCode()) {
      return 1;
    }
  }
  warn << "Comparing opaques. Not implemented yet!!!\n";
  exit(-1);
  //  return cmp(b, cast(other));
  return -1;
}

/* 
 * End of file
 */
