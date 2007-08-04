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

#include "val_str.h"
#include "val_double.h"

class OperStr : public opr::OperCompare<Val_Str> {
  virtual ValuePtr _plus (const ValuePtr& v1, const ValuePtr& v2) const {
    return Val_Str::mk( Val_Str::cast(v1) + Val_Str::cast(v2) );
  };
};
const opr::Oper* Val_Str::oper_ = new OperStr();

string Val_Str::toConfString() const
{
  ostringstream conf;
  conf << "Val_Str(\"" << s << "\")";
  return conf.str();
}

//
// Marshal a string
// 
void Val_Str::marshal_subtype( boost::archive::text_oarchive *x )
{
  const char *st = s.c_str();
  int32_t sl = s.length();
  *x & sl;
  x->save_binary(const_cast<char *>(st), sl + 1);
}


ValuePtr Val_Str::unmarshal( boost::archive::text_iarchive *x )
{
  int32_t sl;
  *x & sl;
  // Now fetch the string itself
  static const int STATIC_STRING_BUFFER = 10000;

  if (sl + 1 <= STATIC_STRING_BUFFER) {
    // We can use the static buffer
    static char stringBuffer[STATIC_STRING_BUFFER];
    static char* sb = &(stringBuffer[0]);
    x->load_binary(sb, sl + 1);
    sb[sl] = 0;       // make sure it's null terminated
    string st(sb, sl);
    return mk(st);
  }  else {
    // We can't use the static buffer. We must allocate a one-shot
    // buffer
    char * localBuffer = new char[sl + 1];
    x->load_binary(localBuffer, sl);
    localBuffer[sl] = 0;
    string st(localBuffer, sl);
    delete localBuffer;
    return mk(st);
  }
}

int Val_Str::compareTo(ValuePtr other) const
{
  if (Value::STR < other->typeCode() ) {
    return -1;
  } else if(Value::STR > other->typeCode() ) {
    return 1;
  } else {
    return s.compare(cast(other));
  }
}

//
// Casting: we special-case doubles...
//
string Val_Str::cast(ValuePtr v)
{
  return v->toString();
}

/* 
 * End of file
 */
