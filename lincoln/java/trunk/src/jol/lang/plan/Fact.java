package jol.lang.plan;

import java.util.ArrayList;
import java.util.List;

import jol.core.Runtime;
import jol.types.basic.Tuple;
import jol.types.exception.JolRuntimeException;
import jol.types.exception.PlannerException;
import jol.types.exception.UpdateException;
import jol.types.function.TupleFunction;
import jol.types.table.HashIndex;
import jol.types.table.Index;
import jol.types.table.Key;
import jol.types.table.ObjectTable;
import jol.types.table.TableName;

public class Fact extends Clause {
	
	public static class FactTable extends ObjectTable {
		public final static TableName TABLENAME = new TableName(GLOBALSCOPE, "fact");
		public static final Key PRIMARY_KEY = new Key();
		
		public enum Field {PROGRAM, TABLENAME, TUPLE};
		public static final Class[] SCHEMA =  {
			String.class,    // Program name
			TableName.class, // Table name
			Tuple.class      // Tuple object
		};

		public FactTable(Runtime context) {
			super(context, TABLENAME, PRIMARY_KEY, SCHEMA);
			Key programKey = new Key(Field.PROGRAM.ordinal());
			Index index = new HashIndex(context, this, programKey, Index.Type.SECONDARY);
			this.secondary.put(programKey, index);
		}
		
		@Override
		protected boolean insert(Tuple tuple) throws UpdateException {
			return super.insert(tuple);
		}
		
		@Override
		protected boolean delete(Tuple tuple) throws UpdateException {
			return super.delete(tuple);
		}
	}

	private TableName name;
	
	private List<Expression> arguments;
	
	public Fact(xtc.tree.Location location, TableName name, List<Expression> arguments) {
		super(location);
		this.name = name;
		this.arguments = arguments;
	}
	
	@Override
	public String toString() {
		String value = name + "(";
		if (arguments.size() == 0)
			return value + ")";
		value += arguments.get(0).toString();
		for (int i = 1; i < arguments.size(); i++) {
			value += ", "  + arguments.get(i);
		}
		return value + ").";
	}

	@Override
	public void set(Runtime context, String program) throws UpdateException {
		try {
			List<Object> values = new ArrayList<Object>();
			for (Expression argument : this.arguments) {
				TupleFunction function = argument.function(null);
				try {
					values.add(function.evaluate(null));
				} catch (JolRuntimeException e) {
					e.printStackTrace();
					throw new UpdateException(e.toString());
				}
			}
			context.catalog().table(FactTable.TABLENAME)
				.force(new Tuple(program, name, new Tuple(values)));
		} catch (PlannerException e) {
			throw new UpdateException(e.toString());
		}
	}

}
