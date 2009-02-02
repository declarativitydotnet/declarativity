package gfs;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.io.InputStream;
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

import jol.core.JolSystem;
import jol.core.Runtime;
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
    private JolSystem system;
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
        } else if (op.equals("create")) {
            shell.doCreateFile(argList, true);
        } else if (op.equals("ls")) {
            ValueList<String> list = shell.doListFiles(argList);
            System.out.println("ls:");
            int i = 1;
            for (String file : list) {
                System.out.println("  " + i + ". " + file);
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
        String port = System.getenv("PORT");
        if (port == null) {
            port = "5501";
        }

        /* this shouldn't be a static member at all... */
        Conf.setSelfAddress("tcp:localhost:" + port);

        this.system = Runtime.create(Integer.valueOf(port));

        this.system.install("gfs_global", ClassLoader.getSystemResource("gfs/gfs_global.olg"));
        this.system.evaluate();
        this.system.install("gfs", ClassLoader.getSystemResource("gfs/gfs.olg"));
        this.system.evaluate();

        scheduleNewMaster();
        this.system.start();
    }

    public void doAppend(List<String> args) throws UpdateException {
        this.doAppend(args,System.in);
    }

    public void doAppend(List<String> args, InputStream s) throws UpdateException {
        if (args.size() != 1)
            usage();

        String filename = args.get(0);

        int b = 1;
        while (b != -1) {
            ValueList<String> path = getNewChunk(filename);
            String firstAddr = path.remove(0);
            int chunkId = Integer.valueOf(path.remove(path.size() - 1));
            Socket sock = setupStream(firstAddr);
            try {
                DataOutputStream dos = new DataOutputStream(sock.getOutputStream());
                sendRoutedData(dos, chunkId, path);
                int read = 0;
                while (b != -1 && read < Conf.getChunkSize()) {
                    byte buf[] = new byte[Conf.getBufSize()];
                    //b = System.in.read(buf,0,Conf.getBufSize());
                    b = s.read(buf,0,Conf.getBufSize());
                    //read += Conf.getChunkSize();
                    dos.write(buf);
                    read += b;
                }
                System.out.println("exiting inner loop with "+b+" retval and "+read+" bytes read\n");
                dos.close();
            } catch (Exception e) {
                throw new RuntimeException(e);
            }
        }
    }

    private void doRead(List<String> args) throws UpdateException,JolRuntimeException {
        if (args.size() != 1)
            usage();

        String filename = args.get(0);

        /* Ask a master node for the list of chunks */
        List<Integer> chunks = getChunkList(filename);
        System.out.println("chunks = " + chunks);

        /*
         * For each chunk, ask the master node for a list of data nodes that
         * hold the chunk, and then read the chunk's contents from one or
         * more of those nodes.
         */
        for (Integer chunk : chunks) {
            List<String> locations = getChunkLocations(chunk);
            System.out.println("I would read the chunk here.  but I won't");
            //readChunk(chunk, locations);
        }
    }

    private ValueList<String> getNewChunk(final String filename) throws UpdateException {
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

        ValueList<String> chunkList = (ValueList) this.responseQueue.get(); // XXX: timeout?
        responseTbl.unregister(responseCallback);
        return chunkList;
    }


    private ValueList<Integer> getChunkList(final String filename) throws UpdateException,JolRuntimeException {
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

        ValueList chunkList = (ValueList) spinGet(Conf.getListingTimeout());
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
                System.out.println("Content of chunk " +
                                   chunk + ": " + sb);
                return;
            }
        }

        throw new RuntimeException("Failed to read chunk " + chunk);
    }

    public static Socket setupStream(String addr) {
        String[] parts = addr.split(":");
        String host = parts[1];
        int controlPort = Integer.parseInt(parts[2]);

        System.out.println("TEST1 -- " + addr);
        int dataPort = Conf.findDataNodeDataPort(host, controlPort);
        System.out.println("Connecting to: " + host + ":" + dataPort);

        try {
            SocketAddress sockAddr = new InetSocketAddress(host, dataPort);
            SocketChannel inChannel = SocketChannel.open();
            inChannel.configureBlocking(true);
            inChannel.connect(sockAddr);
            Socket sock = inChannel.socket();
            return sock;
        } catch (Exception e) {
            throw new RuntimeException("failed to open socket\n");
        }
    }

    public static void sendRoutedData(DataOutputStream dos, int chunkId, List<String> path) {
        try {
            dos.writeByte(DataProtocol.WRITE_OPERATION);
            dos.writeInt(chunkId);

            int newSize = path.size();
            if (newSize > Conf.getRepFactor() - 1) {
                newSize = Conf.getRepFactor() - 1;
            }

            dos.writeInt(newSize);
            for (int i = 0; i < newSize; i++) {
                System.out.println("write " + path.get(i));
                dos.writeChars(path.get(i));
                dos.writeChar('|');
            }
            dos.writeChar(';');
            // caller writes the actual data
        } catch (Exception e) {
            System.out.println("Exception reading chunk " + e.toString());
            return;
        }
    }

    private StringBuilder readChunkFromAddress(Integer chunkId, String addr) {
        try {
            String[] parts = addr.split(":");
            String host = parts[1];
            int controlPort = Integer.parseInt(parts[2]);

            System.out.println("TEST2\n");
            int dataPort = Conf.findDataNodeDataPort(host, controlPort);

            System.out.println("Reading chunk " + chunkId + " from: " + host + ":" + dataPort);
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
            this.system.schedule("gfs", MasterTable.TABLENAME, master, null);
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
        TableName tblName = new TableName("gfs", "start_request");
        TupleSet req = new TupleSet(tblName);
        req.add(new Tuple(Conf.getSelfAddress(), requestId, "Create", filename));
        this.system.schedule("gfs", tblName, req, null);

        // Wait for the response
        Object obj = spinGet(Conf.getFileOpTimeout());
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

        Object obj = spinGet(Conf.getListingTimeout());
        responseTbl.unregister(responseCallback);

        ValueList<String> lsContent = (ValueList) obj;
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
        TableName tblName = new TableName("gfs", "start_request");
        TupleSet req = new TupleSet(tblName);
        req.add(new Tuple(Conf.getSelfAddress(), requestId, "Rm", file));
        this.system.schedule("gfs", tblName, req, null);

        // Wait for the response
        Object obj = spinGet(Conf.getFileOpTimeout());
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
        System.err.println("Usage: java gfs.Shell op_name args");
        System.err.println("Where op_name = {append,create,ls,read,rm}");

        shutdown();
        System.exit(0);
    }
}
