package p2.net;

import java.io.Serializable;

import p2.types.basic.TupleSet;

public class Packet implements Serializable {

	private String program;
	
	private TupleSet insertions;
	
	private TupleSet deletions;
	
	public Packet(String program, TupleSet insertions, TupleSet deletions) {
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
