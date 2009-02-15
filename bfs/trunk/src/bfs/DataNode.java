package bfs;

import java.io.File;
import java.util.ArrayList;

import bfs.DataServer;
import bfs.DataConnection;
import bfs.OlgAssertion;

import jol.core.JolSystem;
import jol.core.Runtime;
import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.exception.JolRuntimeException;
import jol.types.exception.UpdateException;
import jol.types.table.Table;
import jol.types.table.TableName;
import jol.types.table.Table.Callback;
import java.io.FileInputStream;
import java.nio.channels.FileChannel;

import java.util.Set;
import java.util.List;
import java.util.LinkedList;

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
    private JolSystem system;

    public DataNode(int nodeId, String fsRoot) {
        this.nodeId = nodeId;
        this.fsRoot = fsRoot;
        this.port = Conf.getDataNodeControlPort(nodeId);
        this.dserver = new DataServer(Conf.getDataNodeDataPort(nodeId), fsRoot);
        this.dserver.start();
    }

    public int getPort() {
        return port;
    }

    public void start() throws JolRuntimeException, UpdateException {

        Callback copyCallback = new Callback() {
            @Override
            public void deletion(TupleSet tuples) {}

            @Override
            public void insertion(TupleSet tuples) {

                System.out.println("COPYCOPYCOPYCOPY!\n");
                for (Tuple t : tuples) {
                    Integer chunkId = (Integer)t.value(2);
                    Set args = (Set) t.value(3);
                    List<String> path = new LinkedList<String>(args);
                    for (int i = 0; i < path.size(); i++) {
                        //String addr = path.remove(i);
                        try {
                            DataConnection conn = new DataConnection(path);

                            String f = fsRoot + File.separator + "chunks" + File.separator + chunkId.toString();
                            FileChannel fc = new FileInputStream(f).getChannel();

                            conn.sendRoutingData(chunkId);
                            conn.sendChunkContent(fc);

                            fc.close();
                        //} catch (java.net.ConnectException e) {
                        } catch (RuntimeException e) {
                            // fall through
                        } catch(Exception e) {
                            throw new RuntimeException(e);
                        }
                        
                    }
                }
            }
        };
        setupFsRoot();

        /* Identify the address of the local node */
        Conf.setSelfAddress(Conf.getDataNodeAddress(this.nodeId));

        this.system = Runtime.create(Runtime.DEBUG_ALL, System.err, this.port);

        OlgAssertion oa = new OlgAssertion(this.system, false);

        this.system.install("bfs", ClassLoader.getSystemResource("bfs/chunks_global.olg"));
        this.system.evaluate();
        this.system.install("bfs_heartbeat", ClassLoader.getSystemResource("bfs/files.olg"));
        this.system.evaluate();

        /* Identify the data directory */
        TableName tblName = new TableName("bfs_heartbeat", "datadir");
        TupleSet datadir = new TupleSet(tblName);
        datadir.add(new Tuple(Conf.getSelfAddress(), fsRoot));
        this.system.schedule("bfs_heartbeat", tblName, datadir, null);

        Table table = this.system.catalog().table(new TableName("bfs_chunks", "send_migrate"));
        if (table == null) {
          throw new RuntimeException("send_migrate callback not defined");
        }
        table.register(copyCallback);

        this.system.start();
        System.out.println("DataNode @ " + this.port + " (" +
                           Conf.getDataNodeDataPort(this.nodeId) + ") ready!");
    }

    public void shutdown() {
        this.dserver.shutdown();
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
