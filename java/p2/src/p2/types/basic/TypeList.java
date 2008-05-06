package p2.types.basic;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;
import java.util.ListIterator;

public class TypeList extends ArrayList<Class> implements Comparable {
	
	public TypeList() {
		
	}
	
	public TypeList(Class[] types) {
		for (Class type : types) {
			add(type);
		}
	}

	public int compareTo(Object o) {
		if (o instanceof TypeList) {
			TypeList other = (TypeList) o;
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
		return toString().compareTo(o.toString());
	}


}
