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

#include <algorithm>
#include <iostream>

#include "rccr.h"
#include "rcct.h"
#include "p2Time.h"
#include "val_uint64.h"
#include "val_uint32.h"
#include "val_double.h"
#include "val_str.h"
#include "val_time.h"
#include "val_tuple.h"
#include "tupleseq.h"

#define RF        0.9
#define HIST_SIZE 100000
#define LOSS_SIZE 8
#define MAX_CTIME (5 * 60 * 1000) 

class TupleInfo {
public:
  TupleInfo(SeqNum s) : seq_(s), rtt_(0) {};
  TupleInfo(SeqNum s, uint r, boost::posix_time::ptime ts) 
    : seq_(s), rtt_(r), ts_(ts) {} 

  /**
   * A temporal comparison method with the usual semantics... 
   * case  0: t is equal to this tuple
   * case  1: t comes after this tuple
   * case -1: t comes before this tuple
   */
  int compare(TupleInfo* t) {
    long d = delay(t);
    if (d < 0)      return -1;
    else if (d > 0) return  1;
    return 0;
  };

  /** Returns the delay in milliseconds between this tuple and t */
  long delay(TupleInfo* t) {
	return((t->ts_ - ts_).total_milliseconds());
  };

  SeqNum   seq_;
  uint     rtt_;
  boost::posix_time::ptime ts_;	/* This timestame comes from the source machine
                                 * I will only use it to compute durations relative
                                 * to the source machines timestamps. */
};

class LossRec {
public:
  LossRec(std::vector<TupleInfo*> history, SeqNum miss) : length_(history.size()) {
    TupleInfo *a = NULL;	// The tuple after the miss
    TupleInfo *b = NULL;	// The tuple just before the miss
    for (std::vector<TupleInfo*>::iterator i = history.begin();
         b == NULL && i != history.end(); i++) {
      if ((*i)->seq_ + 1 == miss) b = *i;
      else a = *i;
    }
    assert(a->seq_ - b->seq_ > 0);
    for (SeqNum s = b->seq_+1; s < a->seq_; s++) {
      // Create a missing record for each missing seqeuence number
      TupleInfo* l = new TupleInfo(s);	
      // Determine when we should have heard from the missing tuple
      nominal_time(l, b, a);
      loss_tuples_.push_back(l); 
    }
  }

  void remove(SeqNum s) {
    for (std::vector<TupleInfo*>::iterator i = loss_tuples_.begin();
         i != loss_tuples_.end(); i++)
      if ((*i)->seq_ == s) {
        delete *i;
        loss_tuples_.erase(i);
        return;
      }
  };
  bool exists(SeqNum s) {
    for (std::vector<TupleInfo*>::iterator i = loss_tuples_.begin();
         i != loss_tuples_.end(); i++)
       if ((*i)->seq_ == s) return true;
    return false;
  };
  bool empty() { return loss_tuples_.empty(); };
  uint length() { return length_; }

private:
  void nominal_time(TupleInfo* t, TupleInfo* b, TupleInfo* a)
  {
    long offset = long(b->delay(a)*(double(t->seq_-b->seq_)/double(a->seq_-b->seq_)));
    t->ts_ = b->ts_ + boost::posix_time::milliseconds(offset);
  }

  uint length_;
  std::vector<TupleInfo*> loss_tuples_;
};

class RateCCR::Connection {
public:
  Connection(uint o=5) 
    : loss_rate_(0.), order_(o), rate_(1), seq_miss_(0),
      i_weights_(LOSS_SIZE) {
    for (int i = 0; i < LOSS_SIZE; i++)
      if (i < LOSS_SIZE/2) i_weights_[i] = 1.;
      else i_weights_[i] = 1. - (double(i) - (LOSS_SIZE/2. - 1.)) /
                               (LOSS_SIZE/2. + 1.); 
    getTime(active_ts_);
  }

  // Computes loss event rate from intervals
  REMOVABLE_INLINE void handle_tuple(SeqNum, uint, boost::posix_time::ptime);
  
  double  lossRate()    { return loss_rate_; }
  uint    receiveRate() { return rate_; }
  bool    active()      { return RateCCT::delay(&active_ts_) < MAX_CTIME; }

