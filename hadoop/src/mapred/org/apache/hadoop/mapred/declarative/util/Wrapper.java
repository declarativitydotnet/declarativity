package org.apache.hadoop.mapred.declarative.util;

public class Wrapper<T> implements Comparable<Wrapper<T>> {
	
	private T object;
	
	public Wrapper(T object) {
		this.object = object;
	}

	public int compareTo(Wrapper<T> o) {
		return this.object.toString().compareTo(o.object().toString());
	}
	
	public T object() {
		return this.object;
	}

}
