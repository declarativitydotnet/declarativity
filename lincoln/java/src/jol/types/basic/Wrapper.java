package jol.types.basic;

public class Wrapper<T> implements Comparable<Wrapper<T>> {
	
	private T object;
	
	public Wrapper(T object) {
		this.object = object;
	}
	
	@Override
	public String toString() {
		return this.object == null ? 
				"null" : this.object.toString();
	}

	public int compareTo(Wrapper<T> o) {
		if (o == this) {
			return 0;
		}
		else if (o == null) {
			return -1;
		}
		else if (o.object == this.object ||
		         o.object.toString() == this.object.toString()) {
			return 0;
		}
		else if (o.object.toString() == null || 
				this.object.toString() == null) {
			return -1;
		}
		return this.object.toString().compareTo(o.object().toString());
	}
	
	public T object() {
		return this.object;
	}

}
