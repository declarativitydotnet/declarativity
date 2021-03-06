package bfs;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Random;
import java.util.Set;

import jol.core.JolSystem;
import jol.core.Runtime;
import jol.types.basic.BasicTupleSet;
import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.exception.JolRuntimeException;
import jol.types.table.Table;
import jol.types.table.TableName;
import jol.types.table.Table.Callback;

/**
 * This class provides the BoomFS client API. It communicates with both master
 * nodes, to obtain metadata, and data nodes, to read and write data.
 */
public class BFSClient {
	private int[] currentMaster;
    private Random rand;
	private SimpleQueue<Object> responseQueue;
	private JolSystem system;
	private String selfAddr;
	private String selfTargetControlAddr;

	public BFSClient(int port, boolean isDaemon) {
        this.rand = new Random();
        this.currentMaster = new int[Conf.getNumPartitions()];
        this.responseQueue = new SimpleQueue<Object>();

		try {
			this.system = Runtime.create(Runtime.DEBUG_WATCH, System.err, port);
			this.system.setDaemon(isDaemon);

			// If we didn't specify an explicit port (i.e. zero), use the one
			// that JOL has bound to.
			port = this.system.getServerPort();
			this.selfAddr = Conf.findLocalAddress(port);
	        this.selfTargetControlAddr = Conf.findSelfAddress(false);

	        this.system.install("bfs_global", ClassLoader.getSystemResource("bfs/bfs_global.olg"));
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
	        for (int i = 0; i < Conf.getNumPartitions(); i++) {
	        	updateMasterAddr(i);
	        }
	        this.system.start();
		} catch (JolRuntimeException e) {
			throw new RuntimeException(e);
		}
	}

	public BFSClient(int port) {
		this(port, false);
	}

	public BFSClient(boolean isDaemon) {
		this(0, isDaemon);
	}

	public String getSelfDataNodeAddr() {
		return this.selfTargetControlAddr;
	}

	public synchronized boolean createFile(String pathName) {
		return doCreate(pathName, false);
	}

	public synchronized boolean createDir(String pathName) {
		return doCreate(pathName, true);
	}

