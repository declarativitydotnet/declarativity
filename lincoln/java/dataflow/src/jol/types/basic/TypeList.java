package jol.types.basic;

import java.io.Serializable;
import java.util.List;

/**
 * Simple data structure that stores an ordered list of types.
 */
public class TypeList extends ValueList<TypeList.ComparableClass> {
    private static final long serialVersionUID = 1L;
    
    private static class ComparableClass implements Comparable<ComparableClass>, Serializable {
    	public Class c;
    	
    	public ComparableClass(Class c) {
    		this.c = c;
    	}

		public int compareTo(ComparableClass o) {
			return this.c.getCanonicalName().compareTo(o.c.getCanonicalName());
		}
    }

    public TypeList() {
        super();
    }

    public TypeList(Class[] values) {
        for (Class c : values) {
            add(new ComparableClass(c));
        }
    }

    public TypeList(List<Class> values) {
        for (Class c : values) {
            add(new ComparableClass(c));
        }
    }
    
    public boolean add(Class c) {
    	return add(new ComparableClass(c));
    }
    
    public Class getClass(int index) {
    	return super.get(index).c;
    }
    
    public Class[] toArray() {
    	Class[] array = new Class[size()];
    	for (int i = 0; i < size(); i++) {
    		array[i] = getClass(i);
    	}
    	return array;
    }
    

    @Override
    public boolean equals(Object o) {
        if (o instanceof TypeList) {
            return ((TypeList) o).compareTo(this) == 0;
        }
        return false;
    }
}
