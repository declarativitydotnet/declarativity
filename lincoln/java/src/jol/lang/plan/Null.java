package jol.lang.plan;

public class Null extends Value<Object> {
	
	public Null() {
		super(null);
	}
	
	@Override
	public String toString() {
		return "null";
	}
}
