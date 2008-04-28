package p2.lang.plan;

import p2.types.table.Key;
import p2.types.table.Schema;

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

	@Override
	public String toString() {
		return "define (" + name + ", " + size + ", " + lifetime + 
		       ", keys(" + key + "), {" + schema + "}).";
	}
}
