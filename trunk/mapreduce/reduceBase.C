#include "reduceBase.h"
#include "val_list.h"
#include "val_str.h"

ReduceBase::ReduceBase(string name, int in, int out) : Element(name, in, out)
{
	log(LoggerI::INFO,-1,"YAY! I am live! -- from Reduce Stage: "+name);
}

TuplePtr	ReduceBase::simple_action(TuplePtr p)
{
	if( p->size() ==0 ){
                log(LoggerI::WARN, -1, "Input tuple has no first field!!");
                return TuplePtr();
        }

	return reduce(Val_Str::cast((*p)[0]),Val_List::cast((*p)[1]));
}
