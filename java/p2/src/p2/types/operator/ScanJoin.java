package p2.types.operator;

import p2.lang.plan.Predicate;
import p2.lang.plan.Variable;
import p2.types.basic.Intermediate;
import p2.types.basic.Schema;
import p2.types.basic.Tuple;
import p2.types.basic.TupleSet;
import p2.types.table.Table;

import java.util.Set;

public class ScanJoin extends Operator {
	
	private Predicate predicate;
	
	private Table table;
	
	public ScanJoin(String ID, Predicate predicate) {
		super(ID);
		this.predicate = predicate;
		this.table = Table.table(predicate.name());
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

	@Override
	public Schema schema(Schema input) {
		return input.join(predicate.schema());
	}

	@Override
	public Set<Variable> requires() {
		return predicate.requires();
	}

}
