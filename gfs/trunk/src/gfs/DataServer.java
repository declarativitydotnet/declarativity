package gfs;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.EOFException;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;

public class DataServer implements Runnable {
    private class DataWorker implements Runnable {
        private Socket socket;
        private String clientAddr;
        private DataInputStream in;
        private DataOutputStream out;

        DataWorker(Socket socket) throws IOException {
            this.socket = socket;
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
                        if (!this.socket.isClosed())
                            this.socket.close();

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
            out.writeInt((int) blockFile.length());

            FileInputStream fis = new FileInputStream(blockFile);
            int remaining = (int) blockFile.length();
        }
    }

    private String fsRoot;
    private ThreadGroup workers;
    private ServerSocket serverSocket;

    DataServer(int port, String fsRoot) {
        this.fsRoot = fsRoot;
        this.workers = new ThreadGroup("DataWorkers");

        try {
            this.serverSocket = new ServerSocket(port);
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }

    public void run() {
        while (true) {
            try {
                Socket socket = this.serverSocket.accept();

                DataWorker dw = new DataWorker(socket);
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
