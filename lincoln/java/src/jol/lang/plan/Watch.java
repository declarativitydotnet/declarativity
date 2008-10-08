package jol.lang.plan;

import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.basic.TypeList;
import jol.types.exception.BadKeyException;
import jol.types.exception.UpdateException;
import jol.types.operator.Operator;
import jol.types.table.Key;
import jol.types.table.ObjectTable;
import jol.types.table.TableName;
import jol.lang.Compiler;
import jol.core.Runtime;

public class Watch extends Clause {
	
	public static class WatchTable extends ObjectTable {
		public final static TableName TABLENAME = new TableName(GLOBALSCOPE, "watches");
		public static final Key PRIMARY_KEY = new Key(0,1,2);
		public enum Field {PROGRAM, TABLENAME, MODIFIER, OPERATOR};
		
		private Runtime context;
		
		public static final Class[] SCHEMA =  {
			String.class,                             // Program name
			TableName.class,                          // Table name
			jol.types.operator.Watch.Modifier.class,   // Modifier
			jol.types.operator.Watch.class             // Operator
		};

		public WatchTable(Runtime context) {
			super(context, TABLENAME, PRIMARY_KEY, new TypeList(SCHEMA));
			this.context = context;
		}

		@Override
		protected boolean insert(Tuple tuple) throws UpdateException {
			return super.insert(tuple);
		}

		@Override
		protected boolean delete(Tuple tuple) throws UpdateException {
			return super.delete(tuple);
		}
		
		public Operator watched(String program, TableName name, jol.types.operator.Watch.Modifier modifier) {
			Tuple key = new Tuple(program, name, modifier);
			try {
				TupleSet tuples = primary().lookup(key);
				if (tuples.size() > 0) {
					return (Operator) tuples.iterator().next().value(Field.OPERATOR.ordinal());
				}
			} catch (BadKeyException e) {
				e.printStackTrace();
			}
			return null;
		}
	}
	
	private TableName name;
	
	private jol.types.operator.Watch.Modifier modifier;

	public Watch(xtc.tree.Location location, TableName name, jol.types.operator.Watch.Modifier modifier) {
		super(location);
		this.name = name;
		this.modifier = modifier;
	}
	
	public String toString() {
		return "watch(" + name + ", " + modifier + ").";
	}

	@Override
	public void set(Runtime context, String program) throws UpdateException {
		context.catalog().table(WatchTable.TABLENAME).force(
				new Tuple(program, name, modifier, 
						  new jol.types.operator.Watch(context, program, null, name, modifier)));
	}
}
