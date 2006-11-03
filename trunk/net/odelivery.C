/*
 * 
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
#include "odelivery.h"
#include "val_uint64.h"
#include "val_uint32.h"
#include "val_double.h"
#include "val_str.h"
#include "val_tuple.h"
#include "netglobals.h"

#define CONNECTION_TIMEOUT 240

void ODelivery::Connection::insert(TuplePtr tp) {
  SeqNum seq = Val_UInt64::cast((*tp)[SEQ]);

  if (queue_.size() == 0 && next_seq_ <= seq) {
    queue_.push_back(tp);
  }
  else {
    // Insert the tuple in sequence order
    std::vector<TuplePtr>::iterator iter; 
    for (iter = queue_.begin(); iter != queue_.end(); iter++) {
      SeqNum seq2 = Val_UInt64::cast((**iter)[SEQ]);
      if (seq == seq2) {
        return;		// We already have it
      }
      else if (seq < seq2) {
        queue_.insert(iter, tp);
        break;
      } 
    }
    if (iter == queue_.end()) {
      queue_.push_back(tp);
    }
  }
  // Move the next sequence pointer if we've filled a hole
  for (std::vector<TuplePtr>::iterator iter = queue_.begin(); 
       iter != queue_.end(); iter++) {
    SeqNum seq3 = Val_UInt64::cast((**iter)[SEQ]);
    if (seq3 == next_seq_) next_seq_++;
    else break;
  }
}

long ODelivery::Connection::touch_duration() const {
  boost::posix_time::ptime now;
  getTime(now);

  boost::posix_time::time_duration duration = now - last_touched;
  return duration.seconds();
}

void ODelivery::Connection::touch() {
  getTime(last_touched);
}

ODelivery::ODelivery(string n)
  : Element(n, 1, 1), _in_cb(0), out_on_(1)
{
}

int ODelivery::push(int port, TuplePtr tp, b_cbv cb)
{
  assert(port == 0);

  ValuePtr src = (*tp)[SRC];
  SeqNum   seq = Val_UInt64::cast((*tp)[SEQ]);
  SeqNum  cseq = Val_UInt64::cast((*tp)[CUMSEQ]);
  ConnectionPtr cp = lookup(src);

  if (cseq == 0 && cp) {
    unmap(src);
    cp.reset();
  }

  if (cp && seq < cp->next_seq_) {
    /** We've already received this tuple sequence, so ignore it */
    return out_on_;
  }
  else if (cp) {
    cp->touch();
    cp->insert(tp);
  }
  else {
    cp.reset(new Connection(cseq));
    map(src, cp);
    cp->insert(tp);
  }

  if (out_on_) {
    flush(cp);
  }
  return out_on_;
}

REMOVABLE_INLINE void ODelivery::flush(ConnectionPtr cp) {
  while ( out_on_ && cp->queue_.size() > 0 ) {
    TuplePtr tpl = cp->queue_.front();
    ValuePtr src = (*tpl)[SRC];
    SeqNum   seq = Val_UInt64::cast((*tpl)[SEQ]);
    double   rto = Val_Double::cast((*tpl)[RTT]); 
    if (seq <= cp->next_seq_) {
      out_on_ = output(0)->push(tpl, boost::bind(&ODelivery::out_cb, this));
      cp->queue_.erase(cp->queue_.begin());
      if (seq == cp->next_seq_) {
         cp->next_seq_++;
      }

      /** Remove timeout for the first waiting tuple */
      if (cp->tcb_ != NULL) {
        timeCBRemove(cp->tcb_);
        cp->tcb_ = NULL;
      }
    }
    else if (cp->queue_.size() > 0 && cp->tcb_ == NULL) {
      /** Set timeout for receiving the tuple with the next sequence number */
      cp->tcb_ = delayCB(5.0 + ((10.0 * rto) / 1000.0),
                         boost::bind(&ODelivery::flushConnectionQ, this, src), this);
      break;
    }
    else {
      break;
    }
  }
}

/**
 * Run through all the connections and flush tuples up to the next_seq
 */
REMOVABLE_INLINE void ODelivery::out_cb() 
{
  out_on_ = true;
  for (ConnectionIndex::iterator iter = cmap_.begin();
       iter != cmap_.end() && out_on_; iter++) {
    flush(iter->second);
  }
}

/**
 * Try and flush all tuples in this connection.
 */
REMOVABLE_INLINE void ODelivery::flushConnectionQ(ValuePtr src)
{
  TELL_INFO << "FLUSH CONNECTION QUEUE CALLED" << std::endl;
  ConnectionPtr cp = lookup(src);
  cp->tcb_ = NULL;

  if (cp->queue_.size() > 0) {
    TuplePtr t = cp->queue_.back();
    SeqNum   s = Val_UInt64::cast((*t)[SEQ]);
    cp->next_seq_ = s + 1;
    flush(cp); 
  }

  /** Check if the connection is dead */
  if (cp->queue_.size() == 0 && 
      cp->touch_duration() > CONNECTION_TIMEOUT) {
    TELL_INFO << "REMOVING CONNECTION" << std::endl;
    unmap(src);
  }
}

REMOVABLE_INLINE void ODelivery::map(ValuePtr src, ODelivery::ConnectionPtr cp)
{
  unmap(src);
  cmap_.insert(std::make_pair(src, cp));
}

REMOVABLE_INLINE void ODelivery::unmap(ValuePtr src)
{
  ODelivery::ConnectionIndex::iterator iter = cmap_.find(src);
  if (iter != cmap_.end()) {
    cmap_.erase(iter);
  }
}

REMOVABLE_INLINE ODelivery::ConnectionPtr ODelivery::lookup(ValuePtr src)
{
  ODelivery::ConnectionIndex::iterator iter = cmap_.find(src);
  return iter == cmap_.end() ? ODelivery::ConnectionPtr() : iter->second;
}

