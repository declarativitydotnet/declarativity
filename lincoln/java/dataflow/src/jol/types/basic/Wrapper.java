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
	
	@Override
	public int hashCode() {
		return this.object == null ? "null".hashCode() :
			this.object.hashCode();
	}
	
	@Override
	public boolean equals(Object o) {
		if (o instanceof Wrapper) {
			Object other = ((Wrapper)o).object;
			if (this.object == other) return true;
			else if (this.object != null && other != null) {
				return object.equals(((Wrapper)o).object);
			}
		}
		return false;
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
