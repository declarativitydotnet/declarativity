package bfs;

import java.io.BufferedInputStream;
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
import java.nio.channels.ClosedByInterruptException;
import java.nio.channels.ClosedChannelException;
import java.nio.channels.FileChannel;
import java.nio.channels.ServerSocketChannel;
import java.nio.channels.SocketChannel;
import java.util.LinkedList;
import java.util.List;
import java.util.zip.CRC32;
import java.util.zip.CheckedInputStream;

public class DataServer extends Thread {
    /*
     * A DataWorker instance handles a single client connection, and runs in a
     * separate thread from the DataServer.
     */
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
            case DataProtocol.DELETE_OPERATION:
                doDeleteOperation();
                break;
            default:
                throw new IOException("Unrecognized opcode: " + opCode);
            }
        }

        private void doWriteOperation() {
            System.out.println("WRITE OP\n");

            try {
                int chunkId = readChunkId();
                List<String> path = readHeaders();

                File newf = createChunkFile(chunkId);
                System.out.println("Ready to read file in\n");
                FileChannel fc = new FileOutputStream(newf).getChannel();
                fc.transferFrom(this.channel, 0, Conf.getChunkSize());
                fc.close();

                // we are not pipelining yet.
                if (path.size() > 0) {
                    System.out.println("path size was " + path.size());
                    copyToNext(newf, chunkId, path);
                }

                // sadly, for the time being,
                createCRCFile(chunkId, getFileChecksum(newf));
            } catch (Exception e) {
                throw new RuntimeException(e);
            }
        }

        private void doDeleteOperation() {
            try {
                int chunkId = readChunkId();
                File victim = getChunkFile(chunkId);
                victim.delete();
            } catch (Exception e) {
                throw new RuntimeException(e);
            }
        }

        private void copyToNext(File f, int chunkId, List<String> path) {
            try {
                String nextAddr = path.remove(0);
                DataConnection conn = new DataConnection(nextAddr);
                conn.sendRoutingData(chunkId, path);

                FileChannel fc = new FileInputStream(f).getChannel();
                conn.sendChunkContent(fc);
                fc.close();
                conn.close();
            } catch (Exception e) {
                throw new RuntimeException(e);
            }
        }

        private int readChunkId() {
            try {
                return this.in.readInt();
            } catch (Exception e) {
                throw new RuntimeException(e);
            }
        }

        private List<String> readHeaders() {
            String[] path = null;
            // clean me later
            try {
                int sourceRouteListLen = this.in.readInt();
                System.out.println("LISTLEN: " + sourceRouteListLen);
                if (sourceRouteListLen > 0) {
                    path = new String[sourceRouteListLen + 1];
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
                        System.out.println("APTH: " + s);
                    }

                } else {
                    if (this.in.readChar() != ';') {
                        throw new RuntimeException("invalid header");
                    }
                }

                List<String> ret = new LinkedList<String>();
                for (int i = 0; i < sourceRouteListLen; i++) {
                    ret.add(path[i]);
                }
                return ret;
            } catch (Exception e) {
                throw new RuntimeException(e);
            }
        }

        private void doReadOperation() throws IOException {
            int blockId = this.in.readInt();
            File blockFile = getChunkFile(blockId);

            int fileSize = (int) blockFile.length();
            out.writeInt(fileSize);

            FileChannel fc = new FileInputStream(blockFile).getChannel();
            long nwrite = fc.transferTo(0, fileSize, this.channel);
            if (nwrite != (long) fileSize)
                throw new RuntimeException("Failed to write expected file "
                        + "size: wrote " + nwrite + ", expected " + fileSize);
            fc.close();
        }
    }

    private String fsRoot;
    private ThreadGroup workers;
    private ServerSocketChannel listener;
    private boolean inShutdown;

    DataServer(int port, String fsRoot) {
        this.fsRoot = fsRoot;
        this.workers = new ThreadGroup("DataWorkers");
        this.inShutdown = false;

        try {
            this.listener = ServerSocketChannel.open();
            ServerSocket serverSocket = this.listener.socket();
            serverSocket.setReuseAddress(true);

            InetSocketAddress listenAddr = new InetSocketAddress(port);
            serverSocket.bind(listenAddr);
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }

    public void shutdown() {
    	if (this.inShutdown)
    		return;

    	// Notify the DataServer thread that it should shutdown
    	this.inShutdown = true;
    	interrupt();

        System.out.println("active workers: " + this.workers.activeCount() + "\n");
        Thread[] threadList = new Thread[this.workers.activeCount()];
        this.workers.enumerate(threadList);
        for (Thread t : threadList) {
            System.out.println("stop thread " + t.toString() + ")\n");
            t.stop();
        }
        this.workers.destroy();
    }

    public void run() {
		while (true) {
			try {
				if (this.inShutdown) {
					System.out.println("Got shutdown request");
					// shutdown() notifies us that we should shutdown by setting
					// the "inShutdown" field, and then calling interrupt(). If
					// we're inside accept() at the time, interrupt() auto-closes
					// the socket; otherwise, we need to do so by hand.
					if (this.listener.isOpen())
						this.listener.close();
					break;
				}

				SocketChannel channel = this.listener.accept();
				DataWorker dw = new DataWorker(channel);
				Thread t = new Thread(this.workers, dw);
				t.start();
			} catch (ClosedByInterruptException e) {
				if (!this.inShutdown)
					throw new IllegalStateException("unexpected interrupt", e);
				// Otherwise, just rerun the loop and then exit
			} catch (ClosedChannelException e) {
				throw new RuntimeException(e);
			} catch (IOException e) {
				throw new RuntimeException(e);
			}
		}
	}

    public void createCRCFile(int chunkId, long checksum) {
        System.out.println("create file for chunk " + chunkId  + " and checksum " + checksum + "\n");
        String filename = this.fsRoot + File.separator + "checksums" + File.separator
                + chunkId + ".cksum";
        File newf = new File(filename);
        try {
            if (!newf.createNewFile())
                throw new RuntimeException("Failed to create fresh file " + filename);

            FileOutputStream fos = new FileOutputStream(newf);
            // temporary
            fos.write(new Long(checksum).toString().getBytes());
            fos.close();
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
    }

    private long getFileChecksum(File file) {
        try {
            FileInputStream s = new FileInputStream(file);
            CheckedInputStream check = new CheckedInputStream(s, new CRC32());
            BufferedInputStream in = new BufferedInputStream(check);
            while (in.read() != -1) {

            }
            return check.getChecksum().getValue();
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
    }

    public File createChunkFile(int chunkId) {
        String filename = this.fsRoot + File.separator + "chunks" + File.separator
                + chunkId;
        File newf = new File(filename);
        try {
            if (!newf.createNewFile()) {
                throw new RuntimeException("Failed to create fresh file " + filename);
            }
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
        return newf;
    }

    public File getChunkFile(int chunkId) {
        String filename = this.fsRoot + File.separator + "chunks" + File.separator
                + chunkId;
        File file = new File(filename);
        if (!file.exists())
            throw new RuntimeException("Chunk not found: " + chunkId);

        if (!file.isFile())
            throw new RuntimeException("Chunk file is not a normal file: " + file);

        return file;
    }
}
