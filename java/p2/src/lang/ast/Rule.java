package lang.ast;

import java.util.List;

public class Rule extends Clause {
	
	private String name;
	
	private java.lang.Boolean deletion;
	
	private Predicate head;
	
	private List<Term> body;
	
	public Rule(String name, java.lang.Boolean deletion, Predicate head, List<Term> body) {
		this.name = name;
		this.deletion = deletion;
		this.head = head;
		this.body = body;
	}
}
