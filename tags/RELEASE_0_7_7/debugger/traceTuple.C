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

#include "traceTuple.h"
#include "val_tuple.h"
#include "val_str.h"
#include "val_uint32.h"
#include "loggerI.h"

TraceTuple::TraceTuple(string id, string tupleName)
  : Element(id, 1, 2),
    _id(id),
    _tupleName(tupleName),
    _push_cb(0),
    _block_flags(),
    _block_flag_count(0)
{
  _block_flags.resize(noutputs());
}

TraceTuple::~TraceTuple()
{
}

void TraceTuple::unblock(unsigned output)
{
  assert(output <= noutputs());

  if(_block_flags[output]){
    log(Reporting::INFO, -1, "unblock");
    _block_flags[output] = false;
    _block_flag_count --;
    assert(_block_flag_count >= 0);
  }

  // call a pushback if we have it
  if(_push_cb){
    log(Reporting::INFO, -1, "unblock: propagating aggregate unblock");
    _push_cb();
    _push_cb = 0;
  }
}


int TraceTuple::push(int port, TuplePtr p, b_cbv cb)
{
  log(Reporting::INFO, -1, "Tracing element " + name() + " handling tuple " 
      + p->toString() + "\n");

  if(output(0)->push(p, boost::bind(&TraceTuple::unblock, this, 0)) == 0){
    log(Reporting::WARN, -1, " Problem in pushing the original tuple " 
	+ p->toString() + "\n");
    _block_flags[0] = true;
    _block_flag_count ++;
  }
  
  assert(_tupleName == ((*p)[0])->toString());  
  
  // create a new tuple
  TuplePtr t = Tuple::mk();
  t->append(Val_Str::mk(name()));

  // append the fields
  for(uint32_t i = 1; i < p->size(); i++)
    t->append((*p)[i]);
  t->append(Val_UInt32::mk(p->ID()));
  t->freeze();
  log(Reporting::INFO, -1, "Produced tuple " + t->toString() + "\n");

  //TELL_INFO << "Produced tuple " << t->toString() << "\n";

  if(output(1)->push(t, boost::bind(&TraceTuple::unblock, this, 1)) == 0){
    log(Reporting::WARN, -1, " Problem in pushing the trace tuple " 
	+ t->toString() + "\n");
    _block_flags[1] = true;
    _block_flag_count ++;
  }

  if(_block_flag_count > 0){
    _push_cb = cb;
    log(Reporting::WARN, -1, "push: Blocking input");
    return 0;
  }
  else
    return 1;
}
