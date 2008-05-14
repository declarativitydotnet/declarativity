package p2.exec;

import java.util.ArrayList;
import java.util.List;

import p2.core.Program;
import p2.lang.plan.Rule;
import p2.types.basic.Tuple;
import p2.types.basic.TupleSet;
import p2.types.basic.TypeList;
import p2.types.exception.UpdateException;
import p2.types.table.Key;
import p2.types.table.ObjectTable;

public class Planner extends ObjectTable {
	
	public static final Key PRIMARY_KEY = new Key(0);
	
	public enum Field{NAME, OBJECT};
	public static final Class[] SCHEMA = {
		String.class,  // Program name
		Program.class    // The program object
	};

	public Planner() {
		super("planner", PRIMARY_KEY, new TypeList(SCHEMA));
	}
	
	@Override
	protected boolean insert(Tuple tuple) throws UpdateException {
		Program program = (Program) tuple.value(Field.OBJECT.ordinal());
		program.plan();
		return super.insert(tuple);
	}

}
