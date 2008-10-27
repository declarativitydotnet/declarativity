package gfs;

import jol.core.System;
import jol.core.Runtime;
import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.exception.JolRuntimeException;
import jol.types.exception.UpdateException;
import jol.types.table.Table.Callback;

public class Master {
	private static final int DEFAULT_MASTER_PORT = 5500;

	public static void main(String[] args) throws JolRuntimeException, UpdateException, InterruptedException {
		int port = DEFAULT_MASTER_PORT;

		if (args.length > 1)
			usage();

		if (args.length == 1)
			port = Integer.parseInt(args[0]);

		Master m = new Master(port);
		m.start();
	}

	private static void usage() {
		java.lang.System.err.println("Usage: gfs.Master [port-number]");
		java.lang.System.exit(1);
	}

	private int port;
	private System system;

	public Master(int port) {
		this.port = port;
	}

	public void start() throws JolRuntimeException, UpdateException, InterruptedException {
		this.system = Runtime.create(this.port);

		this.system.catalog().register(new MasterRequestTable((Runtime) this.system));
        this.system.catalog().register(new SelfTable((Runtime) this.system));

        /* Identify which address the local node is at */
        TupleSet self = new TupleSet();
        self.add(new Tuple("tcp:localhost:" + this.port));
        system.schedule("gfs", SelfTable.TABLENAME, self, null);
        Thread.sleep(1000);

		Callback cb = new Callback() {
			@Override
			public void deletion(TupleSet tuples) {}

			@Override
			public void insertion(TupleSet tuples) {
				java.lang.System.out.println("Got request @ master: " + tuples.toString());
			}
		};

		Thread.sleep(1000);
		this.system.catalog().table(MasterRequestTable.TABLENAME).register(cb);

		this.system.install("gfs", ClassLoader.getSystemResource("gfs/gfs.olg"));

		java.lang.System.out.println("Server ready!");
	}
}
