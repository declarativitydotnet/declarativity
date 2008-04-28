package p2.lang.plan;

public class Null extends Value {
	public static Null NULLV = new Null(null);
	
	public Null(Object value) {
		super(null);
	}
	
	@Override
	public String toString() {
		return "null";
	}
}
