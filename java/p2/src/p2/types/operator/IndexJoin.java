package p2.types.operator;

import java.util.ArrayList;
import java.util.List;

import p2.lang.plan.Predicate;
import p2.lang.plan.Variable;
import p2.types.basic.Tuple;
import p2.types.basic.TupleSet;
import p2.types.exception.BadKeyException;
import p2.types.exception.P2RuntimeException;
import p2.types.table.Index;
import p2.types.table.Key;

public class IndexJoin extends Join {
	
	private Index index;
	
	private List<String> names;
	
	public IndexJoin(Predicate predicate, Index index) {
		super(predicate);
		this.index = index;
		this.names = new ArrayList<String>();
		List<Variable> variables = predicate.schema().variables();
		for (Integer i : index.key()) {
			this.names.add(variables.get(i).name());
		}
		
	}
	
	@Override
	public String toString() {
		return "INDEX JOIN PREDICATAE[" + predicate.toString() + "]";
	}
	
	@Override
	public TupleSet evaluate(TupleSet tuples) throws P2RuntimeException {
		TupleSet result = new TupleSet();
		for (Tuple outer : tuples) {
			Key key = new Key();
			for (String name : this.names) {
				int position = outer.schema().position(name);
				if (position < 0) {
					throw new P2RuntimeException("Variable " + name + 
							" does not exist in probe tuple schema " + outer.schema());
				}
				key.add(position);
			}
			
			for (Tuple inner : this.index.lookup(key, outer)) {
				if (validate(outer, inner)) {
					Tuple join = outer.join(inner);
					if (join != null) {
						result.add(join);
					}
				}
			}
		}
		return result;
	}
}
