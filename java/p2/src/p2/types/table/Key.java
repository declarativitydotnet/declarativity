package p2.types.table;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.Vector;

import p2.types.basic.Tuple;
import p2.types.exception.BadKeyException;


public class Key implements Comparable<Key>, Iterable<Integer> {
	
	public static class Value {
		private Vector<Comparable> values;
		public Value(Vector<Comparable> values) {
			this.values = values;
		}
		
		@Override
		public boolean equals(Object other) {
			return toString().equals(other.toString());
		}
		
		@Override
		public int hashCode() {
			return toString().hashCode();
		}
		
		public String toString() {
			return this.values.toString();
		}
	}

	private List<Integer> attributes;

	public Key(Integer... attr) {
		this.attributes = new ArrayList<Integer>();
		for (Integer i : attr) {
			this.attributes.add(i);
		}
	}
	
	@Override
	public String toString() {
		if (attributes.size() == 0) return "None";
		String value = this.attributes.get(0).toString();
		for (int i = 1; i < attributes.size(); i++) {
			value += ", " + attributes.get(i).toString();
		}
		return value;
	}
	
	public void add(Integer field) {
		this.attributes.add(field);
	}
	
	public Value value(Tuple tuple) {
		Vector<Comparable> values = new Vector<Comparable>();
		if (empty()) {
			values.add("TupleID:" + tuple.id());
		}
		else {
			for (Integer attr : attributes) {
				values.add(tuple.value(attr));
			}
		}
		return new Value(values);
	}
	
	public Value value(Comparable...values) throws BadKeyException {
		if (values.length != attributes.size()) {
			throw new BadKeyException("Key value mismatch!");
		}
		Vector<Comparable> keyValues = new Vector<Comparable>();
		for (Comparable c : values) {
			keyValues.add(c);
		}
		return new Value(keyValues);
	}
	
	public boolean empty() {
		return attributes.size() == 0;
	}
	
	public int compareTo(Key o) {
		if (attributes.size() < o.attributes.size()) {
			return -1;
		}
		else if (attributes.size() > o.attributes.size()) {
			return 1;
		}
		else {
			for (int i = 0; i < attributes.size(); i++) {
				int value = this.attributes.get(i).compareTo(o.attributes.get(i));
				if (value != 0) return value;
			}
			return 0;
		}
	}

	public Iterator<Integer> iterator() {
		return attributes.iterator();
	}
}
