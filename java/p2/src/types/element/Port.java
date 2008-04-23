package types.element;

import java.util.HashSet;
import javax.security.auth.callback.Callback;

import types.basic.Tuple;
import types.basic.TupleSet;
import types.exception.UpdateException;
import types.table.ObjectTable;

public class Port implements Comparable {
	public enum Interface{INPUT, OUTPUT};
	
	/* Port types. */
	public enum Type{PUSH, PULL};
	
	/* The interface that this port lies. */
	private Interface io;
	
	/* The type of port. */
	private Type type;
	
	/* The port owner. */
	private Element owner;
	
	/* The port key. */
	private String portKey;
	
	/* The other end of this port. */
	private Element corespondent;

	public Port(Interface io, Type type, Element owner, String portKey) {
		this.io = io;
		this.type = type;
		this.owner = owner;
		this.portKey = portKey;
	}
	
	public int compareTo(Object o) {
		return hashCode() < o.hashCode() ? -1 :
			     hashCode() > o.hashCode() ? 1 : 0;
	}
	
	public int hashCode() {
		if (owner != null) {
			String code = owner.id() + io.toString() + portKey.toString();
			return code.hashCode();
		}
		return 0;
	}
	
	public Interface io() {
		return this.io;
	}
	
	public Type type() {
		return this.type;
	}
	
	public String portKey() {
		return this.portKey;
	}

	public void correspondent(Element e) {
		this.corespondent = e;
	}


	public int push(TupleSet t, Callback cb) {
		Port port = this.corespondent.input(portKey);
		return port.tuple(t, cb);
	}
	
	public TupleSet pull(Callback cb) {
		Port port = this.corespondent.output(portKey);
		return port.tuple(cb);
	}

	private int tuple(TupleSet t, Callback cb) {
		return owner.push(portKey, t, cb); 
	}

	private TupleSet tuple(Callback cb) {
		return owner.pull(portKey, cb);
	}

}