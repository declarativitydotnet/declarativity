package jol.types.basic;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.io.Serializable;
import java.util.ArrayList;
import java.util.List;

import jol.lang.plan.DontCare;
import jol.lang.plan.Variable;
import jol.types.exception.JolRuntimeException;

/**
 * A tuple is an ordered list of values. Each value
 * in the tuple must implement the {#link Comparable} interface
 * in order to perform relational comparison operations on the
 * tuple values.  Tuple values must also implement the {#link Serializable}
 * interface in order to ship tuples to remote locations. 
 */
public class Tuple implements Comparable<Tuple>, Serializable {
	private static final long serialVersionUID = 1L;

	transient private static Long idGen = new Long(0);
	
	/** The tuple identifier. */
	transient protected String id;
	
	/** An ordered list of tuple values. */
	protected List<Comparable> values;
	
	/** A timestamp on when this tuple was created. */
	transient protected Long timestamp;
	
	/** A tuple refcount. */
	transient protected Long refCount;
	
	/** The tuple schema. */
	transient protected Schema schema;
	
	
	/** 
	 * Create a new tuple.
	 * @param values The values that make up the tuple.
	 */
	public Tuple(Comparable... values) {
		initialize();
		this.values = new ArrayList<Comparable>();
		for (Comparable value : values) {
			this.values.add(value);
		}
	}
	
	/**
	 * Create a new tuple.
	 * @param values The values that make up the tuple.
	 */
	public Tuple(List<Comparable> values) {
		initialize();
		this.values = new ArrayList<Comparable>(values);
	}
	
	/**
	 * Create an empty tuple.
	 */
	private Tuple() {
		initialize();
		this.values = new ArrayList<Comparable>();
	}
	/**
	 * Read tuple from byte array.
	 * @param b Should be generated by toBytes(), not by serialization
	 * @throws IOException
	 */
	public Tuple(byte[] b) throws IOException {
		initialize();
		fromBytes(b);
	}
	
	private final static int OBJECT = 0;
	private final static int STRING = 1;
	private final static int INT = 2;
	private final static int LONG = 3;
	private final static int SHORT = 4;
	private final static int BOOLEAN = 5;
	private final static int CHAR = 6;
	private final static int BYTE = 7;
	private final static int FLOAT = 8;
	private final static int DOUBLE = 9;
	
	private boolean warned = false;
	
	public byte[] toBytes() throws IOException {
		ByteArrayOutputStream ret = new ByteArrayOutputStream();
		DataOutputStream out = new DataOutputStream(ret);
		out.writeShort(values.size());
		for (Object o : values) {
			if (o instanceof String) {
				out.writeByte(STRING);
				out.writeUTF((String)o);
			} else if (o instanceof Integer) {
				out.writeByte(INT);
				out.writeInt((Integer)o);
			} else if (o instanceof Long) {
				out.writeByte(LONG);
				out.writeLong((Long)o);
			} else if (o instanceof Short) {
				out.writeByte(SHORT);
				out.writeShort((Short)o);
			} else if (o instanceof Boolean) {
				out.writeByte(BOOLEAN);
				out.writeBoolean((Boolean)o);
			} else if (o instanceof Character) {
				out.writeByte(CHAR);
				out.writeChar((Character)o);
			} else if (o instanceof Byte) {
				out.writeByte(BYTE);
				out.writeByte((Byte)o);
			} else if (o instanceof Float) {
				out.writeByte(FLOAT);
				out.writeFloat((Float)o);
			} else if (o instanceof Double) {
				out.writeByte(DOUBLE);
				out.writeDouble((Double)o);
			} else {
				if (!warned) {
					System.out.println("sending non-primitive: " + o.getClass().toString());
					warned = true;
				}
				out.writeByte(OBJECT);
				ByteArrayOutputStream subret = new ByteArrayOutputStream();
				ObjectOutputStream oout = new ObjectOutputStream(subret);
				oout.writeObject(o);
				oout.close();
				byte[] bytes = subret.toByteArray();
				out.writeInt(bytes.length);
				out.write(bytes);
			}
		}
		out.close();
		return ret.toByteArray();
	}
	
