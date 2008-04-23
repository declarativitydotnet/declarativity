package core;

import java.util.Hashtable;

import types.basic.Tuple;
import types.exception.UpdateException;
import types.table.Index;
import types.table.Key;
import types.table.ObjectTable;
import types.table.Schema;

public class IndexTable extends ObjectTable {
	private static final Integer[] PRIMARY_KEY = {1,2};
	
	private static final Schema schema = 
		new Schema(new Schema.Entry("Name",     String.class),
				   new Schema.Entry("Key",      Integer[].class),
				   new Schema.Entry("Type",     Index.Type.class),
				   new Schema.Entry("Method",   Index.Method.class));
	
	public Hashtable<String, Index> indices;

	public IndexTable() {
		super(IndexTable.class.getName(), schema, new Key(PRIMARY_KEY));
		indices = new Hashtable<String, Index>();
	}

	@Override
	public Tuple insert(Tuple tuple) throws UpdateException {
		// TODO Auto-generated method stub
		return super.insert(tuple);
	}

	@Override
	public boolean remove(Tuple tuple) throws UpdateException {
		// TODO Auto-generated method stub
		return super.remove(tuple);
	}

}
