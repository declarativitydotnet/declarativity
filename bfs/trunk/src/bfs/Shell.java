package bfs;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.net.SocketAddress;
import java.nio.ByteBuffer;
import java.nio.channels.SocketChannel;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.LinkedList;
import java.util.List;
import java.util.Random;
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


public class Shell {
    private int currentMaster;
    private JolSystem system;
    private Random rand;
    private SimpleQueue<Object> responseQueue;

    /*
     * TODO:
     *  (1) connect to an instance of JOL
     *  (2) parse command-line argument into command + arguments
     *  (3) inject the appropriate inserts into JOL; wait for the results to come back
     *  (4) return results to stdout
     */
    public static void main(String[] args) throws Exception {
        Shell shell = new Shell();

        try {
			List<String> argList = new LinkedList<String>(Arrays.asList(args));

			if (argList.size() == 0)
				shell.usage();

			String op = argList.remove(0);
			if (op.equals("append")) {
				shell.doAppend(argList);
			} else if (op.equals("read")) {
				shell.doRead(argList);
			} else if (op.equals("create")) {
				shell.doCreateFile(argList, true);
			} else if (op.equals("ls")) {
				Set<BFSFileInfo> listing = shell.doListFiles(argList);
				System.out.println("ls:");
				int i = 1;
				for (BFSFileInfo fInfo : listing) {
					System.out.println("  " + i + ". " + fInfo.getName());
					i++;
				}
			} else if (op.equals("rm")) {
				shell.doRemove(argList);
			} else {
				shell.usage();
			}
		} finally {
			shell.shutdown();
		}
    }

    public Shell() throws JolRuntimeException, UpdateException {
        this.rand = new Random();
        this.responseQueue = new SimpleQueue<Object>();
        this.currentMaster = 0;

        /* Identify the address of the local node */
        /* this is necessary for current tests, but may not be done this way in the future */
        String port = System.getenv("PORT");
        if (port == null) {
            port = "5501";
        }

        /* this shouldn't be a static member at all... */
        Conf.setSelfAddress("tcp:localhost:" + port);

        this.system = Runtime.create(true, Integer.valueOf(port));

        this.system.install("bfs", ClassLoader.getSystemResource("bfs/bfs_global.olg"));
        this.system.install("bfs_global", ClassLoader.getSystemResource("bfs/heartbeats.olg"));
        this.system.evaluate();
        this.system.install("bfs_global", ClassLoader.getSystemResource("bfs/chunks.olg"));
        this.system.evaluate();
        this.system.install("bfs", ClassLoader.getSystemResource("bfs/bfs.olg"));
        this.system.evaluate();

        scheduleNewMaster();
        this.system.start();
    }

    public void doAppend(List<String> args) throws UpdateException {
        this.doAppend(args, System.in);
    }

    public void doAppend(List<String> args, InputStream s) throws UpdateException {
        if (args.size() != 1)
            usage();

        String filename = args.get(0);

        int b = 1;
        while (b != -1) {
            // This should be a list of candidate datanodes for the chunk.
            // The last element is the newly-assigned chunk ID. Note that
        	// we make a copy of the list, since we're going to modify it
        	// and don't want to mess with JOL-owned data.
            // XXX: if the first data node in the list is down, we should
            // retry the write to one of the other data nodes
            List<String> path = new ArrayList<String>(getNewChunk(filename));
            if (path.size() < (Conf.getRepFactor() + 1))
                throw new RuntimeException("server sent too few datanodes: " + path.toString());

            String firstAddr = path.remove(0);
            int chunkId = Integer.valueOf(path.remove(path.size() - 1));
            DataConnection conn = new DataConnection(firstAddr);
            try {
                conn.sendRoutingData(chunkId, path);

                int nread = 0;
                byte buf[] = new byte[Conf.getBufSize()];
                while (nread < Conf.getChunkSize()) {
                    b = s.read(buf, 0, Conf.getBufSize());
                    if (b == -1)
                    	break;
                    conn.write(buf);
                    nread += b;
                }
                System.out.println("exiting inner loop with " + b + " retval and " + nread + " bytes written");
                conn.close();
            } catch (Exception e) {
                throw new RuntimeException(e);
            }
        }
    }

