package p2.types.table;

import p2.types.basic.Schema;
import p2.types.basic.Tuple;
import p2.types.basic.TypeList;
import p2.types.exception.UpdateException;

public abstract class ObjectTable extends RefTable {
	
	protected ObjectTable(String name, Key key, TypeList types) {
		super(name, key, types);
	}

	@Override
	protected Tuple insert(Tuple tuple) throws UpdateException {
		return super.insert(tuple);
	}
	
	@Override
	protected boolean remove(Tuple tuple) throws UpdateException {
		return super.remove(tuple);
	}

}
