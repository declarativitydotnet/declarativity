package jol.types.operator;

import java.io.PrintStream;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

import jol.core.Runtime;
import jol.lang.plan.Variable;
import jol.types.basic.Schema;
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
	
	/** The print stream that this watch operators sends to. */
	private PrintStream stream;

	/**
	 * Create a new watch operator.
	 * stderr is used as the print stream.
	 * @param context The runtime context.
	 * @param program The program name 
	 * @param rule The rule name
	 * @param name The table name.
	 * @param modifier The modifier type.
	 */
	public Watch(Runtime context, String program, String rule, TableName name, Modifier modifier) {
		this(context, program, rule, name, modifier, System.err);
	}
	
	/**
	 * Create a new watch operator.
	 * stderr is used as the print stream.
	 * @param context The runtime context.
	 * @param program The program name 
	 * @param rule The rule name
	 * @param name The table name.
	 * @param modifier The modifier type.
	 * @param stream the print stream.
	 */
	public Watch(Runtime context, String program, String rule, TableName name, Modifier modifier, PrintStream stream) {
		super(context, program, rule);
		this.name = name;
		this.modifier = modifier;
		this.stream = stream;
	}

	@Override
	public TupleSet evaluate(TupleSet tuples) throws JolRuntimeException {
		if (tuples.size() == 0) return tuples;
		
		String header = "Program " + program + " [CLOCK " + context.clock().current() + "] " + 
				        modifier.toString() + ": " + name;
				        
		if (this.rule != null) {
			header += " Rule " + rule;
		}
		
		stream.println(header);
		for (Tuple tuple : tuples) {
			stream.println("\t" + tuple);
		}
		return tuples;
	}

	@Override
	public Set<Variable> requires() {
		return new HashSet<Variable>();
	}

	@Override
	public Schema schema() {
		return null;
	}

	@Override
	public String toString() {
		return "Watch " + modifier + ": " + name;
	}

}
