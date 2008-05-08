package p2.types.operator;

import java.util.ArrayList;
import java.util.List;
import java.util.Set;

import p2.lang.plan.Predicate;
import p2.lang.plan.Variable;
import p2.types.basic.Intermediate;
import p2.types.basic.Schema;
import p2.types.basic.Tuple;
import p2.types.basic.TupleSet;
import p2.types.table.Index;
import p2.types.table.Key;

public class IndexJoin extends Operator {
	
	private Predicate predicate;
	
	private Index index;
	
	private List<String> names;
	
	public IndexJoin(String ID, Predicate predicate, Index index) {
		super(ID);
		this.predicate = predicate;
		this.index = index;
		this.names = new ArrayList<String>();
		List<Variable> variables = predicate.schema().variables();
		for (Integer i : index.key()) {
			this.names.add(variables.get(i).name());
		}
		
	}
	
	@Override
	public TupleSet evaluate(TupleSet tuples) {
		TupleSet result = new TupleSet("(" + tuples.name() + " join " + predicate.name() + ")");
		for (Tuple outer : tuples) {
			Key key = new Key();
			for (String name : this.names) {
				int position = outer.schema().position(name);
				if (position < 0) {
					// TODO throw runtime exception
				}
				key.add(position);
			}
			Key.Value value = key.value(outer);
			TupleSet inner = this.index.lookup(value);
			for (Tuple i : inner) {
				Tuple join = outer.join(i);
				if (join != null) {
					result.add(join);
				}
			}
		}
		return result;
	}

	@Override
	public Set<Variable> requires() {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Schema schema(Schema outer) {
		return outer.join(predicate.schema());
	}

}