    private void doRead(List<String> args) throws UpdateException, JolRuntimeException {
        if (args.size() != 1)
            usage();

        String filename = args.get(0);

        /* Ask a master node for the list of chunks */
        List<BFSChunkInfo> chunks = getChunkList(filename);
        System.out.println("chunks = " + chunks);

        /*
         * For each chunk, ask the master node for a list of data nodes that
         * hold the chunk, and then read the chunk's contents from one or
         * more of those nodes.
         */
        for (BFSChunkInfo chunk : chunks) {
            Set<String> locations = getChunkLocations(chunk.getId());
            readChunk(chunk, locations);
        }
    }

    private List<String> getNewChunk(final String filename) throws UpdateException {
        final int requestId = generateId();

        // Register a callback to listen for responses
        Callback responseCallback = new Callback() {
            @Override
            public void deletion(TupleSet tuples) {}

            @Override
            public void insertion(TupleSet tuples) {
                for (Tuple t : tuples) {
                    Integer tupRequestId = (Integer) t.value(1);

                    if (tupRequestId.intValue() == requestId) {
                        Boolean success = (Boolean) t.value(3);
                        if (success.booleanValue() == false)
                            throw new RuntimeException("Failed to get chunk list for " + filename);

                        Object chunkList = t.value(4);
                        responseQueue.put(chunkList);
                        break;
                    }
                }
            }
        };
        Table responseTbl = registerCallback(responseCallback, "response");

        // Create and insert the request tuple
        TableName tblName = new TableName("bfs", "start_request");
        TupleSet req = new TupleSet(tblName);
        req.add(new Tuple(Conf.getSelfAddress(), requestId, "NewChunk", filename));
        this.system.schedule("bfs", tblName, req, null);

        List<String> resultList = (List<String>) this.responseQueue.get(); // XXX: timeout?
        responseTbl.unregister(responseCallback);
        return Collections.unmodifiableList(resultList);
    }

    private List<BFSChunkInfo> getChunkList(final String filename) throws UpdateException, JolRuntimeException {
        final int requestId = generateId();

        // Register a callback to listen for responses
        Callback responseCallback = new Callback() {
            @Override
            public void deletion(TupleSet tuples) {}

            @Override
            public void insertion(TupleSet tuples) {
                for (Tuple t : tuples) {
                    Integer tupRequestId = (Integer) t.value(1);

                    if (tupRequestId.intValue() == requestId) {
                        Boolean success = (Boolean) t.value(3);
                        if (success.booleanValue() == false)
                            throw new RuntimeException("Failed to get chunk list for " + filename);

                        Object chunkSet = t.value(4);
                        responseQueue.put(chunkSet);
                        break;
                    }
                }
            }
        };
        Table responseTbl = registerCallback(responseCallback, "response");

        // Create and insert the request tuple
        TableName tblName = new TableName("bfs", "start_request");
        TupleSet req = new TupleSet(tblName);
        req.add(new Tuple(Conf.getSelfAddress(), requestId, "ChunkList", filename));
        this.system.schedule("bfs", tblName, req, null);

        Set<BFSChunkInfo> chunkSet = (Set<BFSChunkInfo>) spinGet(Conf.getListingTimeout());
        responseTbl.unregister(responseCallback);

        // The server returns the list of chunks in unspecified order; we sort by
        // ascending chunk ID, on the assumption that this agrees with the correct
        // order for the chunks in a file
        List<BFSChunkInfo> sortedChunks = new ArrayList<BFSChunkInfo>(chunkSet);
        Collections.sort(sortedChunks);
        return sortedChunks;
    }

    private Table registerCallback(Callback callback, String tableName) {
        Table table = this.system.catalog().table(new TableName("bfs", tableName));
        table.register(callback);
        return table;
    }

