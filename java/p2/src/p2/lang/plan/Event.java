package p2.lang.plan;

import p2.types.basic.TypeList;

public class Event extends Clause {
	
	private Value<String> name;
	
	private TypeList schema;
	
	public Event(Value<String> name, TypeList schema) {
		this.name = name;
		this.schema = schema;
	}

	public String toString() {
		return "define " + name + "(" + schema + ").";
	}
}
