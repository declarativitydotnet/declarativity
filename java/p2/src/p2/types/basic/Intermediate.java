package p2.types.basic;

import java.util.HashSet;
import java.util.Set;

import p2.types.operator.Operator;
import p2.types.table.Schema;

public abstract class Intermediate {
	
	private Set<Operator> operators;
	
	protected Intermediate() {
		this.operators = new HashSet<Operator>();
	}
	
	public boolean contains(Operator operator) {
		return this.operators.contains(operator);
	}
	
	public boolean add(Operator operator) {
		return this.operators.add(operator);
	}

	public boolean addAll(Set<Operator> operators) {
		return this.operators.addAll(operators);
	}
	
	public abstract Schema schema();
}
