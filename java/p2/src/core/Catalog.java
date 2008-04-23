package core;

import java.lang.reflect.Constructor;
import java.util.Hashtable;
import java.util.Vector;

import types.basic.Tuple;
import types.basic.TupleSet;
import types.exception.BadKeyException;
import types.exception.UpdateException;
import types.table.BasicTable;
import types.table.Key;
import types.table.ObjectTable;
import types.table.RefTable;
import types.table.Schema;
import types.table.Table;
import types.table.Table.Type;
import xtc.util.SymbolTable;

public class Catalog extends ObjectTable {
	private static final Integer[] PRIMARY_KEY = {1};
	
	private static final Schema schema = 
		new Schema(new Schema.Entry("Name",     String.class),
				   new Schema.Entry("Type",     Table.Type.class),
				   new Schema.Entry("Schema",   Schema.class),
				   new Schema.Entry("Size",     Long.class),
				   new Schema.Entry("Lifetime", Long.class),
				   new Schema.Entry("Key",      Key.class),
				   new Schema.Entry("Table",    Table.class));

	/** The system catalog. */
	private static Catalog system = null;
	
	private SymbolTable symbolTable;
	
	private Catalog() {
		super(Catalog.class.getName(), schema, new Key(PRIMARY_KEY));
		symbolTable = new SymbolTable();
	}
	
	public static void initialize() {
		if (system == null) {
			system = new Catalog();
		}
	}

	public SymbolTable symbolTable() {
		return symbolTable;
	}
	
	public static Catalog system() {
		return system;
	}
	
	public static Table create(String name, Table.Type type, Schema schema, Long size, Long lifetime, Integer[] key) 
	throws UpdateException {
		if (system == null) {
			throw new UpdateException("Catalog has not been created!");
		}
		
		Tuple tuple = new Tuple(name, type, schema, size, lifetime, new Key(key));
		Tuple table = system.insert(tuple);
		if (table != null) {
			return (Table) table.value(schema.field("Table"));
		}
		return null;
	}
	
	public static boolean drop(String name, Table.Type type) throws UpdateException {
		if (system == null) {
			throw new UpdateException("Catalog has not been created!");
		}
		
		try {
			TupleSet tuples = system.primary().lookup(system.key().value(name));
			for (Tuple table : tuples) {
				return system.remove(table);
			}
		} catch (BadKeyException e) {
			// TODO Fatal error
			e.printStackTrace();
		}
		return false;
	}
	
	public static Table table(String name) {
		try {
			TupleSet table = system.primary().lookup(system.key().value(name));
			if (table.size() == 1) {
				return (Table) table.iterator().next().value(schema.field("Table"));
			}
			else if (table.size() > 1) {
				// TODO Fatal error.
			}
		} catch (BadKeyException e) {
			// TODO Fatal error.
			e.printStackTrace();
		}
		return null;
	}
	
	@Override
	protected boolean remove(Tuple tuple) throws UpdateException {
		symbolTable.root().undefine((String)tuple.value(2));
		return super.remove(tuple);
	}
	
	@Override
	protected Tuple insert(Tuple tuple) throws UpdateException {
		Table table = null;
		Table.Type type = (Table.Type) tuple.value(1);
		switch(type) {
		case BASIC:
			table = new BasicTable((String)tuple.value(schema.field("Name")), 
					               (Schema)tuple.value(schema.field("Schema")),
			                       (Long)tuple.value(schema.field("Size")),  
			                       (Long)tuple.value(schema.field("Lifetime")), 
			                       (Key)tuple.value(schema.field("Key")));
			break;
		case REFCOUNT:
			table = new RefTable((String)tuple.value(schema.field("Name")), 
					             (Schema)tuple.value(schema.field("Schema")),
			                     (Key)tuple.value(schema.field("Key")));
			break;
		case OBJECT:
			Class eclass;
				try {
					eclass = Class.forName((String)tuple.value(2));
					Constructor constructor = eclass.getConstructor(String.class, Integer.class, Integer[].class);
					table = (Table) constructor.newInstance(tuple.value(1), tuple.value(schema.field("Name")));
				} catch (Exception e) {
					throw new UpdateException(e.toString());
				}
		}
		assert (table != null);
		tuple.value(schema.field("Table"), table);
		
		Tuple previous = super.insert(tuple);
		symbolTable.root().addDefinition((String)tuple.value(schema.field("Name")), 
				                         ((Schema)tuple.value(schema.field("Schema"))).types());
		/* Update the symbol table. */
		return previous;
	}
	
}
