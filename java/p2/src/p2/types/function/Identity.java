package p2.types.function;

import p2.types.basic.Function;
import p2.types.basic.Tuple;

public class Identity implements Function<Comparable> {

	private Comparable obj;
	
	public Identity(Comparable obj) {
		this.obj = obj;
	}
	
	public Comparable eval(Tuple tuple) {
		return this.obj;
	}

}
