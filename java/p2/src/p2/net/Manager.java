package p2.net;

import p2.types.basic.Tuple;
import p2.types.basic.TupleSet;
import p2.types.table.TableName;

public final class Manager {
	
	private static final NetworkBuffer buffer = new NetworkBuffer();
	
	private static TCP tcp;
	
	public Manager(String hostname, Integer port) {
		Manager.tcp = new TCP(hostname, port);
	}
	
	public static TupleSet marshall(String program, String name, Comparable... values) {
		TupleSet marshall = new TupleSet(new TableName(program, name));
		marshall.add(new Tuple(values));
		return marshall;
	}

}
