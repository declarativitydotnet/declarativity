package p2.types.operator;

import p2.lang.plan.Predicate;
import p2.lang.plan.Variable;
import p2.types.basic.Intermediate;
import p2.types.basic.Schema;
import p2.types.basic.Tuple;
import p2.types.basic.TupleSet;
import p2.types.table.Table;

import java.util.Set;

public class ScanJoin extends Join {
	
	private Table table;
	
	public ScanJoin(Predicate predicate) {
		super(predicate);
		this.table = Table.table(predicate.name());
	}
	
	@Override
	public String toString() {
		return "NEST LOOP JOIN: PREDICATE[" + this.predicate  + "]";
	}
	
	@Override
	public TupleSet evaluate(TupleSet tuples) {
		TupleSet result = new TupleSet(tuples.name() + 
				                       " JOIN " + 
				                       predicate.name());
		for (Tuple outer : tuples) {
			for (Tuple inner : this.table) {
				Tuple join = outer.join(inner);
				if (join != null) {
					result.add(join);
				}
			}
		}
		return result;
	}


}
