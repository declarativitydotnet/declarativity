package bfs;

import java.io.File;

import bfs.DataServer;

import jol.core.JolSystem;
import jol.core.Runtime;
import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.exception.JolRuntimeException;
import jol.types.exception.UpdateException;
import jol.types.table.TableName;

public class DataNode {
    public static void main(String[] args) throws JolRuntimeException, UpdateException {
        if (args.length != 2)
            usage();

        int dataNodeIdx = Integer.parseInt(args[0]);
        if (dataNodeIdx < 0 || dataNodeIdx >= Conf.getNumDataNodes()) {
            System.err.println("Illegal data node index: " + dataNodeIdx);
            usage();
        }

        DataNode dn = new DataNode(dataNodeIdx, args[1]);
        dn.start();
    }

    private static void usage() {
        System.err.println("Usage: bfs.DataNode index dir_root");
        System.err.println("    where 0 <= \"index\" <= " + (Conf.getNumDataNodes() - 1));
        System.exit(1);
    }

    private int nodeId;
    private int port;
    private String fsRoot;
    private DataServer dserver;
    private Thread serverThread;
    private JolSystem system;

    public DataNode(int nodeId, String fsRoot) {
        this.nodeId = nodeId;
        this.fsRoot = fsRoot;
        this.port = Conf.getDataNodeControlPort(nodeId);
        this.dserver = new DataServer(Conf.getDataNodeDataPort(nodeId), fsRoot);
        this.serverThread = new Thread(this.dserver);
        this.serverThread.start();
    }

    public int getPort() {
        return port;
    }

    public void start() throws JolRuntimeException, UpdateException {
        setupFsRoot();

        /* Identify the address of the local node */
        Conf.setSelfAddress(Conf.getDataNodeAddress(this.nodeId));

        this.system = Runtime.create(this.port);
        this.system.install("bfs_global", ClassLoader.getSystemResource("bfs/files.olg"));
        this.system.evaluate();

        /* Identify the data directory */
        TableName tblName = new TableName("bfs", "datadir");
        TupleSet datadir = new TupleSet(tblName);
        //datadir.add(new Tuple(Conf.getSelfAddress(), fsRoot + File.separator + "chunks"));
        datadir.add(new Tuple(Conf.getSelfAddress(), fsRoot));
        this.system.schedule("bfs", tblName, datadir, null);

        this.system.start();

        System.out.println("DataNode @ " + this.port + " (" +
                           Conf.getDataNodeDataPort(this.nodeId) + ") ready!");
    }

    public void shutdown() {
        this.dserver.stop();
        this.system.shutdown();
    }

    private void setupFsRoot() {
        File root = new File(fsRoot);
        if (root.exists()) {
            if (!root.isDirectory())
                throw new RuntimeException("FS root is not a directory: " + root);
        } else {
            if (!root.mkdir())
                throw new RuntimeException("Failed to create directory: " + root);

            System.out.println("Created new root directory: " + root);
        }
    }
}
