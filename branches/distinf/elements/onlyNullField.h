#ifndef ONLYNULLFIELD_
#define ONLYNULLFIELD_

#include "element.h"
#include "elementRegistry.h"

class OnlyNullField : public Element { 
public:
  OnlyNullField(string, unsigned);
  OnlyNullField(TuplePtr args);
  ~OnlyNullField();
  
  /** Overridden to perform the projecting. */
  TuplePtr simple_action(TuplePtr p);

  const char *class_name() const		{ return "OnlyNullField";}
  const char *processing() const		{ return "a/a"; }
  const char *flow_code() const			{ return "x/x"; }

  DECLARE_PUBLIC_ELEMENT_INITS

private:
  /** The field number to check */
  unsigned _fieldNo;

  DECLARE_PRIVATE_ELEMENT_INITS
};

#endif /*ONLYNULLFIELD_*/
