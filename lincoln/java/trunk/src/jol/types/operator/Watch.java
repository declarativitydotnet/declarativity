package jol.types.operator;

import java.io.PrintStream;
import java.util.HashMap;
import java.util.Map;

import jol.core.Runtime;
import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.exception.JolRuntimeException;
import jol.types.table.TableName;

/**
 * Watch operators print the input tuples to some PrintStream before
 * passing the same set of tuples to the output.
 */
public class Watch extends Operator {
	/** The type of watches that exist in the system. */
	public static enum Modifier{NONE, TRACE, ADD, ERASE, INSERT, DELETE, RECEIVE, SEND};

	/** A map from the watch character to the Modifier type. */
	public static final Map<Character, Modifier> modifiers = new HashMap<Character, Modifier>();

	static {
		modifiers.put('t', Modifier.TRACE);
		modifiers.put('a', Modifier.ADD);
		modifiers.put('e', Modifier.ERASE);
		modifiers.put('i', Modifier.INSERT);
		modifiers.put('d', Modifier.DELETE);
		modifiers.put('r', Modifier.RECEIVE);
		modifiers.put('s', Modifier.SEND);
	};


	/** The table name that this watch operator is assigned. */
	private TableName name;

	/** The Modifier type of this watch operators. */
	private Modifier modifier;

	/**
	 * Create a new watch operator.
	 * stderr is used as the print stream.
	 * @param context The runtime context.
	 * @param program The program name.
	 * @param rule The rule name.
	 * @param name The table name.
	 * @param modifier The modifier type.
	 */
	public Watch(Runtime context, String program, String rule, TableName name, Modifier modifier) {
		super(context, program, rule);
		this.name = name;
		this.modifier = modifier;
	}

	@Override
	public TupleSet evaluate(TupleSet tuples) throws JolRuntimeException {
		if (tuples.isEmpty()) return tuples;

		StringBuilder sb = new StringBuilder();
		sb.append("Program ");
		sb.append(program);
		sb.append(" [CLOCK ");
		sb.append(context.clock().current());
		sb.append("] ");
		sb.append(modifier);
		sb.append(": ");
		sb.append(name);

		if (this.rule != null) {
			sb.append(" Rule ");
			sb.append(rule);
		}

		sb.append("\n");
		for (Tuple tuple : tuples) {
			sb.append("\t");
			sb.append(tuple);
			sb.append("\n");
		}

		this.context.output().print(sb);
		return tuples;
	}

	@Override
	public String toString() {
		return "Watch " + modifier + ": " + name;
	}
}
