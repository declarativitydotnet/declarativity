package bfs;

import java.util.ArrayList;
import java.util.Collections;
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

/**
 * This class provides the BoomFS client API. It communicates with both master
 * nodes, to obtain metadata, and data nodes, to read and write data.
 */
public class BFSClient {
	private int currentMaster;
    private Random rand;
	private SimpleQueue responseQueue;
	private JolSystem system;

	public BFSClient(int port) {
        this.rand = new Random();
        this.currentMaster = 0;
        this.responseQueue = new SimpleQueue();

		try {
	        /* this shouldn't be a static member at all... */
	        Conf.setSelfAddress("tcp:localhost:" + port);

			this.system = Runtime.create(port);

	        this.system.install("bfs_global", ClassLoader.getSystemResource("bfs/bfs_global.olg"));
	        this.system.evaluate();
            this.system.install("bfs_global", ClassLoader.getSystemResource("bfs/heartbeats.olg"));
            this.system.evaluate();
            this.system.install("bfs_global", ClassLoader.getSystemResource("bfs/chunks.olg"));
            this.system.evaluate();
	        this.system.install("bfs", ClassLoader.getSystemResource("bfs/bfs.olg"));
	        this.system.evaluate();

	        updateMasterAddr();
	        this.system.start();
		} catch (JolRuntimeException e) {
			throw new RuntimeException(e);
		} catch (UpdateException e) {
			throw new RuntimeException(e);
		}
	}

	public void createFile() {
		;
	}

	public boolean delete(final String path) {
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
                        System.out.println("Remove of file \"" + path + "\": " +
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
        req.add(new Tuple(Conf.getSelfAddress(), requestId, "Rm", path));
        try {
        	this.system.schedule("bfs", tblName, req, null);
        } catch (UpdateException e) {
        	throw new RuntimeException(e);
        }

        Boolean success = (Boolean) waitForResponse(Conf.getFileOpTimeout());
        responseTbl.unregister(responseCallback);

        if (success.booleanValue() == false)
        	return false;
        else
        	return true;
	}

	public boolean rename(String oldPath, String newPath) {
		return false;
	}

	// We ignore the path, for now
	public List<BFSFileInfo> getDirListing(String path) {
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
        try {
        	this.system.schedule("bfs", tblName, req, null);
        } catch (UpdateException e) {
        	throw new RuntimeException(e);
        }

        List<String> lsContent = (List<String>) waitForResponse(Conf.getListingTimeout());
        responseTbl.unregister(responseCallback);

        // Currently, we just get a list of file names back from Overlog,
        // so we do the conversion to BFSFileInfo by hand.
        List<BFSFileInfo> result = new ArrayList<BFSFileInfo>();
        for (String fileName : lsContent) {
        	result.add(new BFSFileInfo(fileName, null));
        }

        return result;
	}

	public BFSFileInfo getFileInfo(final String pathName) {
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
        TableName tblName = new TableName("bfs", "start_request");
        TupleSet req = new TupleSet(tblName);
        req.add(new Tuple(Conf.getSelfAddress(), requestId, "FileInfo", pathName));
        try {
        	this.system.schedule("bfs", tblName, req, null);
        } catch (UpdateException e) {
        	throw new RuntimeException(e);
        }

        List<BFSFileInfo> result = (List<BFSFileInfo>) waitForResponse(Conf.getFileOpTimeout());
        responseTbl.unregister(responseCallback);

        if (result.size() == 0)
        	return null; // No such file
        else
        	return result.get(0);
	}

	public List<Integer> getChunkList(final String path) {
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
        TableName tblName = new TableName("bfs", "start_request");
        TupleSet req = new TupleSet(tblName);
        req.add(new Tuple(Conf.getSelfAddress(), requestId, "ChunkList", path));
        try {
        	this.system.schedule("bfs", tblName, req, null);
        } catch (UpdateException e) {
        	throw new RuntimeException(e);
        }

        Set<Integer> chunkSet = (Set<Integer>) waitForResponse(Conf.getListingTimeout());
        responseTbl.unregister(responseCallback);

        // The server returns the set of chunks in unspecified order; we sort by
        // ascending chunk ID, on the assumption that this agrees with the correct
        // order for the chunks in a file
        List<Integer> sortedChunks = new ArrayList<Integer>(chunkSet);
        Collections.sort(sortedChunks);
        return sortedChunks;
	}

	public Set<String> getChunkLocations(final int chunkId) {
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
        TableName tblName = new TableName("bfs", "start_request");
        TupleSet req = new TupleSet(tblName);
        req.add(new Tuple(Conf.getSelfAddress(), requestId,
                          "ChunkLocations", Integer.toString(chunkId)));
        try {
        	this.system.schedule("bfs", tblName, req, null);
        } catch (UpdateException e) {
        	throw new RuntimeException(e);
        }

        Set<String> nodeSet = (Set<String>) waitForResponse(Conf.getFileOpTimeout());
        responseTbl.unregister(responseCallback);
        return Collections.unmodifiableSet(nodeSet);
	}

    private Table registerCallback(Callback callback, String tableName) {
        Table table = this.system.catalog().table(new TableName("bfs", tableName));
        table.register(callback);
        return table;
    }

    // XXX: this should be rewritten to account for the fact that masters can
	// die and then resume operation; we should be willing to try to contact
    // them again.
    private Object waitForResponse(long timeout) {
        while (this.currentMaster < Conf.getNumMasters()) {
            Object result = this.responseQueue.get(timeout);
            if (result != null)
                return result;

            System.out.println("Master #" + this.currentMaster +
            		           "(" + Conf.getMasterAddress(this.currentMaster) +
            		           ") timed out. Retry?");
            this.currentMaster++;
            if (this.currentMaster == Conf.getNumMasters())
            	break;
            updateMasterAddr();
        }

        throw new RuntimeException("BFS request timed out");
    }

    private void updateMasterAddr() {
        TupleSet master = new TupleSet();
        master.add(new Tuple(Conf.getSelfAddress(),
                             Conf.getMasterAddress(this.currentMaster)));
        try {
            this.system.schedule("bfs", MasterTable.TABLENAME, master, null);
            this.system.evaluate();
        } catch (UpdateException e) {
            throw new RuntimeException(e);
        } catch (JolRuntimeException e) {
        	throw new RuntimeException(e);
        }
    }

    private int generateId() {
        return rand.nextInt();
    }
}
