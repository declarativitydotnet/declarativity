#ifndef _MAP_H_
#define _MAP_H_

#include "element.h"
#include "val_list.h"
#include <string>

class ReduceBase : public Element {
public:
	ReduceBase(string name, int in, int out);


	TuplePtr		simple_action(TuplePtr p);

protected:
	virtual TuplePtr	reduce(string key, Val_List val)=0;
};

#endif
