/*
 * @(#)$Id$
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

#include "val_time.h"

#include "val_int64.h"
#include "val_double.h"
#include "val_str.h"
#include "val_null.h"
#include "val_tuple.h"
#include "math.h"
#include "tuple.h"

using namespace opr;
using namespace boost::gregorian;
using namespace boost::posix_time;

class OperTime : public opr::OperCompare<Val_Time> {
  // removed _plus from here since it doesn't make sense to 
  // add two absolute calendar times (i.e. boost::ptime's) -- JMH 
  
  virtual ValuePtr _minus (const ValuePtr& v1, const ValuePtr& v2) const {
    // This is slightly confusing, but based on Boost's time
    // semantics.
    // Given two Val_Time values, their difference is a
    // Val_Time_Duration.
    // But the difference between a Val_Time and another numeric type
    // is of type Val_Time.
    if (v2->typeCode() == Value::TIME) {
      return Val_Time_Duration::mk(Val_Time::cast(v1) -
                                   Val_Time::cast(v2));
    } else {
      return Val_Time::mk(Val_Time::cast(v1) -
                          Val_Time_Duration::cast(v2));
    }
  };
};
const opr::Oper* Val_Time::oper_ = new OperTime();

string Val_Time::toConfString() const
{
  ostringstream conf;
  conf << "Val_Time(" << to_simple_string(t)<< ")";
  return conf.str();
}

//
// Marshalling and unmarshallng
//

void Val_Time::xdr_marshal_subtype( XDR *x )
{
  
  ptime epoch(date(1970,Jan,1));
  
  assert(time_duration::num_fractional_digits() <= 9);

  time_duration d = t - epoch;
  
  uint32_t hours = d.hours();
  uint32_t mins = d.minutes();
  uint32_t secs = d.seconds();
  uint32_t frac_secs = d.fractional_seconds() * PTIME_SECS_FACTOR;
  
  xdr_uint32_t(x, &hours);
  xdr_uint32_t(x, &mins);
  xdr_uint32_t(x, &secs);
  xdr_uint32_t(x, &frac_secs);
}

ValuePtr Val_Time::xdr_unmarshal( XDR *x )
{
  uint32_t hours;
  uint32_t mins;
  uint32_t secs;
  uint32_t frac_secs;
  xdr_uint32_t(x, &hours);
  xdr_uint32_t(x, &mins);
  xdr_uint32_t(x, &secs);
  xdr_uint32_t(x, &frac_secs);
  
  ptime epoch(date(1970,Jan,1));
  time_duration td(hours, mins, secs, frac_secs);
  
  ptime theTime = epoch + td;
  
  return mk(theTime);
}

double Val_Time::_theDouble = 0;

//
// Construction from a timespec object
//

Val_Time::Val_Time(struct timespec theTime) {
  ptime epoch(date(1970,Jan,1));
  time_duration td(0,0,theTime.tv_sec, 
                   (uint32_t) round(theTime.tv_nsec / PTIME_SECS_FACTOR));
  t = epoch + td;
}

/*
 * Casting
 * 
 * All integer casts will deal with conversion from UNIX timestamps (seconds 
 * since 1/1/70 UTC) to boost ptime objects. 
 * 
 * When dealing with negative integers, we'll assume that the user is 
 * requesting to generate a time before the UNIX epoch; since the time space 
 * for ptime extends from 1400 to 9999, we might as well allow for values 
 * older than 1970.
 * 
 * --ACR
 */

boost::posix_time::ptime Val_Time::cast(ValuePtr v) {
  ptime epoch(date(1970, Jan, 1));
	
  // Setup the time duration from epoch according to input.
  switch (v->typeCode()) {
  case Value::TIME:
    {
      return (static_cast<Val_Time *>(v.get()))->t;
    }
  case Value::NULLV:
    {
      ptime t;
      return t;
    }
  case Value::INT64:
    {
      time_duration elapsed(0,0,Val_Int64::cast(v), 0);
      ptime pt = epoch + elapsed;
      return pt;
    }
  case Value::DOUBLE:
    {
      double d = Val_Double::cast(v);
      uint32_t secs = (uint32_t) trunc(d);
      // ensure we interpret this fractional part appropriately
      // regardless of how much precision boost is compiled for
      uint32_t frac_secs =
        (uint32_t) round(modf(d, &Val_Time::_theDouble)  
                         * PTIME_FRACTIONAL_FACTOR);
			
      time_duration elapsed(0, 0, secs, frac_secs);       
      ptime pt = epoch + elapsed;
      return pt;
    }
  case Value::STR:
    {
      /*
       * ptimes take two different kinds of strings. They take the 
       * following form: 
       * yyyy-mm-dd hr:mn:sc.mil
       * Where hr is from 0 to 23 and mil is a number of milliseconds.
       * 
       * Alternatively, a 'non-delimited iso form string' will be 
       * accepted. Its form is as follows:
       * 
       * yyyymmddThrmnsecmil
       *
       * Observe that this is identical to the above form, save 
       * for the fact that its only delimiter is the T between the date 
       * and the time.
       * 
       * For now, I'll assume that this string is of one of those types.
       * 
       * TODO: Check form of the strings with regexps.
       */
			 
      ptime pt(time_from_string(Val_Str::cast(v)));	
      return pt;
    }
  default:
    throw Value::TypeError(v->typeCode(),
                           v->typeName(),
                           Value::TIME,
                           "time");
  }
	
}

