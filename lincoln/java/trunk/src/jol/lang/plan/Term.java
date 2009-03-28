package jol.lang.plan;

import java.util.Set;

import jol.core.Runtime;
import jol.types.basic.Schema;
import jol.types.exception.PlannerException;
import jol.types.exception.UpdateException;
import jol.types.operator.Operator;

public abstract class Term implements Comparable<Term> {
	private static long identifier = 0;
	
	private xtc.tree.Location location;
	
	private String identifer;
	
	protected String  program;
	protected String  rule;
	protected Integer position;
	
	protected Term() {
		this.program  = null;
		this.rule     = null;
		this.position = null;
		this.identifer = "tid:" + identifier++;
	}
	
	public String identifier() {
		return this.identifer;
	}
	
	public void location(xtc.tree.Location location) {
		this.location = location;
	}
	
	public xtc.tree.Location location() {
		return this.location;
	}
	
	@Override
	public boolean equals(Object o) {
		if (o instanceof Term) {
			return compareTo((Term) o) == 0;
		}
		return false;
	}
	
	public int compareTo(Term o) {
		return this.identifer.compareTo(o.identifer);
	}
	
	public String program() {
		return this.program;
	}
	
	public String rule() {
		return this.rule;
	}
	
	public Integer position() {
		return this.position;
	}
	
	@Override
	public abstract String toString();
	
	public abstract Set<Variable> requires();
	
	public abstract Schema schema(Schema input);
	
	public void set(Runtime context, String program, String rule, Integer position) throws UpdateException {
		this.program  = program;
		this.rule     = rule;
		this.position = position;
	}
	
	public abstract Operator operator(Runtime context, Schema input) throws PlannerException;

	public boolean extension() {
		return false;
	}
}
