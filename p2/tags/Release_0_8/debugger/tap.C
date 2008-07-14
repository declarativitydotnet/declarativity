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

#include "tap.h"

Tap::Tap(string ruleName, int ruleNum)
  : Element(ruleName, 1, 2),
    _ruleNum(ruleNum)
{

}


Tap::~Tap()
{
}

TuplePtr Tap::simple_action(TuplePtr p)
{
  if(output(1)->push(p, NULL) == 0){
    std::cout << name() << " Blocking on port 1, should never "
	      << "happen since ruleTracer always accepts\n";
    std::exit(-1);
  }
  return p;
}


