package p2.types.basic;

import java.util.List;

public class TypeList extends ValueList<Class> {
	
	public TypeList() {
		super();
	}
	
	public TypeList(Class[] values) {
		for (Class value : values) {
			add(value);
		}
	}
	
	public TypeList(List<Class> values) {
		addAll(values);
	}
	
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
