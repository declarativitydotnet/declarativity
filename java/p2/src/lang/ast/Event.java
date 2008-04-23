package lang.ast;

import types.table.Schema;

public class Event extends Clause {
	
	private Value name;
	
	private Schema schema;
	
	public Event(Value name, Schema schema) {
		this.name = name;
		this.schema = schema;
	}

}
