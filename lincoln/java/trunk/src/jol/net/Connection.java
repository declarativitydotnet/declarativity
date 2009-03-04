package jol.net;

import jol.types.basic.Tuple;
import jol.types.basic.BasicTupleSet;
import jol.types.basic.TupleSet;
import jol.types.exception.BadKeyException;
import jol.types.exception.JolRuntimeException;
import jol.types.exception.UpdateException;
import jol.types.table.Key;
import jol.types.table.ObjectTable;
import jol.types.table.TableName;
import jol.core.Runtime;

public class Connection extends ObjectTable {

	public static final Key PRIMARY_KEY = new Key(0,1);

	public enum Field{PROTOCOL, ADDRESS, CHANNEL};
	public static final Class[] SCHEMA = {
		String.class,   // protocol
		Address.class,  // address
		Channel.class   // channel
	};

	private Runtime context;
	private Network manager;

	protected Connection(Runtime context, Network manager) {
		super(context, new TableName("network", "connection"), PRIMARY_KEY, SCHEMA);
		this.context = context;
		this.manager = manager;
	}

	@Override
	public TupleSet insert(TupleSet tuples, TupleSet conflicts) throws UpdateException {
		TupleSet success = new BasicTupleSet();
		for (Tuple tuple : tuples) {
			String  protocol = (String) tuple.value(Field.PROTOCOL.ordinal());
			Address address  = (Address) tuple.value(Field.ADDRESS.ordinal());
			Channel channel = (Channel) tuple.value(Field.CHANNEL.ordinal());
			if (channel == null) {
				channel = manager.create(protocol, address);
				Object[] vals = tuple.toArray();
				vals[Field.CHANNEL.ordinal()] = channel;
				tuple = new Tuple(vals);
			}

			if (channel != null) {
				success.add(tuple);
			}
		}

		return super.insert(success, conflicts);
	}

	@Override
	public boolean delete(Tuple tuple) throws UpdateException {
		if (super.delete(tuple)) {
			Channel channel = (Channel) tuple.value(Field.CHANNEL.ordinal());
			if (channel != null) {
				manager.close(channel);
			}
			return true;
		}
		return false;
	}

	public void register(Channel channel) throws UpdateException {
		force(new Tuple(channel.protocol(), channel.address(), channel));
	}

	public void unregister(Channel channel) throws JolRuntimeException {
		try {
			TupleSet channels = primary().lookupByKey(channel.protocol(), channel.address());
			this.context.schedule(name().scope, name(), null, channels);
		} catch (BadKeyException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}
}
