/*
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 * DESCRIPTION: Element that creates and pushes a new tuple at every
 * timed interval.  The element becomes inactive if a push is rejected,
 * waking up when its callback is invoked.
 */


#ifndef __TRAFFIC_MANAGER_H__
#define __TRAFFIC_MANAGER_H__

#include <element.h>
#include <deque>

class TrafficManager : public Element { 
 public:
  
  /** Initialized with the interval between tuple generation events. */
  TrafficManager(string name, string my_addr, uint my_key, uint key_range, double seconds);

  const char *class_name() const { return "TrafficManager"; }
  const char *flow_code() const	 { return "-/--"; }
  const char *processing() const { return "h/hh"; }

  int push(int port, TuplePtr tp, b_cbv cb);

  virtual int initialize();

  void runTimer();
  
 private:
  REMOVABLE_INLINE uint genLookupKey();
  REMOVABLE_INLINE int getKey(TuplePtr tp);
  REMOVABLE_INLINE TuplePtr mkResponse(TuplePtr tp);
  REMOVABLE_INLINE bool processResponse(TuplePtr tp);
  REMOVABLE_INLINE uint32_t delay(timespec *ts);

  /** My wakeup method */
  void wakeup();

  /** The integer seconds portion of the interval */
  uint                  _seconds;
  /** The nsec portion of the interval */
  uint                  _nseconds;
  /** My wakeup callback */
  b_cbv                   _wakeupCB;
  /** Callback to my runTimer() */
  b_cbv                  _runTimerCB;
  /** My time callback ID. */
  timeCBHandle *           _timeCallback;

  string  my_addr_;
  uint my_key_;
  uint key_range_;
  uint total_received_;
};

#endif /* __TRAFFIC_MANAGER_H__ */
