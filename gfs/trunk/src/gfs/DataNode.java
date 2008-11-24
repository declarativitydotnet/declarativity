package gfs;

import jol.core.System;
import jol.core.Runtime;
import jol.types.exception.JolRuntimeException;

public class DataNode {
    public static void main(String[] args) throws JolRuntimeException {
        if (args.length != 1)
            usage();

        int dataNodeIdx = Integer.parseInt(args[0]);
        if (dataNodeIdx < 0 || dataNodeIdx >= Conf.getNumDataNodes()) {
            java.lang.System.err.println("Illegal data node index: " + dataNodeIdx);
            usage();
        }

        DataNode dn = new DataNode(dataNodeIdx);
        dn.start();
    }

    private static void usage() {
        java.lang.System.err.println("Usage: gfs.DataNode index");
        java.lang.System.err.println("    where 0 <= \"index\" <= " + (Conf.getNumDataNodes() - 1));
        java.lang.System.exit(1);
    }

    private int nodeId;
    private int port;
    private DataServer dserver;
    private Thread serverThread;
    private System system;

    DataNode(int nodeId) {
        this.nodeId = nodeId;
        this.port = Conf.getDataNodeControlPort(nodeId);
        this.dserver = new DataServer(Conf.getDataNodeDataPort(nodeId));
        this.serverThread = new Thread(this.dserver);
        this.serverThread.start();
    }

    public void start() throws JolRuntimeException {
        /* Identify the address of the local node */
        Conf.setSelfAddress(Conf.getDataNodeAddress(this.nodeId));

        this.system = Runtime.create(this.port);

        this.system.start();
        java.lang.System.out.println("DataNode @ " + this.port + " (" +
                                     Conf.getDataNodeDataPort(this.nodeId) + ") ready!");
    }
}
