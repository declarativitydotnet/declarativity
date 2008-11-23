package gfs;

import jol.core.System;
import jol.core.Runtime;
import jol.types.exception.JolRuntimeException;

public class DataNode {
    public static void main(String[] args) throws JolRuntimeException {
        int port = 6000;

        if (args.length == 1)
            port = Integer.parseInt(args[0]);
        else
            usage();

        DataNode dn = new DataNode(port);
        dn.start();
    }

    private static void usage() {
        java.lang.System.err.println("Usage: gfs.DataNode [port]");
        java.lang.System.exit(1);
    }

    private int port;
    private DataServer dserver;
    private Thread serverThread;
    private System system;

    DataNode(int port) {
        this.port = port;
        this.dserver = new DataServer(port + 1);
        this.serverThread = new Thread(this.dserver);
        this.serverThread.start();
    }

    public void start() throws JolRuntimeException {
        /* Identify the address of the local node */
        Conf.setSelfAddress("tcp:localhost:" + this.port);

        this.system = Runtime.create(this.port);

        this.system.start();
        java.lang.System.out.println("DataNode @ " + this.port + " ready!");
    }
}
