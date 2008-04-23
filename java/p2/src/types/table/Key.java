package types.table;

import java.util.Vector;

import types.basic.Tuple;
import types.exception.BadKeyException;

public class Key implements Comparable {
	public class Value {
		private Vector<Comparable> values;
		public Value(Vector<Comparable> values) {
			this.values = values;
		}
		
		public int hashCode() {
			String code = "";
			for (Comparable v : values) {
				code += v.toString();
			}
			return code.hashCode();
		}
	}

	private Integer[] attributes;

	public Key(Integer[] attributes) {
		this.attributes = attributes;
	}
	
	public Value value(Tuple tuple) {
		Vector<Comparable> values = new Vector<Comparable>();
		for (Integer attr : attributes) {
			values.add(tuple.value(attr));
		}
		return new Value(values);
	}
	
	public Value value(Comparable...values) throws BadKeyException {
		if (values.length != attributes.length) {
			throw new BadKeyException("Key value mismatch!");
		}
		Vector<Comparable> keyValues = new Vector<Comparable>();
		for (Comparable c : values) {
			keyValues.add(c);
		}
		return new Value(keyValues);
	}
	
	public Integer[] attributes() {
		return attributes;
	}

	public int compareTo(Object o) {
		// TODO Auto-generated method stub
		return 0;
	}
}
