package jol.types.operator;

import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.exception.JolRuntimeException;
import jol.types.exception.UpdateException;
import jol.types.table.Key;
import jol.types.table.ObjectTable;
import jol.types.table.TableName;
import jol.core.Runtime;

/**
 * This class represents the interface extended by all
 * operators in the system.
 *
 * An operator takes a set of tuples as input and produces another
 * set of tuples as output.
 *
 */
public abstract class Operator implements Comparable<Operator> {

	/** Used to create unique operator identifiers. */
	private static Long id = new Long(0);

	/** Creates a unique operator identifier. */
	private static String newID() {
		String identifier = "Operator:" + Operator.id.toString();
		Operator.id += 1L;
		return identifier;
	}

	/**
	 * The operator table stores all operator objects used by
	 * programs installed in the runtime.
	 */
	public static class OperatorTable extends ObjectTable {
		/** The operator table name. */
		public static final TableName TABLENAME = new TableName(GLOBALSCOPE, "operator");

		/** The primary key: should be the unique operator identifier. */
		public static final Key PRIMARY_KEY = new Key(2);

		/** Operator fields. */
		public enum Field {PROGRAM, RULE, ID, SELECTIVITY, OBJECT};

		/** Operator field types. */
		public static final Class[] SCHEMA = {
			String.class,   // Program name
			String.class,   // Rule name
			String.class,   // Operator identifier
			Float.class,    // Selectivity
			Operator.class  // Operator object
		};

		/**
		 * Create a new operator table.
		 * @param context The runtime context.
		 */
		public OperatorTable(Runtime context) {
			super(context, TABLENAME, PRIMARY_KEY, SCHEMA);
		}
	}

	/** A unique identifier. */
	private final String identifier;

	/** The runtime context. */
	protected Runtime context;

	/** The program that this operator is part of. */
	protected String program;

	/** The rule within the program that this operator is part of. */
	protected String rule;

	/**
	 * Create a new operator.
	 * The operator will be registered with the operator table during
	 * this construction.
	 * @param context The runtime context.
	 * @param program The program
	 * @param rule The rule.
	 */
	public Operator(Runtime context, String program, String rule) {
		this.identifier = Operator.newID();
		this.context = context;
		this.program = program;
		this.rule = rule;
		try {
			Tuple me = new Tuple(OperatorTable.TABLENAME, program, rule,
							     this.identifier, null, this);
			context.catalog().table(OperatorTable.TABLENAME).force(me);
		} catch (UpdateException e) {
			e.printStackTrace();
		}
	}

	/**
	 * Compares operator identifiers.
	 */
	public int compareTo(Operator o) {
		return this.identifier.compareTo(o.identifier);
	}

	/**
	 * Sets the rule for this operator.
	 * @param rule
	 */
	public void rule(String rule) {
		this.rule = rule;
	}

	@Override
	public abstract String toString();

	/**
	 * The evaluation routine that all operators use to
	 * implement the operator functionality.
	 * @param tuples The input tuples to be processed by this operator.
	 * @return The output tuples that have been processed by this operator.
	 * @throws JolRuntimeException
	 */
	public abstract TupleSet evaluate(TupleSet tuples) throws JolRuntimeException;

}
