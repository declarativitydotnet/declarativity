package types.basic;

import java.util.Comparator;
import java.util.Set;
import java.util.TreeSet;


public class OrderedTupleSet extends TupleSet {

	public static class CompareAttribute implements Comparator<Tuple> {
		private Integer[] attributes;
		
		public CompareAttribute(Integer[] attributes) {
			this.attributes = attributes;
		}

		public int compare(Tuple t1, Tuple t2) {
			return t1.compareTo(t2);
		}

	}
	
	public OrderedTupleSet(Integer[] sort) {
		super(new TreeSet<Tuple>(new CompareAttribute(sort)), sort);
	}
	

}
