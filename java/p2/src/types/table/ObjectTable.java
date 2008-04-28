package types.table;

import types.basic.Tuple;
import types.exception.UpdateException;

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
