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

#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH

DEFINE_ELEMENT_INITS(Bandwidth, "Bandwidth")

Bandwidth::Bandwidth(string name) 
: Element(name,1, 1), prev_t(now_s()), bytes(0), total_size(0), total_count(0)
{ }


Bandwidth::Bandwidth(TuplePtr args)
  : Element(Val_Str::cast((*args)[2]), 1, 1), 
    prev_t(now_s()), bytes(0), total_size(0), total_count(0) { }


TuplePtr
Bandwidth::simple_action(TuplePtr p)
{
  using std::endl;

  time_t cur_t = now_s();
  TuplePtr msg = Val_Tuple::cast((*p)[1])->clone();
  string name = (*msg)[0]->toString();
  msg->freeze();

  FdbufPtr fb(new Fdbuf());
  XDR xe;
  xdrfdbuf_create(&xe, fb.get(), false, XDR_ENCODE);
  msg->xdr_marshal(&xe);
  xdr_destroy(&xe);

  // std::cerr << "TUPLE NAME: " << name << " size " << fb->length() << endl;
  bytes += fb->length();

  tuple_size[name] += fb->length();
  tuple_count[name]++;

  total_size += fb->length();  
  total_count++;
  

  if ((cur_t - prev_t) > 1) { 
    std::ostream& out = *Reporting::warn();
    string cur_time = 
      to_simple_string(boost::posix_time::microsec_clock::local_time());

    // log the instantaneous bandwidth 
    double bw = double(bytes) / double(cur_t - prev_t);
    TELL_OUTPUT << "Bandwidth, " << bw << ", " << cur_time << endl;

    // log the sizes of all the tuples
    TELL_OUTPUT << "TupleSize, " << total_size << ", " << cur_time;
    foreach(tuple_map::reference p, tuple_size) {
      out << ", " << p.first << ":" << p.second;
    }
    out << endl;

    // log the size of all tuples
    TELL_OUTPUT << "TupleCount, " << total_count << ", " << cur_time;
    foreach(tuple_map::reference p, tuple_count) {
      out << ", " << p.first << ":" << p.second;
    }
    out << endl;

    // clear the old information
    prev_t = cur_t;
    bytes = 0;
    tuple_size.clear();
    tuple_count.clear();
  } 

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