  string toString() {
    ostringstream oss;
    oss << "LOSS RATE: " << loss_rate_ << "\t";
    oss << "RATE: " << rate_ << "\t";
    oss << "SEQ MISS: " << seq_miss_ << "\t";
    oss << "ACTIVE TS: " << active_ts_ << "\t";
    return oss.str();
  };
private:
  REMOVABLE_INLINE void compute_loss_rate();
  REMOVABLE_INLINE void clearHistory();

  double                   loss_rate_;   // Cached loss event rate
  uint                     order_;
  uint                     rate_; 	 // Receiver rate
  SeqNum                   seq_miss_;    // Missing sequence number
  boost::posix_time::ptime active_ts_;   // Last time we heard from this connection
  std::vector<TupleInfo*>  history_;     // Tuple history
  std::deque<LossRec*>     loss_recs_;   // The last n loss intervals
  std::vector <double>     i_weights_;   // Interval weights

};

REMOVABLE_INLINE void RateCCR::Connection::handle_tuple(SeqNum seq, 
                                                        uint rtt, 
                                                        boost::posix_time::ptime ts)
{
  getTime(active_ts_);

  TupleInfo *tip = new TupleInfo(seq, rtt, ts);
  rate_ = 1;
  for (std::vector<TupleInfo*>::iterator i = history_.begin();
       i != history_.end(); i++) {
    if ((*i)->compare(tip) >= 0) {
      if ((*i)->delay(tip) < long(rtt)) rate_++;	// Count all tuples within a RTT
      else if ((*i)->delay(tip) > 2 * long(rtt)) break;	// Stop after 2*RTT
    }
  }

  if (seq_miss_ && (seq - seq_miss_) >= order_) {
    // Create a new loss record and reset history.
    loss_recs_.push_front(new LossRec(history_, seq_miss_));
    if (loss_recs_.size() > LOSS_SIZE) 
      loss_recs_.pop_back();
    seq_miss_ = 0;
    clearHistory();
    history_.insert(history_.begin(), tip);
  }
  else if (history_.empty()) {
    history_.insert(history_.begin(), tip);
  }
  else if (history_.front()->seq_ < tip->seq_) {
    history_.insert(history_.begin(), tip);
    if (!seq_miss_ && history_[1]->seq_ + 1 != history_[0]->seq_)
      seq_miss_ = history_[1]->seq_ + 1; 	// New missing packet
  }
  else if (tip->seq_ < history_.back()->seq_) {
    // Did we void out some loss interval ?
    for (std::deque<LossRec*>::iterator i = loss_recs_.begin();
         i != loss_recs_.end(); i++) {
      if ((*i)->exists(seq)) {
        (*i)->remove(seq);
        if ((*i)->empty()) {
          delete *i;
          loss_recs_.erase(i);
          break;
        }
      }
    }
  }
  else {
    // We're filling some hole.
    for (std::vector<TupleInfo*>::iterator i = history_.begin() + 1; 
         i != history_.end(); i++) {
      if ((*i)->seq_ < tip->seq_) { 
        history_.insert(i, tip);
        break;
      }
    }
    if (seq_miss_ == tip->seq_) {
      seq_miss_ = 0;
      // Look for a new missed sequence number
      for (std::vector<TupleInfo*>::iterator i = history_.begin() + 1; 
           i != history_.end() && (*i)->seq_ != tip->seq_; i++) {
         if ((*(i-1))->seq_ != (*i)->seq_+1) seq_miss_ = (*i)->seq_ + 1;
      }
    }
  }
  compute_loss_rate();
}

REMOVABLE_INLINE void RateCCR::Connection::compute_loss_rate() {
  if (!loss_recs_.empty()) {
    double lint = double(history_.size()) * i_weights_[0];
    double norm = i_weights_[0];
    for (uint i = 0; i < loss_recs_.size() && i+1 < LOSS_SIZE; i++) {
      lint += double(loss_recs_[i]->length()) * i_weights_[i];
      norm += i_weights_[i+1];
    }
    lint /= norm;
    loss_rate_ = 1.0 / lint;
  } else loss_rate_ = 0.;
}

