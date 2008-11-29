package jol.net;

import jol.types.basic.TupleSet;
import jol.types.table.TableName;

/**
 * A network message contains the tuple information that
 * some program is sending over the network using some protocol.
 */
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
	
	/**
	 * The protocol used to transfer this message.
	 * @return The protocol name.
	 */
	public String protocol() {
		return this.protocol;
	}
	
	/**
	 * The program that issued the network message.
	 * @return The program name.
	 */
	public String program() {
		return this.program;
	}
	
	/**
	 * The name of the table to which the insertion/deletion
	 * tuples belong.
	 * @return The table name.
	 */
	public TableName name() {
		return this.name;
	}
	
	/**
	 * A set of tuple insertions.
	 * @return A tuple set.
	 */
	public TupleSet insertions() {
		return this.insertions;
	}
	
	/**
	 * A set of tuple deletions.
	 * @return A tuple set.
	 */
	public TupleSet deletions() {
		return this.deletions;
	}
}
