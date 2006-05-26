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


