#ifndef __VAL_IP_ADDR_H__
#define __VAL_IP_ADDR_H__

#include "value.h"
#include "val_str.h"
#include <loggerI.h>
#include "fdbuf.h"

class Val_IP_ADDR : public Value {
  
 public:  
  
  // Required fields for all concrete types.
  // The type name
  const Value::TypeCode typeCode() const { return Value::IP_ADDR; };
  const char *typeName() const { return "ip_addr"; };
  string toString() const { return _s; };
  virtual unsigned int size() const { _s.length(); }
  
  // Marshalling and unmarshallng
  void xdr_marshal_subtype( XDR *x );
  //static ValuePtr xdr_unmarshal( XDR *x );
  
  // Constructor
  // takes in a string of format "xx.xx.xx.xx:port"
  Val_IP_ADDR(string s) : _s(s){};
  virtual ~Val_IP_ADDR(){};
  
  // Factory
  static ValuePtr mk(string s){ ValuePtr p(new Val_IP_ADDR(s)); return p;};
  
  // Strict comparison
  int compareTo(ValuePtr) const;
  
  static const opr::Oper* oper_;
  
  /** My logging level */
  LoggerI::Level loggingLevel;

  /** My local logger */
  LoggerI * _logger;

  // Casting
  static string cast(ValuePtr v);

#ifndef SWIG
  FdbufPtr getAddress();
#endif
  
 private:
  string _s;
};

#endif /* __VAL_IP_ADDR_H_ */
