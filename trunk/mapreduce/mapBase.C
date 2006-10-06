#include "mapBase.h"
#include "loggerI.h"
#include "val_str.h"

MapBase::MapBase(string name, int in, int out) : Element(name, in, out)
{
	log(LoggerI::INFO,-1,"YAY! I am live! -- from Map Stage: "+name);
}

TuplePtr	MapBase::simple_action(TuplePtr p)
{
	if( p->size() ==0 ){
                log(LoggerI::WARN, -1, "Input tuple has no first field!!");
                return TuplePtr();
        }

	return map(Val_Str::cast((*p)[0]), (*p)[1]);
}
