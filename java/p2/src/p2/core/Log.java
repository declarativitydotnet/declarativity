package p2.core;

import java.io.FilterOutputStream;
import java.io.IOException;

import p2.types.basic.Tuple;
import p2.types.basic.TypeList;
import p2.types.exception.UpdateException;
import p2.types.table.Key;
import p2.types.table.ObjectTable;
import p2.types.table.Table;
import p2.types.table.TableName;

public class Log extends ObjectTable {
	
	public static final Key PRIMARY_KEY = new Key();
	
	public enum Type{INFO, WARNING, ERROR};
	
	public enum Field{CLOCK, PROGRAM, RULE, TYPE, MESSAGE};
	public static final Class[] SCHEMA = {
		Long.class,    // Clock value 
		String.class,  // Program name
		String.class,  // Rule name
		Enum.class,    // Message type
		String.class   // Log message
	};

	private FilterOutputStream stream;
	
	public Log(FilterOutputStream stream) {
		super(new TableName(Table.GLOBALSCOPE, "log"), PRIMARY_KEY, new TypeList(SCHEMA));
		this.stream = stream;
	}
	
	@Override
	public boolean insert(Tuple tuple) throws UpdateException {
		String log = "CLOCK[" + tuple.value(Field.CLOCK.ordinal()) + "], ";
		log       += "PROGRAM[" + tuple.value(Field.PROGRAM.ordinal()) + "], ";
		log       += "RULE[" + tuple.value(Field.RULE.ordinal()) + "], ";
		log       += "TYPE[" + tuple.value(Field.TYPE.ordinal()) + "], ";
		log       += tuple.value(Field.MESSAGE.ordinal()) + "\n";
		
		try {
			stream.write(log.getBytes());
		} catch (IOException e) {
			throw new UpdateException(e.toString());
		}
		return super.insert(tuple);
	}

}
