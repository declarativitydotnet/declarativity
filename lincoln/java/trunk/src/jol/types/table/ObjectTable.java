package jol.types.table;

import jol.core.Runtime;
import jol.types.basic.Tuple;
import jol.types.basic.TypeList;
import jol.types.exception.UpdateException;

/**
 * An ObjectTable is an abstract class extended by those tables created
 * in Java. Creating such tables allow for application specific insertion/deletion
 * semantics.
 */
public abstract class ObjectTable extends BasicTable {
	
	/**
	 * Create a new ObjectTable.
	 * @param context The runtime context.
	 * @param name The table name.
	 * @param key The primary key.
	 * @param types The ordered list of attribute types.
	 */
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
