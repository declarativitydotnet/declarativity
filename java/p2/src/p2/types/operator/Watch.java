package p2.types.operator;

import java.io.PrintStream;
import java.util.HashSet;
import java.util.Hashtable;
import java.util.Set;

import p2.lang.plan.Variable;
import p2.types.basic.Schema;
import p2.types.basic.TupleSet;
import p2.types.basic.Tuple;
import p2.types.exception.P2RuntimeException;
import p2.types.table.TableName;
import p2.core.Runtime;

public class Watch extends Operator {
	public static enum Modifier{NONE, TRACE, ADD, ERASE, INSERT, DELETE, RECEIVE, SEND};
	public static final Hashtable<Character, Modifier> modifiers = new Hashtable<Character, Modifier>();
	
	static {
		modifiers.put('t', Modifier.TRACE);
		modifiers.put('a', Modifier.ADD);
		modifiers.put('e', Modifier.ERASE);
		modifiers.put('i', Modifier.INSERT);
		modifiers.put('d', Modifier.DELETE);
		modifiers.put('r', Modifier.RECEIVE);
		modifiers.put('s', Modifier.SEND);
	};
	
	
	private TableName name;
	
	private Modifier modifier;
	
	private PrintStream stream;

	public Watch(Runtime context, String program, String rule, TableName name, Modifier modifier) {
		super(context, program, rule);
		this.name = name;
		this.modifier = modifier;
		this.stream = System.err;
	}
	
	public Watch(Runtime context, String program, String rule, TableName name, Modifier modifier, PrintStream stream) {
		super(context, program, rule);
		this.name = name;
		this.modifier = modifier;
		this.stream = stream;
	}

	@Override
	public TupleSet evaluate(TupleSet tuples) throws P2RuntimeException {
		if (tuples.size() == 0) return tuples;
		
		String header = "Program " + program + " [CLOCK " + context.clock().current() + "] " + 
				        modifier.toString() + ": " + name;
				        
		if (this.rule != null) {
			header += " Rule " + rule;
		}
		header += "\n\tSCHEMA: " + tuples.iterator().next().schema();
		
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
