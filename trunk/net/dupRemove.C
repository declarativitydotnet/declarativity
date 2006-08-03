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
#include "dupRemove.h"
#include "val_uint64.h"
#include "val_uint32.h"
#include "val_double.h"
#include "val_str.h"
#include "val_tuple.h"
#include "netglobals.h"

#define CONNECTION_TIMEOUT 240

bool DupRemove::Connection::received(TuplePtr tp) {
  ValuePtr src = (*tp)[SRC]; 
  SeqNum seq  = Val_UInt64::cast((*tp)[SEQ]);
  SeqNum cseq = Val_UInt64::cast((*tp)[CUMSEQ]);

  if (_cum_seq < cseq) {
    if (_cum_seq && _receiveMap.size() > 0) {
      for (SeqNum s = _cum_seq; s <= cseq; s++) {
        _receiveMap.erase(s);
      }
    }
    _cum_seq = cseq;
  }

  if (seq < _cum_seq) {
    return true;
  }
  else if (_receiveMap.find(seq) != _receiveMap.end()) {
    return true;
  }
  _receiveMap.insert(std::make_pair(seq, true));
  return false;
}

long DupRemove::Connection::touch_duration() const {
  boost::posix_time::ptime now;
  getTime(now);

  boost::posix_time::time_duration duration = now - last_touched;
  return duration.seconds();
}

void DupRemove::Connection::touch() {
  getTime(last_touched);
}

int DupRemove::push(int port, TuplePtr tp, b_cbv cb)
{
  assert(port == 0);

  ValuePtr src = (*tp)[SRC];
  SeqNum  cseq = Val_UInt64::cast((*tp)[CUMSEQ]);
  ConnectionPtr cp = lookup(src);

  if (cseq == 0 && cp) {
    if (cp->_tcb != NULL) timeCBRemove(cp->_tcb);
    unmap(src);
    cp.reset();
  }

  if (!cp) { 
    cp.reset(new Connection());
    map(src, cp);
  }
  cp->touch();

  if (cp->_tcb != NULL) 
    timeCBRemove(cp->_tcb);
  cp->_tcb = delayCB(CONNECTION_TIMEOUT,
                     boost::bind(&DupRemove::unmap, this, src), this);
  if (!cp->received(tp))
    return output(0)->push(tp, cb);
  return 1;
}

REMOVABLE_INLINE void DupRemove::map(ValuePtr src, DupRemove::ConnectionPtr cp)
{
  unmap(src);
  cmap_.insert(std::make_pair(src, cp));
}

REMOVABLE_INLINE void DupRemove::unmap(ValuePtr src)
{
  DupRemove::ConnectionIndex::iterator iter = cmap_.find(src);
  if (iter != cmap_.end()) {
    cmap_.erase(iter);
  }
}

REMOVABLE_INLINE DupRemove::ConnectionPtr DupRemove::lookup(ValuePtr src)
{
  DupRemove::ConnectionIndex::iterator iter = cmap_.find(src);
  return iter == cmap_.end() ? DupRemove::ConnectionPtr() : iter->second;
}