    private Set<String> getChunkLocations(final Integer chunk) throws UpdateException, JolRuntimeException {
        final int requestId = generateId();

        // Register a callback to listen for responses
        Callback responseCallback = new Callback() {
            @Override
            public void deletion(TupleSet tuples) {}

            @Override
            public void insertion(TupleSet tuples) {
                for (Tuple t : tuples) {
                    Integer tupRequestId = (Integer) t.value(1);

                    if (tupRequestId.intValue() == requestId) {
                        Boolean success = (Boolean) t.value(3);
                        if (success.booleanValue() == false)
                            throw new RuntimeException("Failed to get chunk list for chunk #" + chunk);

                        Object nodeSet = t.value(4);
                        responseQueue.put(nodeSet);
                        break;
                    }
                }
            }
        };
        Table responseTbl = registerCallback(responseCallback, "response");

        // Create and insert the request tuple
        TableName tblName = new TableName("bfs", "start_request");
        TupleSet req = new TupleSet(tblName);
        req.add(new Tuple(Conf.getSelfAddress(), requestId,
                          "ChunkLocations", chunk.toString()));
        this.system.schedule("bfs", tblName, req, null);

        Set<String> nodeSet = (Set<String>) spinGet(Conf.getListingTimeout());
        responseTbl.unregister(responseCallback);
        return Collections.unmodifiableSet(nodeSet);
    }

    private void readChunk(BFSChunkInfo chunk, Set<String> locations) {
        for (String loc : locations) {
            String sb = readChunkFromAddress(chunk, loc);
            if (sb != null) {
                System.out.println("Content of " + chunk + ": " + sb);
                return;
            }
        }

        throw new RuntimeException("Failed to read chunk " + chunk);
    }

    private String readChunkFromAddress(BFSChunkInfo chunk, String addr) {
        try {
            String[] parts = addr.split(":");
            String host = parts[1];
            int controlPort = Integer.parseInt(parts[2]);

            System.out.println("TEST2\n");
            int dataPort = Conf.findDataNodeDataPort(host, controlPort);

            System.out.println("Reading " + chunk + " from: " + host + ":" + dataPort);
            SocketAddress sockAddr = new InetSocketAddress(host, dataPort);
            SocketChannel inChannel = SocketChannel.open();
            inChannel.configureBlocking(true);
            inChannel.connect(sockAddr);

            Socket sock = inChannel.socket();
            DataOutputStream dos = new DataOutputStream(sock.getOutputStream());
            DataInputStream dis = new DataInputStream(sock.getInputStream());

            dos.writeByte(DataProtocol.READ_OPERATION);
            dos.writeInt(chunk.getId());
            int length = dis.readInt();
            if (length != chunk.getLength())
            	throw new RuntimeException("expected length " + chunk.getLength() +
            			                   ", data node copy's length is " +
            			                   length + ", chunk " + chunk.getId());

            // XXX: rewrite this to use FileChannel.transferFrom()
            StringBuilder sb = new StringBuilder();
            ByteBuffer buf = ByteBuffer.allocate(8192);
            int remaining = length;
            while (remaining > 0) {
                int nread = inChannel.read(buf);
                if (nread == -1)
                    throw new IOException("Unexpected EOF from data node");
                remaining -= nread;
                byte[] ary = buf.array();
                for (int i = 0; i < ary.length; i++)
                    sb.append((char) ary[i]);
                buf.rewind();
            }

            if (sb.length() != length)
            	throw new RuntimeException("unexpected length mismatch: " +
            			                   "expected " + length + ", got " +
            			                   sb.length());

            sock.close();
            return sb.toString();
        } catch (Exception e) {
            System.out.println("Exception reading chunk from " +
                               addr + ": " + e.toString());
            e.printStackTrace();
            return null;
        }
    }

    private void scheduleNewMaster() throws JolRuntimeException {
        TupleSet master = new TupleSet();
        master.add(new Tuple(Conf.getSelfAddress(),
                             Conf.getMasterAddress(this.currentMaster)));
        try {
            this.system.schedule("bfs", MasterTable.TABLENAME, master, null);
        } catch (UpdateException e) {
            throw new JolRuntimeException(e);
        }
        this.system.evaluate();
    }

    private int generateId() {
        return rand.nextInt();
    }

