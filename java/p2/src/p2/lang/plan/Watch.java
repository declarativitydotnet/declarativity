package p2.lang.plan;

import p2.types.basic.Tuple;
import p2.types.basic.TupleSet;
import p2.types.basic.TypeList;
import p2.types.exception.UpdateException;
import p2.types.table.HashIndex;
import p2.types.table.Index;
import p2.types.table.Key;
import p2.types.table.ObjectTable;
import p2.types.table.TableName;

public class Watch extends Clause {
	
	public static class WatchTable extends ObjectTable {
		public static final Key PRIMARY_KEY = new Key();

		public enum Field {PROGRAM, SCOPE, TUPLENAME, MODIFIER, OBJECT};
		public static final Class[] SCHEMA =  {
			String.class,    // Program name
			TableName.class, // Table name
			String.class,    // Modifier
			Watch.class      // Object
		};

		public WatchTable() {
			super(new TableName(GLOBALSCOPE, "watch"), PRIMARY_KEY, new TypeList(SCHEMA));
			Key programKey = new Key(Field.PROGRAM.ordinal());
			Index index = new HashIndex(this, programKey, Index.Type.SECONDARY);
			this.secondary.put(programKey, index);
		}

		@Override
		protected boolean insert(Tuple tuple) throws UpdateException {
			String program = (String) tuple.value(Field.PROGRAM.ordinal());
			Watch object = (Watch) tuple.value(Field.OBJECT.ordinal());
			object.program = program;
			return super.insert(tuple);
		}

		@Override
		protected boolean delete(Tuple tuple) throws UpdateException {
			return super.delete(tuple);
		}
	}
	
	private String program;
	
	private TableName name;
	
	private String modifier;

	public Watch(xtc.tree.Location location, TableName name, String modifier) {
		super(location);
		this.name = name;
		this.modifier = modifier;
	}
	
	public String toString() {
		return "watch(" + name + ", " + modifier + ").";
	}

	@Override
	public void set(String program) throws UpdateException {
		Program.watch.force(new Tuple(program, name, modifier, this));
	}
}
