#ifndef _MAP_H_
#define _MAP_H_

#include "element.h"
#include "value.h"
#include <string>

class MapBase : public Element {
public:
	MapBase(string n, int in, int out);

	const char *processing() const                { return "h/a"; }
        const char *flow_code() const                 { return "-/-"; }


	TuplePtr		simple_action(TuplePtr p);

protected:
	virtual TuplePtr	map(string key, ValuePtr val)=0;
};

#endif
