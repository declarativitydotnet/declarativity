package p2.lang.plan;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import p2.types.basic.Tuple;
import p2.types.exception.RuntimeException;
import p2.types.function.TupleFunction;

public abstract class Reference extends Expression {
	
	protected Class type;
	
	protected String name;

	public Reference(Class type, String name) {
		this.type = type;
		this.name = name;
	}
	
	@Override
	public String toString() {
		return this.name;
	}

	@Override
	public Class type() {
		return this.type;
	}
	
	public abstract Expression object();
}
