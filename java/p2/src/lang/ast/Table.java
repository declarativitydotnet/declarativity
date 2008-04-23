package lang.ast;

import types.table.Key;
import types.table.Schema;

public class Table extends Clause {
	
	private Value name;
	
	private Value size;
	
	private Value lifetime;
	
	private Key key;
	
	private Schema schema;
	
	public Table(Value name, Value size, Value lifetime, Key key, Schema schema) {
		this.name = name;
		this.size = size;
		this.lifetime = lifetime;
		this.key = key;
		this.schema = schema;
	}

}