	public void fromBytes(byte[] bytes) throws IOException {
		DataInputStream in = new DataInputStream(new ByteArrayInputStream(bytes));
		short size = in.readShort();
		values = new ArrayList<Comparable>(size);
		for (int i = 0; i < size; i++) {
			int type = in.readByte();
			if (type == STRING) {
				values.add(in.readUTF());
			} else if (type == INT) {
				values.add(in.readInt());
			} else if (type == LONG) {
				values.add(in.readLong());
			} else if (type == SHORT) {
				values.add(in.readShort());
			} else if (type == BOOLEAN) {
				values.add(in.readBoolean());
			} else if (type == CHAR) {
				values.add(in.readChar());
			} else if (type == BYTE) {
				values.add(in.readByte());
			} else if (type == FLOAT) {
				values.add(in.readFloat());
			} else if (type == DOUBLE) {
				values.add(in.readDouble());
			} else if (type == OBJECT) {
				int len = in.readInt();
				byte[] obytes = new byte[len];
				in.readFully(obytes);
				ObjectInputStream oin = new ObjectInputStream(
										  new ByteArrayInputStream(obytes));
				
				try {
					values.add((Comparable) oin.readObject());
				} catch (ClassNotFoundException e) {
					throw new IOException("Couldn't deserialize object in column " + i +
										  " of tuple (partial value is: " + toString() + ")");
				}
			} else {
				throw new IOException("Can't read type " + type + ".");
			}
		}
	}
	private void writeObject(ObjectOutputStream out) throws IOException {
		// Use serialization routines that are optimized for single tuples.
		// (This causes the network to use these routines, which doesn't
		// do much for performance, but helps test these routines.)
		byte[] bytes = toBytes();
		out.writeInt(bytes.length);
		out.write(bytes);

		// This would perform normal java serialization if it weren't
		// commented out.

		//out.writeObject(values);
	}
	private void readObject(ObjectInputStream in) throws IOException {
		// Custom tuple serializer
		byte[] bytes = new byte[in.readInt()];
		in.readFully(bytes);
		fromBytes(bytes);

		// Version that would use Java serialization 
//		try {
//			values = (ArrayList<Comparable>)in.readObject();
//		} catch (ClassNotFoundException e) {
//			// This method can only throw IOException, or these methods won't
//			// be called by serialization stuff.
//			throw new IOException("Couldn't read serialized object", e);
//		}
	}
	
	@Override
	public Tuple clone() {
		Tuple copy = new Tuple(this.values);
		copy.schema = this.schema != null ? this.schema.clone() : null;
		copy.id     = this.id;
		return copy;
	}
	
	/** Initialize an empty tuple. */
	private void initialize() {
		this.schema   = new Schema();
		this.refCount = new Long(1);
		this.id       = idGen.toString();
		idGen += 1L;
	}
	
	/**
	 * The tuple identifier.
	 * @return The tuple identifier.
	 */
	public String id() {
		return this.id;
	}
	
	/**
	 * Set the tuple identifier.
	 * @param id The identifier to set the tuple identifier to.
	 */
	public void id(String id) {
		this.id = id;
	}
	
	public void insert(int index, Comparable value) {
		this.values.add(index, value);
	}
	
	/**
	 * Append the tuple value along with the corresponding schema variable.
	 * @param variable The schema variable.
	 * @param value The tuple value.
	 */
	public void append(Variable variable, Comparable value) {
		if (variable != null) {
			variable = variable.clone();
			variable.position(this.schema.size());
			this.schema.append(variable);
		}
		this.values.add(value);
	}
	
	public void remove(Variable var) throws JolRuntimeException {
		if (!this.schema.contains(var)) {
			throw new JolRuntimeException("Unknown variable " + var + 
					" in schema " + this.schema);
		}
		int pos = this.schema.position(var.name());
		this.schema.remove(var);
		this.values.remove(pos);
	}
	
	@Override
	public String toString() {
		String value = "<";
		if (values.size() > 0) {
			value += values.get(0);
			for (int i = 1; i < values.size(); i++) {
				Comparable element = values.get(i);
				value += ", " + (element == null ? "null" : element.toString());
			}
		}
		value += ">";
		return value;
	}
	
	/**
	 * The tuple schema.
	 * @return The tuple schema.
	 */
	public Schema schema() {
		return this.schema;
	}
	
	/**
	 * Set the tuple schema.
	 * @param schema The schema to assign to this tuple.
	 * @throws JolRuntimeException
	 */
	public void schema(Schema schema) throws JolRuntimeException {
		if (schema.size() != size()) {
			throw new JolRuntimeException("Schema " + schema.name() + schema + " does not match tuple arity! " +
					                     " Tuple: " + this + " size =? " + size());
		}
		this.schema = schema;
	}
	
