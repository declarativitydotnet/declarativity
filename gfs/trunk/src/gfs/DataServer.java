package gfs;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.EOFException;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.net.InetSocketAddress;
import java.net.ServerSocket;
import java.net.Socket;
import java.nio.channels.FileChannel;
import java.nio.channels.ServerSocketChannel;
import java.nio.channels.SocketChannel;

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
