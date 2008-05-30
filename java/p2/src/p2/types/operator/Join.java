package p2.types.operator;

import java.util.ArrayList;
import java.util.Hashtable;
import java.util.List;
import java.util.Set;
import p2.lang.plan.Predicate;
import p2.lang.plan.Variable;
import p2.types.basic.Schema;
import p2.types.basic.Tuple;
import p2.types.exception.P2RuntimeException;
import p2.types.function.TupleFunction;

public abstract class Join extends Operator {
	
	private class TableField implements TupleFunction<Comparable> {
		private Class type;
		private Integer position;
		public TableField(Class type, Integer position) { 
			this.type = type;
			this.position=position; 
		}
		public Comparable evaluate(Tuple tuple) throws P2RuntimeException {
			return tuple.value(this.position);
		}
		public Class returnType() {
			return this.type;
		}
	}
	
	private class JoinFilter {
		private TupleFunction<Comparable> lhs;
		
		private TupleFunction<Comparable> rhs;

		private JoinFilter(TupleFunction<Comparable> lhs, 
				           TupleFunction<Comparable> rhs) {
			this.lhs = lhs;
			this.rhs = rhs;
		}
		
		public Boolean evaluate(Tuple outer, Tuple inner) throws P2RuntimeException {
			Comparable lvalue = null;
			Comparable rvalue = null;
			if (this.lhs instanceof TableField) {
				lvalue = this.lhs.evaluate(inner); 
			}
			else {
				lvalue = this.lhs.evaluate(outer); 
			}
			
			if (this.rhs instanceof TableField) {
				rvalue = this.rhs.evaluate(inner);
			}
			else {
				rvalue = this.rhs.evaluate(outer);
			}
			return lvalue.compareTo(rvalue) == 0;
		}
	}
	
	protected Predicate predicate;
	
	private List<JoinFilter> filters;
	
	public Join(Predicate predicate) {
		super(predicate.program(), predicate.rule());
		this.predicate = predicate;
		this.filters = filters(predicate);
	}
	
	@Override
	public Schema schema(Schema outer) {
		return outer.join(predicate.schema());
	}

	@Override
	public Set<Variable> requires() {
		return predicate.requires();
	}
	
	protected Boolean validate(Tuple outer, Tuple inner) throws P2RuntimeException {
		for (JoinFilter filter : filters) {
			if (filter.evaluate(outer, inner) == Boolean.FALSE) {
				return false;
			}
		}
		return true;
	}
	
	private List<JoinFilter> filters(Predicate predicate) {
		List<JoinFilter> filters = new ArrayList<JoinFilter>();
		
		Hashtable<String, Variable> positions = new Hashtable<String, Variable>();
		for (p2.lang.plan.Expression arg : predicate) {
			assert(arg.position() >= 0);
			
			if (arg instanceof Variable) {
				Variable var = (Variable) arg;
				if (positions.containsKey(var.name())) {
					Variable prev = positions.get(var.name());
					filters.add(new JoinFilter(
								   new TableField(prev.type(), prev.position()), 
								   new TableField(var.type(), prev.position())));
				}
				else {
					positions.put(var.name(), var);
				}
			}
			else {
				filters.add(new JoinFilter(
						    new TableField(arg.type(), 
						    		       arg.position()), 
						    		       arg.function()));
			}
		}
		
		return filters;
	}
}
