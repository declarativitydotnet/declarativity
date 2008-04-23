package types.function;

import types.basic.Function;
import types.basic.Tuple;

public class Identity implements Function<Comparable> {

	private Comparable obj;
	
	public Identity(Comparable obj) {
		this.obj = obj;
	}
	
	public Comparable eval(Tuple tuple) {
		return this.obj;
	}

}
