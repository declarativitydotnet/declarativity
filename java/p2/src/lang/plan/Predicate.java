package lang.plan;

import java.util.Iterator;
import java.util.List;

public class Predicate extends Term implements Iterable<Expression> {
	
	private String name;
	
	private List<Expression> arguments;
	
	public Predicate(String name, List<Expression> arguments) {
		this.name = name;
		this.arguments = arguments;
	}
	
	public String name() {
		return this.name;
	}

	public Iterator<Expression> iterator() {
		return this.arguments.iterator();
	}
	
	@Override
	public String toString() {
		String value = name + "(";
		if (arguments.size() == 0) {
			return value + ")";
		}
		value += arguments.get(0).toString();
		for (int i = 1; i < arguments.size(); i++) {
			value += ", " + arguments.get(i);
		}
		return value + ")";
	}
	
}
