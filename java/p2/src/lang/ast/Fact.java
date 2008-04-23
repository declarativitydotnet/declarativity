package lang.ast;

import java.util.List;

public class Fact extends Clause {
	
	public String name;
	
	public List<Value> arguments;
	
	public Fact(String name, List<Value> arguments) {
		this.name = name;
		this.arguments = arguments;
	}

}
