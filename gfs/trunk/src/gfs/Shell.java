package gfs;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.net.SocketAddress;
import java.nio.ByteBuffer;
import java.nio.channels.SocketChannel;
import java.util.Arrays;
import java.util.Collections;
import java.util.LinkedList;
import java.util.List;
import java.util.Random;

import jol.core.Runtime;
import jol.core.System;
import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.basic.ValueList;
import jol.types.exception.JolRuntimeException;
import jol.types.exception.UpdateException;
import jol.types.table.Table;
import jol.types.table.TableName;
import jol.types.table.Table.Callback;


public class Shell {
    private int currentMaster;
    private System system;
    private Random rand;
    private SimpleQueue responseQueue;

    /*
     * TODO:
     *  (1) connect to an instance of JOL
     *  (2) parse command-line argument into command + arguments
     *  (3) inject the appropriate inserts into JOL; wait for the results to come back
     *  (4) return results to stdout
     */
    public static void main(String[] args) throws Exception {
        Shell shell = new Shell();
        List<String> argList = new LinkedList<String>(Arrays.asList(args));

        if (argList.size() == 0)
            shell.usage();

        String op = argList.remove(0);
        if (op.equals("append")) {
            shell.doAppend(argList);
        } else if (op.equals("read")) {
            shell.doRead(argList);
        } else if (op.equals("cat")) {
            shell.doConcatenate(argList);
        } else if (op.equals("create")) {
            shell.doCreateFile(argList, true);
        } else if (op.equals("ls")) {
            ValueList<String> list = shell.doListFiles(argList);
            java.lang.System.out.println("ls:");
            int i = 1;
            for (String file : list) {
                java.lang.System.out.println("  " + i + ". " + file);
                i++;
            }
        } else if (op.equals("rm")) {
            shell.doRemove(argList);
        } else {
            shell.usage();
        }

        shell.shutdown();
    }

    public Shell() throws JolRuntimeException, UpdateException {
        this.rand = new Random();
        this.responseQueue = new SimpleQueue();
        this.currentMaster = 0;

        /* Identify the address of the local node */
        /* this is necessary for current tests, but may not be done this way in the future */
        String port = java.lang.System.getenv("PORT");
        if (port == null) {
            port = "5501";
        }
        Conf.setSelfAddress("tcp:localhost:" + port);
        this.system = Runtime.create(Integer.valueOf(port));

        this.system.install("gfs_global", ClassLoader.getSystemResource("gfs/gfs_global.olg"));
        this.system.evaluate();
        this.system.install("gfs", ClassLoader.getSystemResource("gfs/gfs.olg"));
        this.system.evaluate();

        scheduleNewMaster();
        this.system.start();
    }

    private void doAppend(List<String> args) throws UpdateException {
        if (args.size() != 1)
            usage();

        String filename = args.get(0);

        ValueList<String> chunks = getNewChunk(filename);
        sendRoutedData(chunks);

    }

    private void doRead(List<String> args) throws UpdateException {
        if (args.size() != 1)
            usage();

        String filename = args.get(0);

        /* Ask a master node for the list of chunks */
        List<Integer> chunks = getChunkList(filename);
        java.lang.System.out.println("chunks = " + chunks);

        /*
         * For each chunk, ask the master node for a list of data nodes that
         * hold the chunk, and then read the chunk's contents from one or
         * more of those nodes.
         */
        for (Integer chunk : chunks) {
            List<String> locations = getChunkLocations(chunk);
            readChunk(chunk, locations);
        }
    }

    private ValueList getNewChunk(final String filename) throws UpdateException {
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
        TableName tblName = new TableName("gfs", "start_request");
        TupleSet req = new TupleSet(tblName);
        req.add(new Tuple(Conf.getSelfAddress(), requestId, "NewChunk", filename));
        this.system.schedule("gfs", tblName, req, null);

        ValueList chunkList = (ValueList) this.responseQueue.get(); // XXX: timeout?
        responseTbl.unregister(responseCallback);
        return chunkList;
    }


    private ValueList getChunkList(final String filename) throws UpdateException {
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
        TableName tblName = new TableName("gfs", "start_request");
        TupleSet req = new TupleSet(tblName);
        req.add(new Tuple(Conf.getSelfAddress(), requestId, "ChunkList", filename));
        this.system.schedule("gfs", tblName, req, null);

        ValueList chunkList = (ValueList) this.responseQueue.get(); // XXX: timeout?
        responseTbl.unregister(responseCallback);
        return chunkList;
    }

