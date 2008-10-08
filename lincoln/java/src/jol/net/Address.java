package jol.net;

public abstract class Address implements Comparable<Address> {
	
	@Override
	public abstract String toString();
	
	@Override
	public int hashCode() {
		return toString().hashCode();
	}
	
	@Override
	public boolean equals(Object o) {
		if (o instanceof Address) {
			return toString().equals(((Address)o).toString());
		}
		return false;
	}
	
	public int compareTo(Address o) {
		return toString().compareTo(o.toString());
	}
}
