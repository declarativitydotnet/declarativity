// -*- cr-basic-offset: 2; related-file-name: "tupleseq.h" -*-
/*
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 */

#include <iostream>
#include "bw.h"

Bandwidth::Bandwidth(str name)
  : Element(name,1, 1), prev_t_(now_s()), bytes_(0), bw_(0.)
{
}


TuplePtr Bandwidth::simple_action(TuplePtr p)
{
  time_t cur_t = now_s();

  for (unsigned int i = 0; i < p->size(); i++)
    bytes_ += (*p)[0]->size();

  if ((cur_t - prev_t_) > 2) { 
    bw_ = double(bytes_) / double(cur_t - prev_t_);
    bytes_ = 0;  
  } else return p;

  prev_t_ = cur_t;

  std::cout << "Bandwidth " << bw_ << " Bps" << std::endl;

  return p;
}

REMOVABLE_INLINE time_t Bandwidth::now_s() const
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec;		// Time in seconds
}
