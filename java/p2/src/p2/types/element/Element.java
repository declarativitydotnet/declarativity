package p2.types.element;

import java.util.*;
import javax.security.auth.callback.Callback;

import p2.types.basic.TupleSet;
import p2.types.exception.ElementException;


public abstract class Element implements Comparable {

	/* Element ID. */
	private String id;
	
	/* The name of the element. */
	private String name;
	
	/* Is this element active? */
	private boolean active;
	
	/* The input ports. */
	private Hashtable<String, Port> inputs;
	
	/* The output ports. */
	private Hashtable<String, Port> outputs;
	
	public Element(String id, String name) {
		this.id = id;
		this.name = name;
		this.active = false;
		this.inputs = new Hashtable<String, Port>();
		this.outputs = new Hashtable<String, Port>();
	}
	
	public int compareTo(Object obj) {
		if (obj instanceof Element) {
			return id().compareTo(((Element)obj).id());
		}
		return hashCode() < obj.hashCode() ? -1 : 1;
	}
	
	public String id() {
		return id;
	}
	
	public String name() {
		return name;
	}
	
	public boolean active() {
		return this.active;
	}
	
	public void active(boolean value) {
		this.active = value;
	}
	
	public int push(String portKey, TupleSet t, Callback cb) {
		TupleSet result = simple_action(t);
		Port port = outputs.get(portKey);
		return port.push(result, cb);
	}
	
	public TupleSet pull(String portKey, Callback cb) {
		Port port = inputs.get(portKey);
		TupleSet result = port.pull(cb);
		return simple_action(result);
	}
	
	public TupleSet simple_action(TupleSet t) {
		return t;
	}
	
	/**
	 * Input port associated with key.
	 * @param key The port key.
	 * @return The input port.
	 */
	public Port input(String key) {
		return inputs.get(key);
	}
	
	/**
	 * Set the port key to the given port.
	 * @param key Port key
	 * @param port Port object. If port == null then
	 * port will be removed.
	 * @throws ElementException 
	 */
	public void input(String key, Port port) throws ElementException {
		if (port == null) {
			this.inputs.remove(key);
		}
		else {
			this.inputs.put(key, port);
		}
	}
	
	/**
	 * Output port associated with key.
	 * @param key The port key.
	 * @return The output port.
	 */
	public Port output(String key) {
		return outputs.get(key);
	}
	
	/**
	 * Set the port key to the given port.
	 * @param key The port key.
	 * @param port The port object. If port == null
	 * then port is removed.
	 * @throws ElementException 
	 */
	public void output(String key, Port port) throws ElementException {
		if (port == null) {
			this.outputs.remove(key);
		}
		else {
			this.outputs.put(key, port);
		}
	}
}
