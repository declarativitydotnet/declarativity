package p2.types.basic;

import java.util.Comparator;
import java.util.List;
import java.util.Set;
import java.util.TreeSet;

import p2.lang.plan.Variable;
import p2.types.table.Schema;


public class Ordered extends TupleSet {

	public static class CompareAttribute implements Comparator<Tuple> {
		private List<Variable> attributes;
		
		public CompareAttribute(List<Variable> attributes) {
			this.attributes = attributes;
		}

		public int compare(Tuple t1, Tuple t2) {
			return t1.compareTo(t2);
		}

	}
	
	private List<Variable> sort;
	
	public Ordered(String name, Schema schema, List<Variable> sort) {
		super(name, schema, new TreeSet<Tuple>(new CompareAttribute(sort)));
		this.sort = sort;
	}
	
	public List<Variable> sort() {
		return sort();
	}
}