	/** Comparison based on tuple values (empty tuples are equivalent). */
	public int compareTo(Tuple other) {
		if (size() != other.size()) {
			return -1;
		}
		for (int i = 0; i < size(); i++) {
			if (values.get(i) == null || other.values.get(i) == null) {
				if (values.get(i) != other.values.get(i)) {
					return -1;
				}
			}
			else if (values.get(i).compareTo(other.values.get(i)) != 0) {
				return values.get(i).compareTo(other.values.get(i));
			}
		}
		return 0;
	}
	
	@Override
	public boolean equals(Object obj) {
		return obj instanceof Tuple && 
				((Tuple)obj).compareTo(this) == 0;
	}
	
	@Override
	public int hashCode() {
		if (this.values.size() == 0)
				return id.hashCode();

		StringBuilder sb = new StringBuilder();
		for (Comparable value : values) {
			if (value == null)
				sb.append("null".hashCode());
			else
				sb.append(value.hashCode());
		}
		return sb.toString().hashCode();
	}
	
	/** The number of attributes in this tuple. */
	public int size() {
		return this.values.size();
	}
	
	/** The value at the indicated field position. 
	 * @param field The field position.
	 */
	public Comparable value(int field) {
		return values.get(field);
	}
	
	/**
	 *  Sets the value to be in the given field position.
	 *  @param field The field position.
	 *  @param value The value to set. 
	 */
	public void value(int field, Comparable value) {
		if (this.values.size() == field) {
			this.values.add(value);
		}
		else {
			values.set(field, value);
		}
	}
	
	/**
	 * Get the value at the variable position indicated by the variable name.
	 * @param name The variable name.
	 * @return The value at the given position, or null if !exist.
	 */
	public Comparable value(String name) {
		if (this.schema != null) {
			return value(this.schema.position(name));
		}
		return null;
	}
	
	/**
	 * Set the value within the tuple based on the position of the variable.
	 * @param variable The variable.
	 * @param value The value to set.
	 */
	public void value(Variable variable, Comparable value) {
		int position = this.schema.position(variable.name());
		if (position < 0) {
			append(variable, value);
		}
		else {
			value(position, value);
		}
	}
	
	/**
	 * Get the value type based on the variable name.
	 * @param name The variable name.
	 * @return The type of the value at the variable position.
	 */
	public Class type(String name) {
		return this.schema.type(name);
	}
	
	/**
	 * SEt the refcount of this tuple.
	 * @param value The refcount value.
	 */
	public void refCount(Long value) {
		this.refCount = value;
	}
	
	/**
	 * Get the refcount of this tuple.
	 * @return The refcount.
	 */
	public Long refCount() {
		return this.refCount;
	}
	
	/**
	 * Set the timestamp of this tuple.
	 * @param value The timestamp to set.
	 */
	public void timestamp(Long value) {
		this.timestamp = value;
	}
	
	/**
	 * Get the timestamp of this tuple.
	 * @return The timestamp.
	 */
	public Long timestamp() {
		return this.timestamp;
	}
	
	/**
	 * Join this tuple with the inner tuple based on the joining
	 * attribute values of each tuple.
	 * @param inner The tuple to join with.
	 * @return The join tuple or null if join fails.
	 */
	public Tuple join(Tuple inner) {
		Tuple join = new Tuple();
		
		/* Take care of all join variables first. */
		for (Variable variable : schema().variables()) {
			if (variable instanceof DontCare) {
				continue;
			}
			else if (inner.schema().contains(variable)) {
				Comparable outerValue = value(variable.name());
				Comparable innerValue = inner.value(variable.name());
				if (outerValue == null || innerValue == null) {
					if (outerValue == innerValue) {
						join.append(variable, null);
					}
				}
				else if (!value(variable.name()).equals(inner.value(variable.name()))) {
					return null; // Tuples do not join
				}
				else {
					join.append(variable, value(variable.name()));
				}
			}
			else {
				/* Inner does not contain variable so just add it. */
				join.append(variable, value(variable.name()));
			}
		}
		
		/* Append any variables from the inner that do 
		 * not match join variable. */
		for (Variable variable : inner.schema().variables()) {
			if (variable instanceof DontCare) {
				continue;
			}
			else if (!join.schema().contains(variable)) {
				join.append(variable, inner.value(variable.name()));
			}
		}
		return join;
	}
}
