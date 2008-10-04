package p2.lang.plan;

import java.util.ArrayList;
import java.util.List;

import p2.types.basic.Tuple;
import p2.types.basic.TypeList;
import p2.types.exception.P2RuntimeException;
import p2.types.exception.UpdateException;
import p2.types.function.TupleFunction;
import p2.types.table.HashIndex;
import p2.types.table.Index;
import p2.types.table.Key;
import p2.types.table.ObjectTable;
import p2.types.table.TableName;
import p2.lang.Compiler;

public class Fact extends Clause {
	
	public static class FactTable extends ObjectTable {
		public static final Key PRIMARY_KEY = new Key();
		
		public enum Field {PROGRAM, TABLENAME, TUPLE};
		public static final Class[] SCHEMA =  {
			String.class,    // Program name
			TableName.class, // Table name
			Tuple.class      // Tuple object
		};

		public FactTable() {
			super(new TableName(GLOBALSCOPE, "fact"), PRIMARY_KEY, new TypeList(SCHEMA));
			Key programKey = new Key(Field.PROGRAM.ordinal());
			Index index = new HashIndex(this, programKey, Index.Type.SECONDARY);
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
	
	private String program;
	
	private TableName name;
	
	private List<Expression> arguments;
	
	public Fact(xtc.tree.Location location, TableName name, List<Expression> arguments) {
		super(location);
		this.name = name;
		this.arguments = arguments;
	}
	
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
	public void set(String program) throws UpdateException {
		List<Comparable> values = new ArrayList<Comparable>();
		for (Expression argument : this.arguments) {
			TupleFunction<Comparable> function = argument.function();
			try {
				values.add(function.evaluate(null));
			} catch (P2RuntimeException e) {
				e.printStackTrace();
				throw new UpdateException(e.toString());
			}
		}
		
		Compiler.fact.force(new Tuple(program, name, new Tuple(values)));
	}

}
