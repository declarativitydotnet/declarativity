// -*- cr-basic-offset: 2; related-file-name: "tupleseq.h" -*-
/*
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 */

#include <iostream>
#include "bw.h"
#include "val_str.h"
#include "val_tuple.h"
#include "fdbuf.h"
#include "xdrbuf.h"

DEFINE_ELEMENT_INITS(Bandwidth, "Bandwidth")

Bandwidth::Bandwidth(string name)
  : Element(name,1, 1), prev_t_(now_s()), bytes_(0), total_bytes(0), bw_(0.)
{
}

Bandwidth::Bandwidth(TuplePtr args)
  : Element(Val_Str::cast((*args)[2]), 1, 1), 
    prev_t_(now_s()), bytes_(0), total_bytes(0), bw_(0.) { 
      hashtable = new std::map<string, std::pair<time_t, unsigned int> >();
    }


TuplePtr
Bandwidth::simple_action(TuplePtr p)
{
  time_t cur_t = now_s();
  TuplePtr msg = Val_Tuple::cast((*p)[1])->clone();
  string name = (*msg)[0]->toString();
  msg->freeze();

  FdbufPtr fb(new Fdbuf());
  XDR xe;
  xdrfdbuf_create(&xe, fb.get(), false, XDR_ENCODE);
  msg->xdr_marshal(&xe);
  xdr_destroy(&xe);

  std::cerr << "TUPLE NAME: " << name << " size " << fb->length() << std::endl;
  std::map<string, std::pair<time_t, unsigned int> >::iterator iter = hashtable->find(name);
  std::pair<time_t, unsigned int> entry = std::make_pair(cur_t, 0);
  if (iter != hashtable->end()) {
    entry = (*iter).second;
  }
  
  entry.second += fb->length();

  if ((cur_t - entry.first) > 2) { 
    double bw_ = double(entry.second) / double(cur_t - entry.first);
    entry.first = cur_t;
    total_bytes = entry.second;
    entry.second = 0;
    //std::cerr << "BANDWIDTH " << name << ": " << bandwidth << " bytes/second." << std::endl;
    TELL_OUTPUT << "Bandwidth, " 
                 << bw_ << ", " 
                 << total_bytes << ", " 
                 << boost::posix_time::to_simple_string(boost::posix_time::microsec_clock::local_time()) 
                 << std::endl; 
  } 

  hashtable->erase(name);
  hashtable->insert(std::make_pair(name, entry));

  return p;
}


REMOVABLE_INLINE time_t
Bandwidth::now_s() const
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec;		// Time in seconds
}


void
Bandwidth::setMarkup(std::string m)
{
  mMarkup_ = "[" + name() + "] " + m;
}
