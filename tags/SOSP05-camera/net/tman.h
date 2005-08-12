/*
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 * DESCRIPTION: Element that creates and pushes a new tuple at every
 * timed interval.  The element becomes inactive if a push is rejected,
 * waking up when its callback is invoked.
 */


#ifndef __TRAFFIC_MANAGER_H__
#define __TRAFFIC_MANAGER_H__

#include <amisc.h>
#include <element.h>
#include <deque>

class TrafficManager : public Element { 
 public:
  
  /** Initialized with the interval between tuple generation events. */
  TrafficManager(str name, str my_addr, uint my_key, uint key_range, double seconds);

  const char *class_name() const { return "TrafficManager"; }
  const char *flow_code() const	 { return "-/--"; }
  const char *processing() const { return "h/hh"; }

  int push(int port, TupleRef tp, cbv cb);

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
  cbv                   _wakeupCB;
  /** Callback to my runTimer() */
  cbv                  _runTimerCB;
  /** My time callback ID. */
  timecb_t *           _timeCallback;

  str  my_addr_;
  uint my_key_;
  uint key_range_;
  uint total_received_;
};

#endif /* __TRAFFIC_MANAGER_H__ */
