#include "mergeSort.h"
#include "val_list.h"
#include "list.h"


MergeSort::MergeSort(string name) : MapBase(name, 1, 1){
}

MergeSort::~MergeSort(){
}

TuplePtr	MergeSort::map(string key, ValuePtr val)
{
	ListPtr l = Val_List::cast( val );

	//do the sorting
	l->sort();
	//pack it up and ship it!
	TuplePtr t( new Tuple());
	t->append( Val_List::mk(l) );
	return t;
}
