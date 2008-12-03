package gfs;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.EOFException;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.net.InetSocketAddress;
import java.net.ServerSocket;
import java.net.Socket;
import java.nio.channels.FileChannel;
import java.nio.channels.ServerSocketChannel;
import java.nio.channels.SocketChannel;

import gfs.Shell;
import jol.types.basic.ValueList;

public class DataServer implements Runnable {
    private class DataWorker implements Runnable {
        private SocketChannel channel;
        private String clientAddr;
        private DataInputStream in;
        private DataOutputStream out;

        DataWorker(SocketChannel channel) throws IOException {
            channel.configureBlocking(true);
            this.channel = channel;
            Socket socket = channel.socket();
            this.clientAddr = socket.toString();
            this.in = new DataInputStream(socket.getInputStream());
            this.out = new DataOutputStream(socket.getOutputStream());
        }

        public void run() {
            while (true) {
                try {
                    try {
                        readCommand();
                    } catch (EOFException e) {
                        this.channel.close();
                        System.out.println("got EOF from " + this.clientAddr);
                        break;
                    }
                } catch (IOException e) {
                    throw new RuntimeException(e);
                }
            }
        }

        private void readCommand() throws IOException {
            byte opCode = this.in.readByte();

            switch (opCode) {
            case DataProtocol.READ_OPERATION:
                doReadOperation();
                break;
            case DataProtocol.WRITE_OPERATION:
                doWriteOperation();
                break;
            default:
                throw new IOException("Unrecognized opcode: " + opCode);
            }
        }

        private void doWriteOperation() {
            // TODO Auto-generated method stub
            java.lang.System.out.println("WRITE OP\n");

            try {
                int chunkId = readChunkId();
                ValueList<String> path = readHeaders();
                path.insert(String.valueOf(chunkId));

                File newf = getNewChunk(chunkId);
                java.lang.System.out.println("Ready to read file in\n");

                FileChannel fc = new FileOutputStream(newf).getChannel();
                fc.transferFrom(this.channel, 0, Conf.getChunkSize());
                fc.close();
               
                // we are not pipelining yet. 
                if (path.size() > 1)  {
                    java.lang.System.out.println("path size was " + path.size());
                    copyToNext(newf, path);
                }
                 
                java.lang.System.out.println("done writing chunk\n");
                
            } catch (Exception e) {
                throw new RuntimeException(e);
            }
        }

        private void copyToNext(File f, ValueList<String> path) {
            try {
                Socket sock = Shell.setupStream(path.get(0));
                SocketChannel chan = sock.getChannel();
                if (chan == null) { 
                    java.lang.System.out.println("OHNO\n");
                }
                DataOutputStream dos = new DataOutputStream(sock.getOutputStream());
                Shell.sendRoutedData(dos, path);
                java.lang.System.out.println("routed data sent\n");
                FileChannel fc = new FileInputStream(f).getChannel();
                java.lang.System.out.println("got fc\n" + fc.toString());
                fc.transferTo(0, Conf.getChunkSize(), chan); 
                fc.close();
                sock.close();
            } catch (Exception e) {
                throw new RuntimeException(e);
            }
        }

        private int dataTransfer(int chunkId, String[] path) {
            final int bufferSize = 8192;
            try {
                File newf = getNewChunk(chunkId);
                FileChannel fc = new FileOutputStream(newf).getChannel();
                int sent = 0;
                while (sent < Conf.getChunkSize()) { 
                    fc.transferFrom(this.channel, sent, bufferSize);
                    sent += bufferSize;
                }
                fc.close();
                 
            } catch (Exception e) {
                throw new RuntimeException(e);
            }
            return 0;
        }
    
        private int readChunkId() {
            try {
                return this.in.readInt();
            } catch (Exception e) {
                throw new RuntimeException(e);
            }
        }
        private ValueList<String> readHeaders() {
            String[] path = null;
            // clean me later
            try {
                int sourceRouteListLen = this.in.readInt();
                java.lang.System.out.println("LISTLEN: "+sourceRouteListLen);
                if (sourceRouteListLen > 0) {
                    path = new String[sourceRouteListLen+1];
                    int i = 0;
                    path[i] = "";
                    for (char b = this.in.readChar(); b != ';' && i < sourceRouteListLen; b = this.in.readChar()) {
                        if (b == '|') {
                            i++;
                            path[i] = "";
                        } else { 
                            path[i] = path[i] + b;
                        }
                    }
                    for (String s : path) {
                        java.lang.System.out.println("APTH: "+s);
                    } 
                    
                } else {
                    if (this.in.readChar() != ';') {
                        throw new RuntimeException("invalid header");
                    }
                }

                ValueList<String> ret = new ValueList();
                for (int i=0; i < sourceRouteListLen; i++) {
                    ret.add(path[i]);
                }
                return ret;
            } catch (Exception e) {
                throw new RuntimeException(e);
            }
    
        }

        private void doReadOperation() throws IOException {
            int blockId = this.in.readInt();
            File blockFile = getBlockFile(blockId);

            int fileSize = (int) blockFile.length();
            out.writeInt(fileSize);

            FileChannel fc = new FileInputStream(blockFile).getChannel();
            long nwrite = fc.transferTo(0, fileSize, this.channel);
            if (nwrite != (long) fileSize)
                throw new RuntimeException("Failed to write expected file " +
                                           "size: wrote " + nwrite + ", expected " +
                                           fileSize);
            fc.close();
        }
    }

    private String fsRoot;
    private ThreadGroup workers;
    private ServerSocketChannel listener;
    private ServerSocket serverSocket;

    DataServer(int port, String fsRoot) {
        this.fsRoot = fsRoot;
        this.workers = new ThreadGroup("DataWorkers");

        try {
            InetSocketAddress listenAddr = new InetSocketAddress(port);
            this.listener = ServerSocketChannel.open();
            this.serverSocket = this.listener.socket();
            this.serverSocket.setReuseAddress(true);
            this.serverSocket.bind(listenAddr);
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }

    public void run() {
        while (true) {
            try {
                SocketChannel channel = this.listener.accept();

                DataWorker dw = new DataWorker(channel);
                Thread t = new Thread(this.workers, dw);
                t.start();
            } catch (IOException e) {
                throw new RuntimeException(e);
            }
        }
    }


    public File getNewChunk(int chunkId) {
        String filename = this.fsRoot + File.separator + chunkId;
        File newf = new File(filename);
        try {
            if (!newf.createNewFile()) {
                throw new RuntimeException("failed to create fresh file "+filename);
            }
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
        return newf;
    }

    public File getBlockFile(int blockId) {
        String filename = this.fsRoot + File.separator + blockId;
        File file = new File(filename);
        if (!file.exists())
            throw new RuntimeException("Block not found: " + blockId);

        if (!file.isFile())
            throw new RuntimeException("Block file is not a normal file: " + file);

        return file;
    }
}
