package bfs;

import java.io.File;
import java.io.FileInputStream;
import java.nio.channels.FileChannel;
import java.util.LinkedList;
import java.util.List;
import java.util.Set;

import jol.core.JolSystem;
import jol.core.Runtime;
import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.exception.JolRuntimeException;
import jol.types.exception.UpdateException;
import jol.types.table.Table;
import jol.types.table.TableName;
import jol.types.table.Table.Callback;

import bfs.Tap;

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
    private File fsRoot;
    private DataServer dserver;
    private JolSystem system;

    public DataNode(int nodeId, String fsRoot) {
        this.nodeId = nodeId;
        this.fsRoot = new File(fsRoot);
        this.port = Conf.getDataNodeControlPort(nodeId);
        this.dserver = new DataServer(Conf.getDataNodeDataPort(nodeId), this.fsRoot);
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
                    Integer chunkId = (Integer) t.value(2);
                    Integer currRepCnt = (Integer) t.value(3);
                    Set<String> args = (Set<String>) t.value(4);
                    List<String> path = new LinkedList<String>(args);

                    while (path.size() > 0) {
                        try {
                            // shorten the possible path to the desired # of replicas
                            List<String> pathCopy = new LinkedList<String>();
                            for (int j = 0; j < path.size() && j < (Conf.getRepFactor() - currRepCnt); j++) {
                                pathCopy.add(path.get(j));
                            }
                            System.out.println("send chunk " + chunkId + " to " + path.get(0));

                            DataConnection conn = new DataConnection(pathCopy);
                            conn.sendRoutingData(chunkId);

                            File f = new File(fsRoot + File.separator + "chunks" + File.separator + chunkId.toString());
                            FileChannel fc = new FileInputStream(f).getChannel();
                            conn.write(fc, (int) f.length());
                            conn.close();
                            fc.close();
                            break;
                        } catch (RuntimeException e) {
                            // fall through
                            path.remove(0);
                        } catch (Exception e) {
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

        ///Tap tap = new Tap(this.system, "tcp:localhost:5678");

        this.system.install("bfs", ClassLoader.getSystemResource("bfs/chunks_global.olg"));
        this.system.evaluate();
        this.system.install("bfs_heartbeat", ClassLoader.getSystemResource("bfs/files.olg"));
        this.system.evaluate();


        /* Identify the data directory */
        TableName tblName = new TableName("bfs_heartbeat", "datadir");
        TupleSet datadir = new TupleSet(tblName);
        datadir.add(new Tuple(Conf.getSelfAddress(), this.fsRoot));
        this.system.schedule("bfs_heartbeat", tblName, datadir, null);

        Table table = this.system.catalog().table(new TableName("bfs_chunks", "send_migrate"));
        table.register(copyCallback);


        /* get a tap program */
        ////tap.do_rewrite("bfs_heartbeat");
        this.system.start();
    
        
        System.out.println("DataNode @ " + this.port + " (" +
                           Conf.getDataNodeDataPort(this.nodeId) + ") ready!");
    }

    public void shutdown() {
        this.dserver.shutdown();
        this.system.shutdown();
    }

    private void setupFsRoot() {
    	mkdir(this.fsRoot);
    	mkdir(new File(this.fsRoot, "chunks"));
    	mkdir(new File(this.fsRoot, "checksums"));
    }

    private void mkdir(File dir) {
    	if (dir.exists()) {
    		if (!dir.isDirectory())
    			throw new RuntimeException("Path exists, but is not a directory:" + dir);
    	} else {
    		if (!dir.mkdir())
    			throw new RuntimeException("Failed to create directory: " + dir);

    		System.out.println("Created new directory: "+ dir);
    	}
    }
}
