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
#include "cumulativeAck.h"
#include "val_uint64.h"
#include "val_uint32.h"
#include "val_double.h"
#include "val_str.h"
#include "val_tuple.h"
#include "netglobals.h"
#include <boost/bind.hpp>

DEFINE_ELEMENT_INITS(CumulativeAck, "CumulativeAck")

#define CONNECTION_TIMEOUT 240

void CumulativeAck::Connection::confirm(SeqNum seq) {
  if (seq < _cum_seq) {
    // DO NOTHING: Already got this one
  } 
  else if (_cum_seq + 1 == seq) {
    _cum_seq++;			// Got what we expected
  }
  else {
    // Insert in sequence order
    std::vector<SeqNum>::iterator iter; 
    for (iter = _queue.begin(); iter != _queue.end(); iter++) {
      if (seq == *iter) {
        return;		// We already have it
      }
      else if (seq < *iter) {
        _queue.insert(iter, seq);
        break;
      } 
    }
    if (iter == _queue.end()) {
      _queue.push_back(seq);
    }
  }

  // Move the next sequence pointer if we've filled a hole
  for (std::vector<SeqNum>::iterator iter = _queue.begin(); 
       iter != _queue.end(); iter++) {
    if (_cum_seq+1 == *iter) _cum_seq++;
    else if (_cum_seq < *iter) break;
  }

  for (std::vector<SeqNum>::iterator iter = _queue.begin(); 
       iter != _queue.end(); ) {
    if (*iter <= _cum_seq)
      iter = _queue.erase(iter);
    else iter++;
  }
}

CumulativeAck::CumulativeAck(string n)
  : Element(n, 1, 1) { }

CumulativeAck::CumulativeAck(TuplePtr args)
  : Element(Val_Str::cast((*args)[2]), 1, 1) { }

TuplePtr CumulativeAck::simple_action(TuplePtr tp)
{
  ValuePtr src = (*tp)[SRC+2];
  SeqNum   seq = Val_UInt64::cast((*tp)[SEQ+2]);
  SeqNum   srcCumulativeSeq = Val_UInt64::cast((*tp)[CUMSEQ+2]);
  ConnectionPtr cp = lookup(src);

  if (srcCumulativeSeq == 0 && cp) {
    if (cp->_tcb != NULL) timeCBRemove(cp->_tcb);
    unmap(src);
    cp.reset();
  }

  if (!cp) {
    cp.reset(new Connection());
    map(src, cp);
  }

  if (srcCumulativeSeq == 0) {
    cp->_cum_seq = seq;
  }
  else if (cp->_cum_seq < srcCumulativeSeq) {
    cp->_cum_seq = srcCumulativeSeq;
  }
  cp->confirm(seq);

  TuplePtr cack = Tuple::mk();
  for (unsigned i = 0; i < tp->size(); i++) {
    if (i == CUMSEQ+2) {
      cack->append(Val_UInt64::mk(cp->_cum_seq));
    }
    else {
      cack->append((*tp)[i]);
    }
  }

  if (cp->_tcb != NULL) {
    timeCBRemove(cp->_tcb);
    cp->_tcb = NULL;
  }
  cp->_tcb = delayCB(CONNECTION_TIMEOUT,
                     boost::bind(&CumulativeAck::unmap, this, src), this);

  return cack;
}

REMOVABLE_INLINE void CumulativeAck::map(ValuePtr src, CumulativeAck::ConnectionPtr cp)
{
  unmap(src);
  _cmap.insert(std::make_pair(src, cp));
}

REMOVABLE_INLINE void CumulativeAck::unmap(ValuePtr src)
{
  CumulativeAck::ConnectionMap::iterator iter = _cmap.find(src);
  if (iter != _cmap.end()) {
    _cmap.erase(iter);
  }
}

REMOVABLE_INLINE CumulativeAck::ConnectionPtr CumulativeAck::lookup(ValuePtr src)
{
  CumulativeAck::ConnectionMap::iterator iter = _cmap.find(src);
  return iter == _cmap.end() ? CumulativeAck::ConnectionPtr() : iter->second;
}

