/*
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 * DESCRIPTION: P2's concrete type system: the list type.
 * 
 */
 
 
#ifndef __VAL_LIST_H__
#define __VAL_LIST_H__
 
#include "value.h"
#include "list.h"
#include "oper.h"

class Val_List : public Value {    

public:
   
   const Value::TypeCode typeCode() const { return Value::LIST; };
   
   // The type name
   const char *typeName() const { return "list"; };
   
   // Print the list as a string.
   string toString() const { return L->toString(); };
   virtual string toConfString() const;

   // Factory
   static ValuePtr mk(ListPtr lp) { ValuePtr p(new Val_List(lp)); return p; };
   
   // Constructors
   Val_List(ListPtr lp) : L(lp) {};
   virtual ~Val_List() {};

   // Get the size of the list as an unsigned int.
   virtual unsigned int size() const { return (L ? L->size() : 0); }
  
   int compareTo(ValuePtr v) const;
   
   // Casting methods; only relevant to extract underlying list.
   static ListPtr cast(ValuePtr v);
     const ValuePtr toMe(ValuePtr other) const { return mk(cast(other)); }
   
   
   // Marshal/unmarshal a list.
   void marshal_subtype( boost::archive::text_oarchive *x );
   
   static ValuePtr unmarshal( boost::archive::text_iarchive *x );
   
   static const opr::Oper* oper_;
   
private:
   ListPtr L;
};

#endif /* __VAL_LIST_H_*/

/* 
 * End of file
 */
