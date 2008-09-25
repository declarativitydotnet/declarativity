package p2.types.basic;

import java.util.ArrayList;
import java.util.List;

public class ValueList<T> 
	extends ArrayList<T> implements Comparable<ValueList> {
	
	public int compareTo(ValueList o) {
		if (o.equals(this)) {
			return 0;
		}
		else {
			return -1;
		}
	}
	
	public void insert(T o) {
		super.add(o);
	}

}
