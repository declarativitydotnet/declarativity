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
#ifdef WIN32
#include "p2_win32.h"
#endif // WIN32
#include <iostream>
#include "bw.h"

Bandwidth::Bandwidth(string name)
  : Element(name,1, 1), prev_t_(now_s()), bytes_(0), bw_(0.)
{
}


TuplePtr
Bandwidth::simple_action(TuplePtr p)
{
  time_t cur_t = now_s();

  for (unsigned int i = 0;
       i < p->size();
       i++) {
    bytes_ += (*p)[i]->size();
  }

  if ((cur_t - prev_t_) > 2) { 
    bw_ = double(bytes_) / double(cur_t - prev_t_);
    bytes_ = 0;  
  } else return p;

  prev_t_ = cur_t;

  std::ostringstream o;
  o << bw_;
  log(mMarkup_, Reporting::INFO, 0, o.str());
      
  return p;
}


REMOVABLE_INLINE time_t
Bandwidth::now_s() const
{
#ifdef WIN32
  SYSTEMTIME st;
  GetSystemTime(&st);
  return(st.wSecond);
#else
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec;		// Time in seconds
#endif
}


void
Bandwidth::setMarkup(std::string m)
{
  mMarkup_ = "[" + name() + "] " + m;
}
