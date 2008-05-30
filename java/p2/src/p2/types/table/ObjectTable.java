package p2.types.table;

import p2.types.basic.Schema;
import p2.types.basic.Tuple;
import p2.types.basic.TypeList;
import p2.types.exception.UpdateException;

public abstract class ObjectTable extends BasicTable {
	
	protected ObjectTable(String name, Key key, TypeList types) {
		super(name, Table.INFINITY, Table.INFINITY, key, types);
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
