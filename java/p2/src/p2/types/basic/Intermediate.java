package p2.types.basic;

import java.util.HashSet;
import java.util.Set;

import p2.types.operator.Operator;

public class Intermediate extends TupleSet {
	
	private Set<Operator> operators;
	
	public Intermediate(Set<Tuple> tuples, Set<Operator> operators) {
		super("Intermediate", tuples);
		this.operators = operators == null ? 
				new HashSet<Operator>() : new HashSet<Operator>(operators);
	}
	
	public boolean contains(Operator operator) {
		return this.operators.contains(operator);
	}
	
	public boolean add(Operator operator) {
		return this.operators.add(operator);
	}
	
	public Set<Operator> operators() {
		return this.operators;
	}

}