    public void doCreateFile(List<String> args, boolean fromStdin) throws UpdateException, JolRuntimeException {
        if (args.size() != 1)
            usage();

        // XXX: file content isn't used right now
        StringBuilder sb = new StringBuilder();
        if (fromStdin && false) {
            /* Read the contents of the file from stdin */
            int b;
            try {
                while ((b = System.in.read()) != -1)
                    sb.append((char) b);
            } catch (IOException e) {
                throw new RuntimeException(e);
            }
        } else {
            sb.append("foo");
        }

        String filename = args.get(0);
        final int requestId = generateId();

        // Register a callback to listen for responses
        Callback responseCallback = new Callback() {
            @Override
            public void deletion(TupleSet tuples) {}

            @Override
            public void insertion(TupleSet tuples) {
                for (Tuple t : tuples) {
                    Integer tupRequestId = (Integer) t.value(1);

                    if (tupRequestId.intValue() == requestId) {
                        Boolean success = (Boolean) t.value(3);

                        if (success.booleanValue())
                            System.out.println("Create succeeded.");
                        else
                            System.out.println("Create failed.");

                        responseQueue.put(success);
                        break;
                    }
                }
            }
        };
        Table responseTbl = registerCallback(responseCallback, "response");

        // Create and insert the request tuple
        TableName tblName = new TableName("bfs", "start_request");
        TupleSet req = new TupleSet(tblName);
        req.add(new Tuple(Conf.getSelfAddress(), requestId, "Create", filename));
        this.system.schedule("bfs", tblName, req, null);

        // Wait for the response
        Object obj = spinGet(Conf.getFileOpTimeout());
        responseTbl.unregister(responseCallback);
    }

    public Set<BFSFileInfo> doListFiles(List<String> args) throws UpdateException, JolRuntimeException {
        if (!args.isEmpty())
            usage();

        final int requestId = generateId();

        // Register a callback to listen for responses
        Callback responseCallback = new Callback() {
            @Override
            public void deletion(TupleSet tuples) {}

            @Override
            public void insertion(TupleSet tuples) {
                for (Tuple t : tuples) {
                    Integer tupRequestId = (Integer) t.value(1);

                    if (tupRequestId.intValue() == requestId) {
                        Object lsContent = t.value(4);
                        responseQueue.put(lsContent);
                        break;
                    }
                }
            }
        };

        Table responseTbl = registerCallback(responseCallback, "response");

        // Create and insert the request tuple
        TableName tblName = new TableName("bfs", "start_request");
        TupleSet req = new TupleSet(tblName);
        req.add(new Tuple(Conf.getSelfAddress(), requestId, "Ls", null));
        this.system.schedule("bfs", tblName, req, null);

        Object result = spinGet(Conf.getListingTimeout());
        responseTbl.unregister(responseCallback);

        Set<BFSFileInfo> lsContent = (Set<BFSFileInfo>) result;
        return Collections.unmodifiableSet(lsContent);
    }

    public void doRemove(List<String> argList) throws UpdateException, JolRuntimeException {
        if (argList.isEmpty())
            usage();

        for (String s : argList) {
            doRemoveFile(s);
        }
    }

    protected void doRemoveFile(final String file) throws UpdateException, JolRuntimeException {
        final int requestId = generateId();

        // Register callback to listen for responses
        Callback responseCallback = new Callback() {
            @Override
            public void deletion(TupleSet tuples) {}

            @Override
            public void insertion(TupleSet tuples) {
                for (Tuple t : tuples) {
                    Integer tupRequestId = (Integer) t.value(1);

                    if (tupRequestId.intValue() == requestId) {
                        Boolean success = (Boolean) t.value(3);
                        System.out.println("Remove of file \"" + file + "\": " +
                                           (success.booleanValue() ? "succeeded" : "failed"));
                        responseQueue.put(success);
                        break;
                    }
                }
            }
        };
        Table responseTbl = registerCallback(responseCallback, "response");

        // Create and insert the request tuple
        TableName tblName = new TableName("bfs", "start_request");
        TupleSet req = new TupleSet(tblName);
        req.add(new Tuple(Conf.getSelfAddress(), requestId, "Rm", file));
        this.system.schedule("bfs", tblName, req, null);

        // Wait for the response
        spinGet(Conf.getFileOpTimeout());
        responseTbl.unregister(responseCallback);
    }

    private Object spinGet(long timeout) throws JolRuntimeException {
        while (this.currentMaster < Conf.getNumMasters()) {
            scheduleNewMaster();
            Object result = this.responseQueue.get(timeout);
            if (result != null)
                return result;

            System.out.println("master "+this.currentMaster+" timed out.  retry?\n");
            this.currentMaster++;
        }
        throw new JolRuntimeException("timed out on all masters");
    }

    public void shutdown() {
        this.system.shutdown();
    }

    private void usage() {
        System.err.println("Usage: java bfs.Shell op_name args");
        System.err.println("Where op_name = {append,create,ls,read,rm}");

        shutdown();
        System.exit(0);
    }
}