int Val_Time::compareTo(ValuePtr other) const
{
  if (other->typeCode() != Value::TIME) {
    if (Value::TIME < other->typeCode()) {
      return -1;
    } else if (Value::TIME > other->typeCode()) {
      return 1;
    }
  }
  if (t < cast(other)) {
    return -1;
  } else if (t > cast(other)) {
    return 1;
  } else {
    return 0;
  }
}






////////////////////////////////////////////////////////////
// Time Duration
// XXX Move to its own file
////////////////////////////////////////////////////////////



















class OperTime_Duration : public opr::OperCompare<Val_Time_Duration> {
  virtual ValuePtr _plus (const ValuePtr& v1, const ValuePtr& v2) const {
    boost::posix_time::time_duration t1 = Val_Time_Duration::cast(v1);
    boost::posix_time::time_duration t2 = Val_Time_Duration::cast(v2);
    return Val_Time_Duration::mk(t1 + t2);
  };
 
  virtual ValuePtr _minus (const ValuePtr& v1, const ValuePtr& v2) const {
    boost::posix_time::time_duration t1 = Val_Time_Duration::cast(v1);
    boost::posix_time::time_duration t2 = Val_Time_Duration::cast(v2);
    return Val_Time_Duration::mk(t1 - t2);
  };
};
const opr::Oper* Val_Time_Duration::oper_ = new OperTime_Duration();

string Val_Time_Duration::toConfString() const
{
  ostringstream conf;
  conf << "Val_Time(" << to_simple_string(td)<< ")";
  return conf.str();
}

//
// Marshalling and unmarshallng
//
void Val_Time_Duration::xdr_marshal_subtype( XDR *x )
{
  // ensure we send nanosecs (1/(10^9) sec) 
  // even if boost is compiled to lower precision 
  assert(time_duration::num_fractional_digits() <= 9);
  uint32_t hours = td.hours();
  uint32_t mins = td.minutes();
  uint32_t secs = td.seconds();
  uint32_t frac_secs = td.fractional_seconds() * PTIME_SECS_FACTOR;
  xdr_uint32_t(x, &hours);
  xdr_uint32_t(x, &mins);
  xdr_uint32_t(x, &secs);
  xdr_uint32_t(x, &frac_secs);
}

ValuePtr Val_Time_Duration::xdr_unmarshal( XDR *x )
{
  uint32_t hours;
  uint32_t mins;
  uint32_t secs;
  uint32_t frac_secs;
  
  xdr_uint32_t(x, &hours);
  xdr_uint32_t(x, &mins);
  xdr_uint32_t(x, &secs);
  xdr_uint32_t(x, &frac_secs);

  // ensure we interpret this as nanosecs (1/(10^9) sec) 
  // even if boost is compiled to lower precision 
  boost::posix_time::time_duration td1(hours, mins, secs, 
                                       frac_secs / PTIME_SECS_FACTOR);
  
  
  
  return mk(td1);
}

double Val_Time_Duration::_theDouble = 0;

//
// Casting
//
boost::posix_time::time_duration Val_Time_Duration::cast(ValuePtr v) {
  switch (v->typeCode()) {
  case Value::TIME_DURATION:
    return (static_cast<Val_Time_Duration *>(v.get()))->td;
  case Value::INT64:
    {
      // treat the input as seconds
      boost::posix_time::time_duration td(0,0,Val_Int64::cast(v), 0);       
      return td;
    }
  case Value::DOUBLE:
    {
      // treat the input as seconds and nanoseconds
      double d = Val_Double::cast(v);
      uint32_t secs = (uint32_t)trunc(d);
      // ensure we interpret this fractional part appropriately
      // regardless of how much precision boost is compiled for
      uint32_t frac_secs = (uint32_t) round(modf(d, &Val_Time_Duration::_theDouble) 
                                            * PTIME_FRACTIONAL_FACTOR);
      time_duration td(0,0,secs,frac_secs);       
      return td;
    }
  case Value::NULLV:
    {
      time_duration td; // default constructor is 0
      return td;
    }
  case Value::TUPLE:
    {
      TuplePtr theTuple = Val_Tuple::cast(v);
      time_duration td;
      if (theTuple->size() >= 2) {
        // treat the input as seconds and nanoseconds
        uint32_t secs = Val_Int64::cast((*theTuple)[0]);
        // ensure we interpret this as nanosecs (1/(10^9) sec) 
        // even if boost is compiled to lower precision 
        uint32_t nsecs = (uint32_t) round((Val_Int64::cast((*theTuple)[1]))
                                          / PTIME_SECS_FACTOR);
        td = time_duration(0,0,secs,nsecs);
      } 
      return td;
    }
  default:
    throw Value::TypeError(v->typeCode(),
                           v->typeName(),
                           Value::TIME,
                           "time");
  }
}

int Val_Time_Duration::compareTo(ValuePtr other) const
{  
  if (other->typeCode() != Value::TIME_DURATION) {
    if (Value::TIME_DURATION < other->typeCode()) {
      return -1;
    } else if (Value::TIME_DURATION > other->typeCode()) {
      return 1;
    }
  }
  if (td < cast(other)) {
    return -1;
  } else if (td > cast(other)) {
    return 1;
  } else {
    return 0;
  }
}


/*
 * End of file
 */
