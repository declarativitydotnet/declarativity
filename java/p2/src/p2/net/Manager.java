package p2.net;

import java.io.IOException;

import p2.types.basic.Tuple;
import p2.types.basic.TupleSet;
import p2.types.table.TableName;

public final class Manager {
	
	public static final NetworkBuffer buffer = new NetworkBuffer();
	
	private static TCP tcp;
	
	public Manager(Integer port) throws IOException {
		Manager.tcp = new TCP(port);
	}

}
