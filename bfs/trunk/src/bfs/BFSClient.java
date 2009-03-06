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

	public BFSClient(int port) {
        this.rand = new Random();
        this.currentMaster = new int[Conf.getNumPartitions()];
        this.responseQueue = new SimpleQueue<Object>();
        this.selfAddr = Conf.findLocalAddress(port);

		try {
			this.system = Runtime.create(Runtime.DEBUG_WATCH, System.err, port);

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

	public synchronized boolean createFile(String pathName) {
		return doCreate(pathName, false);
	}

	public synchronized boolean createDir(String pathName) {
		return doCreate(pathName, true);
	}

	private boolean doCreate(String pathName, final boolean isDir) {
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
        req.add(new Tuple(this.selfAddr, requestId, commandName, pathName));
        try {
        	this.system.schedule("bfs", new TableName("bfs", "start_request"), req, null);
        } catch (JolRuntimeException e) {
        	throw new RuntimeException(e);
        }
        if (isDir) {
        	boolean success = false;
        	Set<Object> b = waitForBroadcastResponse(Conf.getFileOpTimeout(), requestId);
	        unregisterCallback(responseTbl, responseCallback);
        	for (Object bool : b) {
        		success = success || ((Boolean) bool).booleanValue();
        	}
	        return success;
        } else {
	        // Wait for the response
	        int partition = pathName.hashCode() % Conf.getNumPartitions();
	        Boolean success = (Boolean) waitForResponse(Conf.getFileOpTimeout(), partition, requestId);
	        unregisterCallback(responseTbl, responseCallback);
	        return success.booleanValue();
        }
	}

	public synchronized BFSNewChunkInfo getNewChunk(final String path) {
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
                        if (success.booleanValue() == false)
                            throw new RuntimeException("Failed to get directory listing for " + path);
                        String masterName = (String) t.value(2);
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
        try {
        	this.system.schedule("bfs", new TableName("bfs", "start_request"), req, null);
        } catch (JolRuntimeException e) {
        	throw new RuntimeException(e);
        }

        Set<BFSFileInfo> ret = new HashSet<BFSFileInfo>();
    	Set<Object> lsResponses =  waitForBroadcastResponse(Conf.getListingTimeout(), requestId);
        unregisterCallback(responseTbl, responseCallback);
    	for(Object o : lsResponses) {
    		ret.addAll((Set<BFSFileInfo>) o);
    	}
        return Collections.unmodifiableSet(ret);
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

	public synchronized Set<String> getChunkLocations(final String chunkId) {
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
                          "ChunkLocations", chunkId));
        try {
        	this.system.schedule("bfs", new TableName("bfs", "start_request"), req, null);
        } catch (JolRuntimeException e) {
        	throw new RuntimeException(e);
        }

        int partition = chunkId.subSequence(0, chunkId.indexOf(":")).hashCode() % Conf.getNumPartitions();
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

        throw new RuntimeException("BFS (partition "+partition+") request #" + requestId + " timed out.");
    }

    private Set<Object> waitForBroadcastResponse(long timeout, int requestid) throws RuntimeException {
    	boolean done = false;
        Set<Object> ret = new HashSet<Object>();
		Set<Integer> unseenPartitions = new HashSet<Integer>();
		Set<Integer> seenPartitions = new HashSet<Integer>();
		for (int i = 0; i < Conf.getNumPartitions(); i++) {
			unseenPartitions.add(i);
		}
    	while (!done) {
   		long start = System.currentTimeMillis();
    		Tuple results[] = new Tuple[Conf.getNumPartitions()];
    		results[0] = (Tuple)this.responseQueue.get(timeout);
    		for (int i = 1 ; i < Conf.getNumPartitions(); i++) {
    			long now = System.currentTimeMillis();
    			if((start + timeout) - now > 0) {
    				results[i] = (Tuple)this.responseQueue.get((start + timeout) - now);
    			} else {
    				done = true;
    				break;
    			}
    		}
    		for (int i = 0; i < Conf.getNumPartitions(); i++) {
    			if (results[i] != null) {
	    			ret.add(results[i].value(1));
	    			seenPartitions.add((Integer) results[i].value(0));
	    			unseenPartitions.remove((Integer) results[i].value(0));
    			}
    		}

    		if (unseenPartitions.size() == 0)
    			return ret;

    		done = true;
    		for (int i : unseenPartitions) {
    			this.currentMaster[i]++;
    			if (this.currentMaster[i] == Conf.getNumMasters(i)) { done = true; break; }
    			updateMasterAddr(i);
    			done = false;
    		}
    	}
    	throw new RuntimeException("BFS broadcast request #" + requestid + " timed out.  Missing responses from " + unseenPartitions + "got responses from " + seenPartitions);
    }

    private void updateMasterAddr(int partition) {
        TupleSet master = new BasicTupleSet();
        master.add(new Tuple(this.selfAddr, partition,
                             Conf.getMasterAddress(partition, this.currentMaster[partition])));
        try {
            this.system.schedule("bfs", new TableName("bfs_global", "master_for_node"), master, null);
            this.system.evaluate();
        } catch (JolRuntimeException e) {
        	throw new RuntimeException(e);
        }
    }

    private int generateId() {
        return rand.nextInt();
    }
}
