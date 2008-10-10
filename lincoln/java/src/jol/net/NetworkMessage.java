package jol.net;

import jol.types.basic.TupleSet;
import jol.types.table.TableName;

public class NetworkMessage extends Message {
	private static final long serialVersionUID = 1L;

	private static Long ids = 0L;
	
	private String protocol;
	
	private String program;
	
	private TableName name;
	
	private TupleSet insertions;
	
	private TupleSet deletions;
	
	public NetworkMessage(String protocol, String program, TableName name, TupleSet insertions, TupleSet deletions) {
		super(ids++, "network");
		this.protocol   = protocol;
		this.program    = program;
		this.name       = name;
		this.insertions = insertions;
		this.deletions  = deletions;
	}
	
	public String protocol() {
		return this.protocol;
	}
	
	public String program() {
		return this.program;
	}
	
	public TableName name() {
		return this.name;
	}
	
	public TupleSet insertions() {
		return this.insertions;
	}
	
	public TupleSet deletions() {
		return this.deletions;
	}
}