    private Table registerCallback(Callback callback, String tableName) {
        Table table = this.system.catalog().table(new TableName("gfs", tableName));
        table.register(callback);
        return table;
    }

    private List<String> getChunkLocations(final Integer chunk) throws UpdateException {
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

                        Object nodeList = t.value(4);
                        responseQueue.put(nodeList);
                        break;
                    }
                }
            }
        };
        Table responseTbl = registerCallback(responseCallback, "response");

        // Create and insert the request tuple
        TableName tblName = new TableName("gfs", "start_request");
        TupleSet req = new TupleSet(tblName);
        req.add(new Tuple(Conf.getSelfAddress(), requestId,
                          "ChunkLocations", chunk.toString()));
        this.system.schedule("gfs", tblName, req, null);

        ValueList nodeList = (ValueList) this.responseQueue.get(); // XXX: timeout?
        responseTbl.unregister(responseCallback);
        return nodeList;
    }

    private void readChunk(Integer chunk, List<String> locations) {
        for (String loc : locations) {
            StringBuilder sb = readChunkFromAddress(chunk, loc);
            if (sb != null) {
                java.lang.System.out.println("Content of chunk " +
                                             chunk + ": " + sb);
                return;
            }
        }

        throw new RuntimeException("Failed to read chunk " + chunk);
    }

    private void sendRoutedData(ValueList l) {
        try {
            String addr = (String) l.get(0);
            String[] parts = addr.split(":");
            String host = parts[1];
            int controlPort = Integer.parseInt(parts[2]);
            int dataPort = Conf.findDataNodeDataPort(host, controlPort);

            java.lang.System.out.println("Connecting to: " + host + ":" + dataPort);
            Socket sock = new Socket(host, dataPort);
            DataOutputStream dos = new DataOutputStream(sock.getOutputStream());
            dos.writeByte(DataProtocol.WRITE_OPERATION);
            // the last element of the valuelist is our new chunkid
            dos.writeInt(Integer.valueOf((String) l.get(l.size() - 1)));
            // the real size of the list is the list, minus the address we just contacted and the chunkid.
            dos.writeInt(l.size() - 2);
            for (int i = 1; i < l.size() - 2; i++) {
                dos.writeChars((String)l.get(i));
            }
            // then write the actual data

        } catch (Exception e) {
            java.lang.System.out.println("Exception reading chunk " +
                                          e.toString());
            return;
        }
    }

    private StringBuilder readChunkFromAddress(Integer chunkId, String addr) {
        try {
            String[] parts = addr.split(":");
            String host = parts[1];
            int controlPort = Integer.parseInt(parts[2]);
            int dataPort = Conf.findDataNodeDataPort(host, controlPort);

            java.lang.System.out.println("Connecting to: " + host + ":" + dataPort);
            SocketAddress sockAddr = new InetSocketAddress(host, dataPort);
            SocketChannel inChannel = SocketChannel.open();
            inChannel.configureBlocking(true);
            inChannel.connect(sockAddr);

            Socket sock = inChannel.socket();
            DataOutputStream dos = new DataOutputStream(sock.getOutputStream());
            DataInputStream dis = new DataInputStream(sock.getInputStream());

            dos.writeByte(DataProtocol.READ_OPERATION);
            dos.writeInt(chunkId.intValue());
            int length = dis.readInt();

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

            sock.close();
            return sb;
        } catch (Exception e) {
            java.lang.System.out.println("Exception reading chunk from " +
                                         addr + ": " + e.toString());
            e.printStackTrace();
            return null;
        }
    }

    private void doConcatenate(List<String> args) throws UpdateException {
        if (args.isEmpty())
            usage();

        for (String file : args)
            doCatFile(file);
    }

    private void scheduleNewMaster() throws JolRuntimeException {
        TupleSet master = new TupleSet();
        master.add(new Tuple(Conf.getSelfAddress(),
                             Conf.getMasterAddress(this.currentMaster)));
        try {
            this.system.schedule("gfs", MasterTable.TABLENAME, master, null);
        } catch (UpdateException e) {
            throw new JolRuntimeException(e);
        }
        this.system.evaluate();
    }

    private void doCatFile(final String file) throws UpdateException {
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

                        if (success.booleanValue()) {
                            ValueList chunks = (ValueList) t.value(4);
                            java.lang.System.out.println("File name: " + file);
                            java.lang.System.out.println("Chunks: " + chunks.toString());
                            java.lang.System.out.println("=============");
                        } else {
                            String errMessage = (String) t.value(4);
                            java.lang.System.out.println("ERROR on \"cat\":");
                            java.lang.System.out.println("File name: " + file);
                            java.lang.System.out.println("Error message: " + errMessage);
                        }
                        responseQueue.put("done");
                        break;
                    }
                }
            }
        };
        Table responseTbl = registerCallback(responseCallback, "response");

        // Create and insert the request tuple
        TableName tblName = new TableName("gfs", "start_request");
        TupleSet req = new TupleSet(tblName);
        req.add(new Tuple(Conf.getSelfAddress(), requestId, "Cat", file));
        this.system.schedule("gfs", tblName, req, null);

        // Wait for the response
        Object obj = this.responseQueue.get();
        responseTbl.unregister(responseCallback);
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
                while ((b = java.lang.System.in.read()) != -1)
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
                            java.lang.System.out.println("Create succeeded.");
                        else
                            java.lang.System.out.println("Create failed.");

                        responseQueue.put(success);
                        break;
                    }
                }
            }
        };
        Table responseTbl = registerCallback(responseCallback, "response");

        // Create and insert the request tuple
        TableName tblName = new TableName("gfs", "start_request");
        TupleSet req = new TupleSet(tblName);
        req.add(new Tuple(Conf.getSelfAddress(), requestId, "Create", filename));
        this.system.schedule("gfs", tblName, req, null);

        // Wait for the response
        while (true) {
            Object obj = timedGet(Conf.getFileOpTimeout());
            if (obj != null)
                break;
          // we timed out.
          java.lang.System.out.println("retrying (master indx = " + this.currentMaster + ")\n");
        }
        responseTbl.unregister(responseCallback);
    }

    public ValueList<String> doListFiles(List<String> args) throws UpdateException, JolRuntimeException {
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
        TableName tblName = new TableName("gfs", "start_request");
        TupleSet req = new TupleSet(tblName);
        req.add(new Tuple(Conf.getSelfAddress(), requestId, "Ls", null));
        this.system.schedule("gfs", tblName, req, null);

        Object obj = timedGet(Conf.getListingTimeout());
        responseTbl.unregister(responseCallback);
        if (obj == null)
            return doListFiles(args);

        ValueList<String> lsContent = (ValueList<String>) obj;
        Collections.sort(lsContent);
        return lsContent;
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
                        java.lang.System.out.println("Remove of file \"" + file + "\": " +
                                (success.booleanValue() ? "succeeded" : "failed"));
                        responseQueue.put(success);
                        break;
                    }
                }
            }
        };
        Table responseTbl = registerCallback(responseCallback, "response");

        // Create and insert the request tuple
        TableName tblName = new TableName("gfs", "start_request");
        TupleSet req = new TupleSet(tblName);
        req.add(new Tuple(Conf.getSelfAddress(), requestId, "Rm", file));
        this.system.schedule("gfs", tblName, req, null);

        // Wait for the response
        Object obj = timedGet(Conf.getFileOpTimeout());
        responseTbl.unregister(responseCallback);
        if (obj == null)
           doRemoveFile(file);
    }

    private Object timedGet(long timeout) throws JolRuntimeException {
        Object result = this.responseQueue.get(timeout);
        if (result != null)
            return result;

        // We timed out
        this.currentMaster++;
        if (this.currentMaster == Conf.getNumMasters()) {
            java.lang.System.out.println("giving up\n");
            java.lang.System.exit(1);
        }
        scheduleNewMaster();
        return null;
    }

    public void shutdown() {
        this.system.shutdown();
    }

    private void usage() {
        java.lang.System.err.println("Usage: java gfs.Shell op_name args");
        java.lang.System.err.println("Where op_name = {append,cat,create,ls,read,rm}");

        shutdown();
        java.lang.System.exit(0);
    }
}