	private synchronized boolean doCreate(String pathName, final boolean isDir) {
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
                        if (!isDir) {
                        	responseQueue.put(success);
                            break;
                        } else {
                        	String masterName =(String) t.value(2);
                        	String sourceName =(String) t.value(0);
                        	if (masterName.equals(sourceName)) {
                        		responseQueue.put(success);
                        	}
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
        req.add(new Tuple(this.selfAddr, requestId, commandName, pathName));
        try {
        	this.system.schedule("bfs", new TableName("bfs", "start_request"), req, null);
        } catch (JolRuntimeException e) {
        	throw new RuntimeException(e);
        }
        if (isDir) {
        	Boolean success = (Boolean) waitForCollectiveResponse(Conf.getFileOpTimeout(), requestId);
	        return success.booleanValue();
        } else {
	        // Wait for the response
	        int partition = pathName.hashCode() % Conf.getNumPartitions();
	        Boolean success = (Boolean) waitForResponse(Conf.getFileOpTimeout(), partition, requestId);
	        unregisterCallback(responseTbl, responseCallback);
	        return success.booleanValue();
        }
	}

	public synchronized BFSNewChunkInfo createNewChunk(final String path) {
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
                            throw new RuntimeException("Failed to get new chunk for " + path);

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
        req.add(new Tuple(this.selfAddr, requestId, "NewChunk", path));
        try {
        	this.system.schedule("bfs", new TableName("bfs", "start_request"), req, null);
        } catch (JolRuntimeException e) {
        	throw new RuntimeException(e);
        }
        int partition = path.hashCode() % Conf.getNumPartitions();
        BFSNewChunkInfo result = (BFSNewChunkInfo) waitForResponse(Conf.getListingTimeout(), partition, requestId);
        unregisterCallback(responseTbl, responseCallback);

        // Perf hack: if the client is running on a data node, replace the
        // first element of the list with the local address + control port.
        if (this.selfTargetControlAddr != null) {
        	List<String> newNodeList = new ArrayList<String>(result.getCandidateNodes());
        	newNodeList.set(0, this.selfTargetControlAddr);
        	result = new BFSNewChunkInfo(result.getChunkId(), newNodeList);
        }

        return result;
	}

	public synchronized boolean delete(final String path) {
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
        try {
        	this.system.schedule("bfs", new TableName("bfs", "start_request"), req, null);
        } catch (JolRuntimeException e) {
        	throw new RuntimeException(e);
        }
        int partition = path.hashCode() % Conf.getNumPartitions();
        Boolean success = (Boolean) waitForResponse(Conf.getFileOpTimeout(), partition, requestId);
        unregisterCallback(responseTbl, responseCallback);

        System.out.println("Remove of file \"" + path + "\": " +
                (success.booleanValue() ? "succeeded" : "failed"));
        if (success.booleanValue() == false)
        	return false;
        else
        	return true;
	}

	public synchronized boolean rename(String oldPath, String newPath) {
		return false;
	}

	public synchronized Set<BFSFileInfo> getDirListing(final String path) {
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
                        if (success.booleanValue() == false) {
                        	responseQueue.put(null);
                        } else {
                        	String me = (String)t.value(0);
                        	String masterName = (String) t.value(2);
                        	// the hack here is that we (in overlog) assemble the results
                        	// and send a response() tuple to ourselves (Self==Master)
                        	if (me.equals(masterName)) {
                        		Set lsContent = (Set)t.value(4);
                        		System.out.println("master: " + masterName + " : " + lsContent);
                        		// it's going to be a set of sets.
                        		responseQueue.put(lsContent);
                        	}
                        }
                    }
                }
            }
        };

        Table responseTbl = registerCallback(responseCallback, "response");

        // Create and insert the request tuple
        TupleSet req = new BasicTupleSet();
        req.add(new Tuple(this.selfAddr, requestId, "Ls", path));
        try {
        	this.system.schedule("bfs", new TableName("bfs", "start_request"), req, null);
        } catch (JolRuntimeException e) {
        	throw new RuntimeException(e);
        }

    	Set lsResponses = (Set)waitForCollectiveResponse(Conf.getListingTimeout(), requestId);
        
    	Set<BFSFileInfo> bset = new HashSet<BFSFileInfo>();
    	for (Object o : lsResponses) {
    		bset.addAll((Set<BFSFileInfo>)o);
    	}
    	
    	unregisterCallback(responseTbl, responseCallback);
        if (lsResponses == null) // Error
        	return null;

        return Collections.unmodifiableSet(bset);
	}

	public synchronized BFSFileInfo getFileInfo(final String pathName) {
		System.out.println("getFileInfo() called for \"" + pathName + "\"");
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
                        Object fileInfo = t.value(4);
                        // If the request failed, push an empty list onto the
                        // result queue, since we can't push a null
                        List<BFSFileInfo> result = new ArrayList<BFSFileInfo>();
                        if (fileInfo != null)
                        	result.add((BFSFileInfo) fileInfo);

                        responseQueue.put(result);
                        break;
                    }
                }
            }
        };

        Table responseTbl = registerCallback(responseCallback, "response");

        // Create and insert the request tuple
        TupleSet req = new BasicTupleSet();
        req.add(new Tuple(this.selfAddr, requestId, "FileInfo", pathName));
        try {
        	this.system.schedule("bfs", new TableName("bfs", "start_request"), req, null);
        } catch (JolRuntimeException e) {
        	throw new RuntimeException(e);
        }
        int partition = pathName.hashCode() % Conf.getNumPartitions();
        List<BFSFileInfo> result = (List<BFSFileInfo>) waitForResponse(Conf.getFileOpTimeout(), partition, requestId);
        unregisterCallback(responseTbl, responseCallback);

        if (result.size() == 0)
        	return null; // No such file
        else
        	return result.get(0);
	}

	public synchronized List<BFSChunkInfo> getChunkList(final String path) {
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
                            throw new RuntimeException("Failed to get chunk list for " + path);

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
        req.add(new Tuple(this.selfAddr, requestId, "ChunkList", path));
        try {
        	this.system.schedule("bfs", new TableName("bfs", "start_request"), req, null);
        } catch (JolRuntimeException e) {
        	throw new RuntimeException(e);
        }
        int partition = path.hashCode() % Conf.getNumPartitions();
        Set<BFSChunkInfo> chunkSet = (Set<BFSChunkInfo>) waitForResponse(Conf.getListingTimeout(), partition, requestId);
        unregisterCallback(responseTbl, responseCallback);

        // The server returns the set of chunks in unspecified order; we sort by
        // ascending chunk ID, on the assumption that this agrees with the correct
        // order for the chunks in a file
        List<BFSChunkInfo> sortedChunks = new ArrayList<BFSChunkInfo>(chunkSet);
        Collections.sort(sortedChunks);
        return sortedChunks;
	}

	public synchronized Set<String> getChunkLocations(String path, final int chunkId) {
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
                            throw new RuntimeException("Failed to get chunk list for chunk #" + chunkId);

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
                          "ChunkLocations", path + ":" + chunkId));
        try {
        	this.system.schedule("bfs", new TableName("bfs", "start_request"), req, null);
        } catch (JolRuntimeException e) {
        	throw new RuntimeException(e);
        }

        int partition = path.hashCode() % Conf.getNumPartitions();
        Set<String> nodeSet = (Set<String>) waitForResponse(Conf.getFileOpTimeout(), partition, requestId);
        unregisterCallback(responseTbl, responseCallback);
        return Collections.unmodifiableSet(nodeSet);
	}

	public void shutdown() {
		this.system.shutdown();
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

    // XXX: this should be rewritten to account for the fact that masters can
	// die and then resume operation; we should be willing to try to contact
    // them again.
    private Object waitForResponse(long timeout, int partition, int requestId) {
        while (this.currentMaster[partition] < Conf.getNumMasters(partition)) {
            Object result = this.responseQueue.get(timeout);
            if (result != null)
                return result;

            System.out.println("Master #" + this.currentMaster[partition] +
            		           "(" + Conf.getMasterAddress(partition, this.currentMaster[partition]) +
            		           ") timed out. Retry?");
            this.currentMaster[partition]++;
            if (this.currentMaster[partition] == Conf.getNumMasters(partition))
            	break;
            updateMasterAddr(partition);
        }

        throw new RuntimeException("BFS (partition " + partition + ") request #" + requestId + " timed out.");
    }
    
    private Object waitForCollectiveResponse(long timeout, int requestId) {
    	int tries = 0;
    	// one feels that even the timeout should be pushed down into overlog...
    	// but it's easier to do here.
    	while (tries++ < 10) {
    		Object result = this.responseQueue.get(timeout);
            if (result != null)
                return result;
            
            System.out.println("mkdir collective timeout.  Retry?");
    	}

        throw new RuntimeException("mkdir timed out.");
    }    

    private void updateMasterAddr(int partition) {
        TupleSet master = new BasicTupleSet();
        master.add(new Tuple(this.selfAddr, partition,
                             Conf.getMasterAddress(partition, this.currentMaster[partition])));
        try {
            this.system.schedule("bfs_global", new TableName("bfs_global", "master_for_node"), master, null);
            this.system.evaluate();
        } catch (JolRuntimeException e) {
        	throw new RuntimeException(e);
        }
    }

    private int generateId() {
        return rand.nextInt();
    }
}
