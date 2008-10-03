package p2.net;

import p2.types.basic.Tuple;
import p2.types.basic.TupleSet;
import p2.types.basic.TypeList;
import p2.types.exception.BadKeyException;
import p2.types.exception.UpdateException;
import p2.types.table.Key;
import p2.types.table.ObjectTable;
import p2.types.table.TableName;

public class Connection extends ObjectTable {

	public static final Key PRIMARY_KEY = new Key(0,1);
	
	public enum Field{PROTOCOL, ADDRESS, CHANNEL};
	public static final Class[] SCHEMA = {
		String.class,   // protocol
		Address.class,  // address
		Channel.class   // channel
	};
	
	private Network manager;
	
	protected Connection(Network manager) {
		super(new TableName("network", "connection"), PRIMARY_KEY, new TypeList(SCHEMA));
		this.manager = manager;
	}
	
	@Override
	public TupleSet insert(TupleSet tuples, TupleSet conflicts) throws UpdateException {
		for (Tuple tuple : tuples) {
			String  protocol = (String) tuple.value(Field.PROTOCOL.ordinal());
			Address address  = (Address) tuple.value(Field.ADDRESS.ordinal());
			Channel channel = (Channel) tuple.value(Field.CHANNEL.ordinal());
			if (channel == null) {
				channel = manager.create(protocol, address);
				tuple.value(Field.CHANNEL.ordinal(), channel);
			}
		}
		
		return super.insert(tuples, conflicts);
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
	
	public void unregister(Channel channel) throws UpdateException {
		try {
			TupleSet channels = primary().lookup(channel.protocol(), channel.address());
			for (Tuple connection : channels) {
				delete(connection);
			}
		} catch (BadKeyException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		
	}

}
