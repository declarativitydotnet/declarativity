package gfs;

import jol.core.System;
import jol.core.Runtime;
import jol.types.exception.JolRuntimeException;
import jol.types.exception.UpdateException;

public class Master {
	private static final int DEFAULT_MASTER_PORT = 5500;

	public static void main(String[] args) throws JolRuntimeException, UpdateException {
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
	
	public void start() throws JolRuntimeException, UpdateException {
		this.system = Runtime.create(this.port);
		this.system.install("gfs", ClassLoader.getSystemResource("gfs/gfs_master.olg"));
	}
}
