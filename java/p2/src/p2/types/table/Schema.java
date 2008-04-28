package p2.types.table;

import java.util.ArrayList;
import java.util.List;

public class Schema implements Comparable {
	
	public static class Entry {
		private String name;
		
		private Class  type;
		
		public Entry(String name, Class type) {
			this.name = name;
			this.type = type;
		}
		
		public String toString() {
			return name + ":" + type;
		}
		
		public boolean equals(Object obj) {
			if (!(obj instanceof Entry)) return false;
			Entry other = (Entry) obj;
			return other.type() == this.type;
		}
		
		public String name() {
			return this.name;
		}
		
		public Class type() {
			return this.type;
		}
	}
	
	private List<Entry> schema;

	public Schema(Entry... schema) {
		this.schema = new ArrayList<Entry>();
		for (Entry e : schema) {
			this.schema.add(e);
		}
	}
	
	public Schema(List<Class> types) {
		this.schema = new ArrayList<Entry>();
		int index = 0;
		for (Class c : types) {
			this.schema.add(new Entry(Integer.toString(index++), c));
		}
	}
	
	@Override
	public String toString() {
		if (schema.size() == 0) return "";
		
		String value = this.schema.get(0).toString();
		for (int i = 1; i < this.schema.size(); i++) {
			value += ", " + this.schema.get(i).toString();
		}
		return value;
	}
	
	
	@Override
	public boolean equals(Object obj) {
		if (!(obj instanceof Schema)) return false;
		
		Schema other = (Schema) obj;
		if (other.schema.size() == schema.size()) {
			for (int i = 0; i < schema.size(); i++) {
				if (!schema.get(i).equals(other.schema.get(i))) {
					return false;
				}
			}
			return true;
		}
		return false;
	}
	
	public List<Class> types() {
		List<Class> types = new ArrayList<Class>();
		for (Entry attribute : schema) {
			types.add(attribute.type());
		}
		return types;
	}
	
	public void attribute(int i, Entry e) {
		this.schema.set(i, e);
	}
	
	public int field(String name) {
		for (int i=0; i < schema.size(); i++) {
			if (schema.get(i).name().equals(name)) {
				return i;
			}
		}
		return -1;
	}

	public int compareTo(Object o) {
		// TODO Auto-generated method stub
		return 0;
	}
}