REMOVABLE_INLINE void RateCCR::Connection::clearHistory() {
  for (std::vector<TupleInfo*>::iterator i = history_.begin(); 
       i != history_.end(); delete (*i), i++)
    ;
  history_.clear();
}
/////////////////////////////////////////////////////////////////////
//
// Receive element
//

RateCCR::RateCCR(string name, int dest, int src, int seq, int rtt, int ts) 
  : Element(name, 1, 2),
    _ack_cb(0),
    dest_field_(dest),
    src_field_(src),
    seq_field_(seq),
    rtt_field_(rtt),
    ts_field_(ts)
{
}

/**
 * Acknowledge tuple p if ack_q is empty and output1 is open.
 * Otherwise, add to ack_q and enable callback.
 */
TuplePtr RateCCR::simple_action(TuplePtr tp) 
{
  Connection *c  = NULL;
  ValuePtr  src  = (*tp)[src_field_];
  ValuePtr  dest = (*tp)[dest_field_];
  SeqNum    seq  = Val_UInt64::cast((*tp)[seq_field_]);
  int       rtt  = Val_UInt32::cast((*tp)[rtt_field_]);
  boost::posix_time::ptime ts = Val_Time::cast((*tp)[ts_field_]);

  if (seq == 0 || rtt < 0) {
    log(LoggerI::INFO, 0, "NON-RateCC Tuple: " + tp->toString()); 
    return tp;
  }

  if (cmap_.find(src->toString()) == cmap_.end()) {
    c = new Connection(); 
    cmap_.insert(std::make_pair(src->toString(), c));
  } else c = cmap_.find(src->toString())->second;
  assert(c);

/*
  if (!c->active()) { 
    log(LoggerI::WARN, 0, "REMOVING INACTIVE CONNECTION FROM SOURCE: " + src->toString()); 
    cmap_.erase(cmap_.find(src->toString()));
    c = new Connection(); 
    cmap_.insert(std::make_pair(src->toString(), c));
  }
*/
  c->handle_tuple(seq, rtt, ts);

  TuplePtr ack = Tuple::mk();
  ack->append(src);				// Source location
  ack->append(Val_Str::mk("ACK"));		// Acknowledgement name
  ack->append(dest);				// Destination location
  ack->append(Val_UInt64::mk(seq));		// The sequence number
  ack->append(Val_UInt32::mk(c->receiveRate()));// Rate observed in past rtt
  ack->append(Val_Double::mk(c->lossRate()));	// Loss event rate
  ack->append(Val_Time::mk(ts));		// Echo the timestamp
  ack->freeze();
  ack_q_.push_back(ack);			// Append to ack queue

  if (_ack_cb) {
    _ack_cb();				// Notify new ack
    _ack_cb = 0;
  } 
  return strip(tp);				// Forward data tuple
}

/**
 * Pulls the next acknowledgement in ack_q_ to send to the
 * receiver.
 */
TuplePtr RateCCR::pull(int port, b_cbv cb)
{
  if (port == 1) {
    if (!ack_q_.empty()) {
      TuplePtr ack = ack_q_.front();
      ack_q_.pop_front();
      return ack;
    }
    _ack_cb = cb;
    return TuplePtr();
  } 

  // Need this to go through regular pull
  return this->Element::pull(port, cb); 	
}

/**
 * Port 1 deals with Flow control
 */
int RateCCR::push(int port, TuplePtr tp, b_cbv cb)
{
  if (port == 1) {
  /*
    try {
      if (Val_Str::cast((*tp)[0]) == "RRATE")
    }
    catch (Value::TypeError e) {
      log(LoggerI::WARN, 0, "CCR::push TypeError Thrown on port 1"); 
    } 
  */
    return 1;
  }

  // Need this to go through regular push
  return this->Element::push(port, tp, cb); 	
}

REMOVABLE_INLINE TuplePtr RateCCR::strip(TuplePtr p) {
  TuplePtr tuple = Tuple::mk();
  for (uint i = 0; i < p->size(); i++) {
    try {
      TuplePtr t = Val_Tuple::cast((*p)[i]); 
      if (Val_Str::cast((*t)[0]) == "TINFO") continue;
    }
    catch (Value::TypeError e) { } 
    tuple->append((*p)[i]);
  }
  tuple->freeze();
  return tuple;
}
