package jol.types.table;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import jol.types.basic.Tuple;


public class Key implements Comparable<Key>, Iterable<Integer> {
	
	private List<Integer> attributes;

	public Key(Integer... attr) {
		this.attributes = new ArrayList<Integer>();
		for (Integer i : attr) {
			this.attributes.add(i);
		}
	}
	
	public Key(List<Integer> attr) {
		this.attributes = attr;
	}
	
	@Override
	public int hashCode() {
		return toString().hashCode();
	}
	
	@Override
	public boolean equals(Object o) {
		if (o instanceof Key) {
			return toString().equals(((Key)o).toString());
		}
		return false;
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
	
	public int size() {
		return this.attributes.size();
	}
	
	public void add(Integer field) {
		this.attributes.add(field);
	}
	
	public Tuple project(Tuple tuple) {
		if (empty()) {
			return tuple;
		}
		else {
			List<Comparable> values = new ArrayList<Comparable>();
			for (Integer attr : attributes) {
				values.add(tuple.value(attr));
			}

			Tuple project = new Tuple(values);
			project.id(tuple.id());
			return project;
		}
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
