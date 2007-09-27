/*
 * @(#)$Id: benchmark.C 1213 2007-03-10 05:38:10Z maniatis $
 *
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 * DESCRIPTION: Test suite for benchmarking
 *
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <iostream>
#include <cmath>

#include "tuple.h"
#include "value.h"

#include "val_null.h"
#include "val_str.h"
#include "val_int64.h"
#include "val_double.h"
#include "val_time.h"

#include "reporting.h"
#include "p2Time.h"

#include <boost/bind.hpp>

using namespace opr;

const int FIELD_TST_SZ=10000;
const int TUPLE_TST_SZ=10000;

static ValuePtr va[FIELD_TST_SZ];
static Value* vap[FIELD_TST_SZ];

static ValuePtr vf;

typedef boost::function<void (void)>        b_cbv;
 
static double time_fn(b_cbv cb) 
{
  boost::posix_time::ptime before_ts;
  boost::posix_time::ptime after_ts;
  boost::posix_time::time_duration tdiff;
  double elapsed;
  double past_average = 0;
  double average = 0;
  double sum;
  double deviation = 1;
  int iter;

  for (int i = 0; i < iter; i++) {
    (cb) ();
  } 

  for (iter = 1; deviation > .05; iter = iter * 2) {
    getTime(before_ts, LOOP_TIME_WALLCLOCK);

    for (int i = 0; i < iter; i++) {
      (cb) ();
    } 

    getTime(after_ts, LOOP_TIME_WALLCLOCK);

    tdiff = after_ts - before_ts; 
    // ensure we compute nanosecs (1/(10^9) sec) 
    // even if boost is compiled to lower precision 
    elapsed = tdiff.total_nanoseconds();
    // iter is the number of times we actually performed the test.  So
    // 'average' is the time taken to run the callback (which itself
    // is possibly FIELD_TST_SZ operations, for example). 
    average = elapsed / iter;
    sum = (average - past_average);
    if (sum > 0) 
      deviation = sum / average;
    else 
      deviation = -sum / average;
    past_average = average;
  } 

  iter /= 2;
  TELL_INFO << " Total num. operations: " << iter << " * " << FIELD_TST_SZ << " = " << FIELD_TST_SZ * iter;
  TELL_INFO << "\n";
  TELL_INFO << "average: " << average / 1000 << " usecs per iteration, ";
  TELL_INFO << "elapsed: " << elapsed / 1000 << " usecs per iteration\n";
  TELL_INFO << "Per op : " << average / FIELD_TST_SZ << " nsecs\n ";
  return elapsed;
}

static ValuePtr create_val_null_1() {
  return Val_Null::mk();
}

static void create_lots_val_null() {
  for (int i = 0; i < FIELD_TST_SZ; i++) {
    va[i] = Val_Null::mk();
  }
}

static void create_lots_val_null_ptr() {
  for (int i = 0; i < FIELD_TST_SZ; i++) {
    vap[i] = new Val_Null();
  }
}

static void create_lots_val_int64() {
  for (int i = 0; i < FIELD_TST_SZ; i++) {
    va[i] = Val_Int64::mk((int64_t)64);
  }
}

static void create_lots_val_int64_ptr() {
  for (int i = 0; i < FIELD_TST_SZ; i++) {
    vap[i] = new Val_Int64((int64_t)64);
  }
}

static void create_lots_val_double() {
  for (int i = 0; i < FIELD_TST_SZ; i++) {
    va[i] = Val_Double::mk(1.0);
  }
}

static void create_lots_val_double_ptr() {
  for (int i = 0; i < FIELD_TST_SZ; i++) {
    vap[i] = new Val_Double((double_t)64);
  }
}

static void create_lots_val_str() {
  for (int i = 0; i < FIELD_TST_SZ; i++) {
    va[i] = Val_Str::mk("This is a string");
  }
}

static void create_lots_val_str_ptr() {
  for (int i = 0; i < FIELD_TST_SZ; i++) {
    vap[i] = new Val_Str("This is a string");
  }
}

static void unit_test_for(Value::TypeCode t ) {
  switch (t) {
    case Value::NULLV:
      TELL_INFO << "NULLV: shared pointers:";
      time_fn(boost::bind(create_lots_val_null));
      TELL_INFO << "\n";
      TELL_INFO << "NULLV: raw pointers:";
      time_fn(boost::bind(create_lots_val_null_ptr));
      TELL_INFO << "\n";
      break;
    case Value::INT64:
      TELL_INFO << "INT64: shared pointers:";
      time_fn(boost::bind(create_lots_val_int64));
      TELL_INFO << "\n";
      TELL_INFO << "INT64: raw pointers:";
      time_fn(boost::bind(create_lots_val_int64_ptr));
      TELL_INFO << "\n";
      break;
    case Value::DOUBLE:
      TELL_INFO << "DOUBLE: shared pointers:";
      time_fn(boost::bind(create_lots_val_double));
      TELL_INFO << "\n";
      TELL_INFO << "DOUBLE: raw pointers:";
      time_fn(boost::bind(create_lots_val_double_ptr));
      TELL_INFO << "\n";
      break;
    case Value::STR:
      TELL_INFO << "STR: shared pointers:";
      time_fn(boost::bind(create_lots_val_str));
      TELL_INFO << "\n";
      TELL_INFO << "STR: raw pointers:";
      time_fn(boost::bind(create_lots_val_str_ptr));
      TELL_INFO << "\n";
      break;
    default:
      std::cerr << "TypeCode: " << t << " not handle\n";
      break;
  }
}

int main(int argc, char **argv)
{

  Reporting::setLevel(Reporting::INFO);

  unit_test_for(Value::NULLV);
  unit_test_for(Value::INT64);
  unit_test_for(Value::DOUBLE);
  unit_test_for(Value::STR);
  return 0;
}


/*
 * End of file 
 */
