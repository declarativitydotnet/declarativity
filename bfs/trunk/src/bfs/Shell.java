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
import java.util.HashSet;
import java.util.LinkedList;
import java.util.List;
import java.util.Random;
import java.util.Set;

import jol.core.JolSystem;
import jol.core.Runtime;
import jol.types.basic.BasicTupleSet;
import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.exception.JolRuntimeException;
import jol.types.exception.UpdateException;
import jol.types.table.Table;
import jol.types.table.TableName;
import jol.types.table.Table.Callback;


public class Shell {
	private static final int SHELL_PORT = 5501;

    private int currentMaster[];
    private JolSystem system;
    private Random rand;
    private SimpleQueue<Object> responseQueue;
    private String selfAddr;

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
				shell.doCreateFile(argList, false);
			} else if (op.equals("mkdir")) {
				shell.doCreateFile(argList, true);
			} else if (op.equals("ls")) {
				Set<BFSFileInfo> listing = shell.doListFiles(argList);
				System.out.println("ls:");
				int i = 1;
				for (BFSFileInfo fInfo : listing) {
					System.out.print("  " + i + ". " + fInfo.getName());
					if (fInfo.isDirectory())
						System.out.print(" (d)");
					else
						System.out.print(" (size = " + fInfo.getLength() + ")");

					System.out.println();
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
        this.currentMaster = new int[Conf.getNumPartitions()];
        this.selfAddr = Conf.findLocalAddress(SHELL_PORT);

        this.system = Runtime.create(Runtime.DEBUG_WATCH, System.err, SHELL_PORT);

        OlgAssertion oa = new OlgAssertion(this.system, false);
        Tap tap = new Tap(this.system, Conf.getTapSink());

        this.system.install("bfs", ClassLoader.getSystemResource("bfs/bfs_global.olg"));
        this.system.evaluate();
        this.system.install("bfs_global", ClassLoader.getSystemResource("bfs/heartbeats.olg"));
        this.system.evaluate();
        this.system.install("bfs_global", ClassLoader.getSystemResource("bfs/chunks.olg"));
        this.system.evaluate();
        this.system.install("bfs", ClassLoader.getSystemResource("bfs/bfs.olg"));
        this.system.evaluate();

        /* Identify the address of the local node */
        TupleSet self = new BasicTupleSet();
        self.add(new Tuple(this.selfAddr));
        this.system.schedule("bfs", new TableName("bfs", "self"), self, null);
        this.system.evaluate();
        for(int i = 0; i < Conf.getNumPartitions(); i++) {
        	scheduleNewMaster(i);
        }

        if (Conf.getTapSink() != null) {
            tap.doRewrite("bfs");
            tap.doRewrite("chunks");
            tap.doRewrite("heartbeats");
        }

        this.system.start();
    }

    public void doAppend(List<String> args) throws UpdateException, JolRuntimeException {
        this.doAppend(args, System.in);
    }

    public void doAppend(List<String> args, InputStream s) throws UpdateException, JolRuntimeException {
        if (args.size() != 1)
            usage();

        String filename = args.get(0);

        byte buf[] = new byte[Conf.getChunkSize()];
        while (true) {
            try {
                int nread = s.read(buf);
                if (nread == -1)
                	break;

                // This should be a list of candidate datanodes for the chunk.
                // The last element is the newly-assigned chunk ID. Note that
            	// we make a copy of the list, since we're going to modify it
            	// and don't want to mess with JOL-owned data.
                // XXX: if the first data node in the list is down, we should
                // retry the write to one of the other data nodes
            	BFSNewChunkInfo info = getNewChunk(filename);
            	if (info.getCandidateNodes().size() < Conf.getRepFactor())
                    throw new RuntimeException("server sent too few datanodes: " + info);

                DataConnection conn = new DataConnection(info.getCandidateNodes());
                conn.sendRoutingData(info.getChunkId());
                conn.write(buf, 0, nread);
                conn.close();
            } catch (IOException e) {
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
            Set<String> locations = getChunkLocations(filename+":"+new Integer(chunk.getId()).toString());
            readChunk(chunk, locations);
        }
    }

    private BFSNewChunkInfo getNewChunk(final String filename) throws UpdateException, JolRuntimeException {
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
                            throw new RuntimeException("Failed to get new chunk for " + filename);

                        Object newChunkInfo = t.value(4);
                        responseQueue.put(newChunkInfo);
                        break;
                    }
                }
            }
        };
        Table responseTbl = registerCallback(responseCallback, "response");

        // Create and insert the request tuple
        TupleSet req = new BasicTupleSet();
        req.add(new Tuple(this.selfAddr, requestId, "NewChunk", filename));
        this.system.schedule("bfs", new TableName("bfs", "start_request"), req, null);

        int partition = filename.hashCode() % Conf.getNumPartitions();
        BFSNewChunkInfo result = (BFSNewChunkInfo) spinGet(partition, Conf.getListingTimeout());
        unregisterCallback(responseTbl, responseCallback);
        return result;
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
        TupleSet req = new BasicTupleSet();
        req.add(new Tuple(this.selfAddr, requestId, "ChunkList", filename));
        this.system.schedule("bfs", new TableName("bfs", "start_request"), req, null);

        int partition = filename.hashCode() % Conf.getNumPartitions();
        Set<BFSChunkInfo> chunkSet = (Set<BFSChunkInfo>) spinGet(partition, Conf.getListingTimeout());
        unregisterCallback(responseTbl, responseCallback);

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

	private void unregisterCallback(Table table, Callback cb) {
		table.unregister(cb);
		// Avoid a potential race condition: if we timeout on waiting for
		// a response concurrently with the response arriving, we might
		// add an element to the response queue at the same time that we
		// throw an exception. This will leave a spurious element in the
		// response queue, so clear it AFTER we've unregistered the callback.
		this.responseQueue.clear();
	}

    private Set<String> getChunkLocations(final String chunk) throws UpdateException, JolRuntimeException {
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
        TupleSet req = new BasicTupleSet();
        req.add(new Tuple(this.selfAddr, requestId,
                          "ChunkLocations", chunk.toString()));
        this.system.schedule("bfs", new TableName("bfs", "start_request"), req, null);
        int partition = chunk.subSequence(0, chunk.indexOf(":")).hashCode() % Conf.getNumPartitions();
        Set<String> nodeSet = (Set<String>) spinGet(partition, Conf.getListingTimeout());
        unregisterCallback(responseTbl, responseCallback);
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
            boolean success = dis.readBoolean();
            if (success == false)
            	throw new RuntimeException("chunk not found on data node");

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
                for (int i = 0; i < nread; i++)
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

    private void scheduleNewMaster(int partition) throws JolRuntimeException {
        TupleSet master = new BasicTupleSet();
        master.add(new Tuple(this.selfAddr, partition,
                             Conf.getMasterAddress(partition, this.currentMaster[partition])));
        this.system.schedule("bfs_global", new TableName("bfs_global", "master_for_node"), master, null);
        this.system.evaluate();
    }

    private int generateId() {
        return rand.nextInt();
    }

    public void doCreateFile(List<String> args, final boolean isDir) throws UpdateException, JolRuntimeException {
        if (args.size() != 1)
            usage();

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
                        if(!isDir) {
                        	responseQueue.put(success);
                            break;
                        } else {
                        	String masterName =(String) t.value(2);
                            String truncatedMasterName = masterName.substring(masterName.indexOf(':')+1);
                            int partition = Conf.getPartitionFromString(truncatedMasterName); 
                            responseQueue.put(new Tuple(partition, success));
                        }
                    }
                }
            }
        };
        Table responseTbl = registerCallback(responseCallback, "response");
        String commandName;
        if (isDir)
        	commandName = "CreateDir";
        else
        	commandName = "Create";

        // Create and insert the request tuple
        TupleSet req = new BasicTupleSet();
        req.add(new Tuple(this.selfAddr, requestId, commandName, filename));
        this.system.schedule("bfs", new TableName("bfs", "start_request"), req, null);

        // Wait for the response
        if(!isDir) { 
    		int partition = filename.hashCode() % Conf.getNumPartitions();
        	spinGet(partition, Conf.getFileOpTimeout());
        } else {
        	spinGetBroadcast(Conf.getFileOpTimeout());
        }
        unregisterCallback(responseTbl, responseCallback);
    }

    public Set<BFSFileInfo> doListFiles(List<String> args) throws UpdateException, JolRuntimeException {
    	if (args.size() > 1)
    		usage();

    	final String path;
    	if (args.isEmpty())
    		path = "/";
    	else
    		path = args.get(0);

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
                            throw new RuntimeException("Failed to get directory listing for " + path);
                        String masterName =(String) t.value(2);
                        Object lsContent = t.value(4);
                        System.out.println("master: " + masterName + " : " + lsContent);
                        String truncatedMasterName = masterName.substring(masterName.indexOf(':')+1);
                        int partition = Conf.getPartitionFromString(truncatedMasterName); 
                        responseQueue.put(new Tuple(partition, lsContent));
                    }
                }
            }
        };

        Table responseTbl = registerCallback(responseCallback, "response");

        // Create and insert the request tuple
        TupleSet req = new BasicTupleSet();
        req.add(new Tuple(this.selfAddr, requestId, "Ls", path));
        this.system.schedule("bfs", new TableName("bfs", "start_request"), req, null);
        Object result = spinGetBroadcast(Conf.getListingTimeout());
        unregisterCallback(responseTbl, responseCallback);

        Set<Set<BFSFileInfo>> lsContent = (Set<Set<BFSFileInfo>>) result;
        Set<BFSFileInfo> ret = new HashSet<BFSFileInfo>();
        for(Set<BFSFileInfo> s : lsContent) {
        	ret.addAll(s);
        }
        return Collections.unmodifiableSet(ret);
    }

    public void doRemove(List<String> argList) throws UpdateException, JolRuntimeException {
        if (argList.isEmpty())
            usage();

        for (String s : argList) {
            doRemoveFile(s);
        }
    }

    protected void doRemoveFile(final String path) throws UpdateException, JolRuntimeException {
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
                        System.out.println("Remove of path \"" + path + "\": " +
                                           (success.booleanValue() ? "succeeded" : "failed"));
                        responseQueue.put(success);
                        break;
                    }
                }
            }
        };
        Table responseTbl = registerCallback(responseCallback, "response");

        // Create and insert the request tuple
        TupleSet req = new BasicTupleSet();
        req.add(new Tuple(this.selfAddr, requestId, "Rm", path));
        this.system.schedule("bfs", new TableName("bfs", "start_request"), req, null);

        int partition = path.hashCode() % Conf.getNumPartitions();
        // Wait for the response
        spinGet(partition, Conf.getFileOpTimeout());
        unregisterCallback(responseTbl, responseCallback);
    }

    private Object spinGet(int partition, long timeout) throws JolRuntimeException {
        while (this.currentMaster[partition] < Conf.getNumMasters(partition)) {
            scheduleNewMaster(partition);
            Object result = this.responseQueue.get(timeout);
            if (result != null)
                return result;

            System.out.println("master " + this.currentMaster + " timed out.  retry?\n");
            this.currentMaster[partition]++;
        }
        throw new JolRuntimeException("timed out on all masters");
    }
    private Set<Object> spinGetBroadcast(long timeout) throws JolRuntimeException {
    	boolean done = false;
        Set<Object> ret = new HashSet<Object>();
		Set<Integer> unseenPartitions = new HashSet<Integer>();
		Set<Integer> seenPartitions = new HashSet<Integer>();
		for(int i =0; i < Conf.getNumPartitions(); i++) {
			unseenPartitions.add(i);
		}
    	while (!done) {
    		long start = System.currentTimeMillis();
    		Tuple results[] = new Tuple[Conf.getNumPartitions()];
    		results[0] = (Tuple)this.responseQueue.get(timeout);
    		for(int i =1 ; i < Conf.getNumPartitions(); i++) {
    			long now = System.currentTimeMillis();
    			if((start + timeout) - now > 0)
    				results[i] = (Tuple)this.responseQueue.get((start + timeout) - now);
    			else
    				done = true; break;
    		}
    		for(int i =0; i < Conf.getNumPartitions(); i++) {
    			if(results[i] != null) {
	    			ret.add(results[i].value(1));
	    			seenPartitions.add((Integer)results[i].value(0));
	    			unseenPartitions.remove((Integer)results[i].value(0));
    			}
    		}
    		if(unseenPartitions.size() == 0) { return ret; }
    		done = true;
    		for(int i : unseenPartitions) {
    			this.currentMaster[i]++;
    			if(this.currentMaster[i] == Conf.getNumMasters(i)) { done = true; break; }
    			scheduleNewMaster(i);
    			done = false;
    		}
    	}
    	throw new RuntimeException("BFS broadcast request #??? timed out.  Missing responses from " + unseenPartitions + "got responses from " + seenPartitions);
    }
    public void shutdown() {
        this.system.shutdown();
    }

    private void usage() {
        System.err.println("Usage: java bfs.Shell op_name args");
        System.err.println("Where op_name = {append,create,ls,mkdir,read,rm}");

        shutdown();
        System.exit(0);
    }
}
