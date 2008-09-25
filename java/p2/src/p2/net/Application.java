package p2.net;

import p2.types.basic.TupleSet;

public class Application extends Message {
	private static Long ids = 0L;

	private String program;
	
	private TupleSet insertions;
	
	private TupleSet deletions;
	
	public Application(String program, TupleSet insertions, TupleSet deletions) {
		super(ids++);
		this.program = program;
		this.insertions = insertions;
		this.deletions = deletions;
	}
	
	public String program() {
		return this.program;
	}
	
	public TupleSet insertions() {
		return this.insertions;
	}
	
	public TupleSet deletions() {
		return this.deletions;
	}
}
