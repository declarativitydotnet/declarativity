package types.table;

import types.basic.Tuple;
import types.exception.UpdateException;

public abstract class ObjectTable extends BasicTable {
	
	protected ObjectTable(String name, Schema schema, Key key) {
		super(name, schema, Long.MAX_VALUE, Long.MAX_VALUE, key);
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
