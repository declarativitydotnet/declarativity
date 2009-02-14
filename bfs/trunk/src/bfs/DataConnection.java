package bfs;

import java.io.DataOutputStream;
import java.io.IOException;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.net.SocketAddress;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;
import java.nio.channels.SocketChannel;
import java.util.List;

/**
 * This class represents the client side of a connection to a DataNode. It is
 * used by both BfsClients and DataNodes.
 */
public class DataConnection {
    private SocketChannel channel;
    private Socket socket;
    private DataOutputStream dos;
    private String remoteAddr;

    public DataConnection(String nodeAddr) {
        setupStream(nodeAddr);
    }

    private void setupStream(String addr) {
        String[] parts = addr.split(":");
        String host = parts[1];
        int controlPort = Integer.parseInt(parts[2]);
        int dataPort = Conf.findDataNodeDataPort(host, controlPort);
        this.remoteAddr = host + ":" + dataPort;

        try {
            SocketAddress sockAddr = new InetSocketAddress(host, dataPort);
            this.channel = SocketChannel.open();
            this.channel.configureBlocking(true);
            this.channel.connect(sockAddr);
            this.socket = this.channel.socket();
            this.dos = new DataOutputStream(this.socket.getOutputStream());
        } catch (IOException e) {
            throw new RuntimeException("Failed to setup DataConnection with " +
                                       this.remoteAddr, e);
        }
    }

    public void sendRoutingData(int chunkId, List<String> path) {
        try {
            this.dos.writeByte(DataProtocol.WRITE_OPERATION);
            this.dos.writeInt(chunkId);

            int newSize = path.size();
            if (newSize > Conf.getRepFactor() - 1) {
                newSize = Conf.getRepFactor() - 1;
            }

            this.dos.writeInt(newSize);
            for (int i = 0; i < newSize; i++) {
                System.out.println("write " + path.get(i));
                this.dos.writeChars(path.get(i));
                this.dos.writeChar('|');
            }
            this.dos.writeChar(';');
            // caller writes the actual data
        } catch (IOException e) {
            throw new RuntimeException("Exception writing chunk " + chunkId +
                                       " to " + this.remoteAddr, e);
        }
    }

    public void sendChunkContent(FileChannel src) {
        try {
            src.transferTo(0, Conf.getChunkSize(), this.channel);
        } catch (IOException e) {
            throw new RuntimeException("Exception sending chunk content to " +
                                       this.remoteAddr, e);
        }
    }

    public void write(byte[] buf) {
        try {
            this.dos.write(buf);
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }

    public void write(ByteBuffer buf) {
    	try {
    		this.channel.write(buf);
    	} catch (IOException e) {
    		throw new RuntimeException(e);
    	}
    }

    public void close() {
        try {
            this.socket.close();
        } catch (IOException e) {
            throw new RuntimeException("Exception closing data connection to " +
                                       this.remoteAddr, e);
        }
    }
}
