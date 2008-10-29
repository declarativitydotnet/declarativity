package jol.lang.plan;

import java.util.HashSet;
import java.util.Set;

import jol.types.basic.ValueList;

public class Limit extends Aggregate {
	private Number kConst;
	
	private Variable kVar;

	public Limit(String name, String function, Number kConst) {
		super(name, function, ValueList.class);
		this.kConst = kConst;
		this.kVar = null;
	}
	
	public Limit(String name, String function, Variable kVar) {
		super(name, function, ValueList.class);
		this.kConst = null;
		this.kVar = kVar;
	}
	
	public Number kConst() {
		return this.kConst;
	}
	
	public Variable kVar() {
		return this.kVar;
	}
}
