package jol.types.table;

import jol.core.Runtime;
import jol.types.basic.Tuple;
import jol.types.basic.TypeList;
import jol.types.exception.UpdateException;

public abstract class ObjectTable extends RefTable {
	
	protected ObjectTable(Runtime context, TableName name, Key key, TypeList types) {
		super(context, name, key, types);
	}

	@Override
	protected boolean insert(Tuple tuple) throws UpdateException {
		return super.insert(tuple);
	}
	
	@Override
	protected boolean delete(Tuple tuple) throws UpdateException {
		return super.delete(tuple);
	}

}
