package jol.types.table;

import java.io.IOException;

import jol.lang.plan.Aggregate;
import jol.lang.plan.Expression;
import jol.lang.plan.Predicate;
import jol.types.basic.BasicTupleSet;
import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.exception.JolRuntimeException;
import jol.types.exception.UpdateException;

public class Arg extends Function {
	public static final String NAME = "arg";

	private String function;

	private Integer position;

	public Arg(Predicate pred) throws IOException {
		super(NAME + ":" + pred.identifier(), pred.schema().types());
		this.position = 0;
		for (Expression e : pred) {
			if (e instanceof Aggregate) {
				Aggregate a = (Aggregate) e;
				if (a.functionName().equals(jol.types.function.Aggregate.MAX)) {
					this.function = jol.types.function.Aggregate.MAX;
					break;
				}
				else if (a.functionName().equals(jol.types.function.Aggregate.MIN)) {
					this.function = jol.types.function.Aggregate.MIN;
					break;
				}
				else {
					throw new IOException("Unsupported arg function " + a.functionName());
				}
			}
			this.position++;
		}
	}

	@Override
	public TupleSet insert(TupleSet tuples, TupleSet conflicts) throws UpdateException {
		try {
			Comparable best = null;
			TupleSet result = new BasicTupleSet();
			for (Tuple t : tuples) {
				if (best == null) {
					best = (Comparable) t.value(this.position);
					result.add(t);
				}
				else {
					Comparable current = (Comparable) t.value(this.position);
					if (current.equals(best)) {
						result.add(t);
					}
					else if (checkForNewBest(best, current)) {
						result.clear();
						best = current;
						result.add(t);
					}
				}
			}
			return result;
		} catch (Throwable t) {
			throw new UpdateException(t.toString());
		}
	}

	private boolean checkForNewBest(Comparable best, Comparable comp) throws JolRuntimeException {
		if (this.function.equals(jol.types.function.Aggregate.MAX)) {
			return best.compareTo(comp) < 0;
		}
		else if (this.function.equals(jol.types.function.Aggregate.MIN)) {
			return best.compareTo(comp) > 0;
		}
		throw new JolRuntimeException("Arg Table Function Fatal Error");
	}
}
