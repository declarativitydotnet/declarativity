package p2.lang.plan;

import java.util.List;

public class Fact extends Clause {
	
	public String name;
	
	public List<Value> arguments;
	
	public Fact(String name, List<Value> arguments) {
		this.name = name;
		this.arguments = arguments;
	}
	
	public String toString() {
		String value = name + "(";
		if (arguments.size() == 0)
			return value + ")";
		value += arguments.get(0).toString();
		for (int i = 1; i < arguments.size(); i++) {
			value += ", "  + arguments.get(i);
		}
		return value + ").";
	}

}
