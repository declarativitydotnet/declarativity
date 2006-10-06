/**
	$Id$
*/


#ifndef _MERGE_SORT_H_
#define _MERGE_SORT_H_

#include "element.h"
#include "mapBase.h"
#include <string.h>


class MergeSort : public MapBase {
public:
	MergeSort(string name);
	~MergeSort();

	const char *class_name() const                { return "MergeSort";}
	const char *processing() const                { return "a/a"; }
	const char *flow_code() const                 { return "x/x"; }

	TuplePtr	map(string key, ValuePtr val);
};

#endif
