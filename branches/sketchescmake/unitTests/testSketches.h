/* 
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 */

#ifndef __TESTSKETCHES_H__
#define __TESTSKETCHES_H__

#include "boost/test/unit_test.hpp"
#include "p2core/val_sketch.h"

class testSketches_testSuite
  : public boost::unit_test_framework::test_suite
{
public:
  testSketches_testSuite();
};

#endif