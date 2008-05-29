package p2.types.basic;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;
import java.util.ListIterator;

public class TypeList extends ArrayList<Class> implements Comparable<TypeList> {
	
	public TypeList(Class[] types) {
		for (Class type : types) {
			add(type);
		}
	}
	
	public TypeList(List<Class> types) {
		addAll(types);
	}
	
	public TypeList() {}
	
	public boolean equals(Object o) {
		if (o instanceof TypeList) {
			return ((TypeList)o).compareTo(this) == 0;
		}
		return false;
	}

	public int compareTo(TypeList other) {
		if (size() < other.size()) {
			return -1;
		}
		else if (other.size() < size()) {
			return 1;
		}
		else {
			for (int i = 0; i < size(); i++) {
				if (get(i).hashCode() < other.get(i).hashCode()) {
					return -1;
				}
				else if (get(i).hashCode() > other.get(i).hashCode()) {
					return 1;
				}
			}
			return 0;
		}
	}
}
