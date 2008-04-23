package lang.ast;

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
	
}
