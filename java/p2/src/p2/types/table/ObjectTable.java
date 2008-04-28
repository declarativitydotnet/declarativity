package p2.types.table;

import p2.types.basic.Tuple;
import p2.types.exception.UpdateException;

public abstract class ObjectTable extends RefTable {
	
	protected ObjectTable(Table.Name name, Schema schema, Key key) {
		super(name, schema, Catalog.INFINITY, Catalog.INFINITY, key);
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
