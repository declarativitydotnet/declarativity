#include "val_ip_addr.h"
#include <iostream>
#include "val_str.h"
#include "math.h"
#include <loggerI.h>

const opr::Oper* Val_IP_ADDR::oper_ = New opr::OperCompare< Val_IP_ADDR> ();

//
// Marshalling and unmarshallng
//
void Val_IP_ADDR::xdr_marshal_subtype( XDR *x )
{
  exit(-1);
  return;
}



//
// Casting
//
str Val_IP_ADDR::cast(ValueRef v) {
  Value *vp = v;
  if(v->typeCode() == Value::IP_ADDR){
    return (static_cast<Val_IP_ADDR *>(vp))->_s;
  }
  else if(v->typeCode() == Value::STR){
    return Val_Str::cast(v);
  }
  else{
    throw Value::TypeError(v->typeCode(), Value::IP_ADDR );
  }
}

/**
 * Returns the suio constructed from the ip-address string. Also checks if the string
 * is in correct format, i.e. xx.xx.xx.xx:xx. 
 **/
ref<suio> Val_IP_ADDR::getAddress()
{
  ref<suio> x = New refcounted< suio> ();
  struct sockaddr_in saddr;

  char * theAtSign = strchr(_s, ':');
  
  if (theAtSign == NULL) {
    // Couldn't find the correct format
    std::cout << "The IP Address is not in correct format:" << toString() << "\n";
    exit(-1);
    //return 0;
  }
  
  str theAddress(_s, theAtSign - _s);
  str thePort(theAtSign + 1);
  int port = atoi(thePort);
  bzero(&saddr, sizeof(saddr));
  saddr.sin_port = htons(port);
  inet_pton(AF_INET, _s.cstr(),
	    &saddr.sin_addr);
  x->copy(&saddr, sizeof(saddr));
  return x;
  
}

int Val_IP_ADDR::compareTo(ValueRef other) const
{
  if (other->typeCode() != Value::IP_ADDR) {
    if (Value::IP_ADDR < other->typeCode()) {
      return -1;
    } else if (Value::IP_ADDR > other->typeCode()) {
      return 1;
    }
  }
  return toString().cmp(other->toString());
}

/*
 * End of file
 */
