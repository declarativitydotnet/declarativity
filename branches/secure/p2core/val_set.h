/*
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 * DESCRIPTION: P2's concrete type system: the set type.
 * 
 */
 
 
#ifndef __VAL_SET_H__
#define __VAL_SET_H__
 
#include "value.h"
#include "set.h"
#include "oper.h"

class Val_Set : public Value {    

public:
   
   const Value::TypeCode typeCode() const { return Value::SET; };
   
   // The type name
   const char *typeName() const { return "set"; };
   
   // Print the set as a string.
   string toString() const { return L->toString(); };
   virtual string toConfString() const;

   // Factory
   static ValuePtr mk(SetPtr lp) { ValuePtr p(new Val_Set(lp)); return p; };
   
   // Constructors
   Val_Set(SetPtr lp) : L(lp) {};
   virtual ~Val_Set() {};

   // Get the size of the set as an unsigned int.
   virtual unsigned int size() const { return (L ? L->size() : 0); }
  
   int compareTo(ValuePtr v) const;
   
   // Casting methods; only relevant to extract underlying set.
   static SetPtr cast(ValuePtr v);
   const ValuePtr toMe(ValuePtr other) const { return mk(cast(other)); }
   
   
   // Marshal/unmarshal a set.
   void xdr_marshal_subtype( XDR *x );
   
   static ValuePtr xdr_unmarshal( XDR *x );
   
   static const opr::Oper* oper_;
   
private:
   SetPtr L;
};

#endif /* __VAL_SET_H_*/

/* 
 * End of file
 */
