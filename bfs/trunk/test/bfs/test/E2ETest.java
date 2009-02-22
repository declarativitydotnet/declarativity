package bfs.test;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.net.SocketAddress;
import java.nio.ByteBuffer;
import java.nio.channels.SocketChannel;
import java.util.Arrays;
import java.util.List;
import java.util.Random;
import java.util.Set;

import org.junit.Test;

import bfs.BFSChunkInfo;
import bfs.BFSClient;
import bfs.BFSNewChunkInfo;
import bfs.Conf;
import bfs.DataConnection;
import bfs.DataProtocol;

public class E2ETest extends TestCommon {
	private final static String TEST_FILENAME = "/e2e_f1";
	private final static int NUM_TEST_CHUNKS = 6;

	/* XXX: refactor this to avoid duplicated code */
	@Test(timeout=12000)
	public void test1() throws Exception {
		startMany("localhost:5505");
		startManyDataNodes("td1", "td2");

		BFSClient bfs = new BFSClient(10001);
		safeAssert(bfs.createFile(TEST_FILENAME));

		byte[] testData = new byte[NUM_TEST_CHUNKS * Conf.getChunkSize()];
		Random r = new Random();
		r.nextBytes(testData);

		for (int i = 0; i < NUM_TEST_CHUNKS; i++) {
			BFSNewChunkInfo newChunk = bfs.getNewChunk(TEST_FILENAME);

			DataConnection dc = new DataConnection(newChunk.getCandidateNodes());
			dc.sendRoutingData(newChunk.getChunkId());
			dc.write(testData, i * Conf.getChunkSize(), Conf.getChunkSize());
			dc.close();
		}

		/* Pause to let the DNs send HBs back to the master */
		Thread.sleep(4000);

		List<BFSChunkInfo> chunkList = bfs.getChunkList(TEST_FILENAME);
		safeAssert(chunkList.size() == NUM_TEST_CHUNKS);
		int i = 0;
		byte[] readBuf = new byte[testData.length];
		for (BFSChunkInfo chunk : chunkList) {
			safeAssert(chunk.getLength() == Conf.getChunkSize());

			Set<String> locs = bfs.getChunkLocations(chunk.getId());
			for (String addr : locs) {
				fetchChunkFromAddr(chunk, addr, readBuf, i * Conf.getChunkSize());
				break;
			}
			i++;
		}

		for (int j = 0; j < testData.length; j++) {
			if (testData[j] != readBuf[j])
				throw new RuntimeException("Read buffer differs at offset: " + j);
		}

		shutdown();
	}

	private void fetchChunkFromAddr(BFSChunkInfo chunk, String addr, byte[] outBuf, int outOffset) throws IOException {
        String[] parts = addr.split(":");
        String host = parts[1];
        int controlPort = Integer.parseInt(parts[2]);
        int dataPort = Conf.findDataNodeDataPort(host, controlPort);

        SocketAddress sockAddr = new InetSocketAddress(host, dataPort);
        SocketChannel inChannel = SocketChannel.open();
        inChannel.configureBlocking(true);
        inChannel.connect(sockAddr);

        Socket sock = inChannel.socket();
        DataOutputStream dos = new DataOutputStream(sock.getOutputStream());
        DataInputStream dis = new DataInputStream(sock.getInputStream());

        dos.writeByte(DataProtocol.READ_OPERATION);
        dos.writeInt(chunk.getId());
        int length = dis.readInt();
        if (length != chunk.getLength())
        	throw new RuntimeException("expected length " +
        			                   chunk.getLength() +
        			                   ", data node copy's length is " +
        			                   length + ", chunk " +
        			                   chunk.getId());

        ByteBuffer buf = ByteBuffer.allocate(length);
        while (buf.hasRemaining()) {
        	int nread = inChannel.read(buf);
        	if (nread == -1)
        		throw new IOException("Unexpected EOF from data node");
        }

        sock.close();
        buf.flip();
        buf.get(outBuf, outOffset, length);
	}

    public static void main(String[] args) throws Exception {
        E2ETest t = new E2ETest();
        t.test1();
    }
}