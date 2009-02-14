package org.apache.hadoop.fs.bfs;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.net.SocketAddress;
import java.nio.ByteBuffer;
import java.nio.channels.SocketChannel;
import java.util.List;
import java.util.Set;

import org.apache.hadoop.fs.FSInputStream;

import bfs.BFSChunkInfo;
import bfs.BFSClient;
import bfs.BFSFileInfo;
import bfs.Conf;
import bfs.DataProtocol;

public class BFSInputStream extends FSInputStream {
	private BFSClient bfs;
	private String path;
	private List<BFSChunkInfo> chunkList;
	private boolean isClosed;
	private boolean atEOF;

	// Byte-wise offset into the logical file contents
	private long position;

	// Index into "chunkList" identifying the current chunk we're positioned at.
	// These fields are updated by updatePosition().
	private int currentChunkIdx;
	private int currentChunkOffset;
	private BFSChunkInfo currentChunk;
	private Set<String> chunkLocations;
	private ByteBuffer buf;

	public BFSInputStream(String path, BFSClient bfs) throws IOException {
		this.bfs = bfs;
		this.path = path;
		this.isClosed = false;
		this.atEOF = false;
		this.chunkList = bfs.getChunkList(path);
		this.buf = ByteBuffer.allocate(Conf.getChunkSize());
		updatePosition(0);
	}

	@Override
	public long getPos() throws IOException {
		return this.position;
	}

	@Override
	public void seek(long newPos) throws IOException {
		updatePosition(newPos);
	}

	private void updatePosition(long newPos) throws IOException {
		if (newPos < 0)
			throw new IOException("cannot seek to negative position");

		int chunkNum = (int) (newPos / Conf.getChunkSize());
		int chunkOffset = (int) (newPos % Conf.getChunkSize());

		if (chunkNum > this.chunkList.size())
			throw new IOException("cannot seek past end of file");

		int chunkLen;
		if (this.chunkList.isEmpty()) {
			chunkLen = 0;
		} else {
			if (chunkNum != this.currentChunkIdx || this.currentChunk == null) {
				this.currentChunk = this.chunkList.get(chunkNum);
				this.chunkLocations = bfs.getChunkLocations(this.currentChunk.getId());
				fetchChunkContent();
			}

			chunkLen = this.currentChunk.getLength();
		}

		if (chunkOffset > chunkLen)
			throw new IOException("cannot seek past end of file");

		if (chunkOffset == chunkLen && chunkNum == this.chunkList.size())
			this.atEOF = true;
		else
			this.atEOF = false;

		this.currentChunkIdx = (int) chunkNum;
		this.currentChunkOffset = (int) chunkOffset;
		this.position = newPos;
	}

	/**
	 * Read the content of "currentChunk" from one of the data nodes in
	 * "chunkLocations".
	 */
	private void fetchChunkContent() throws IOException {
		for (String loc : this.chunkLocations) {
			try {
				fetchChunkFromAddr(loc);
			} catch (IOException e) {
				continue;
			}

			// Read chunk successfully
			return;
		}

		// Failed to read chunk: all DNs for this chunk are down
		throw new IOException("failed to read chunk!");
	}

	private void fetchChunkFromAddr(String addr) throws IOException {
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
        dos.writeInt(this.currentChunk.getId());
        int length = dis.readInt();
        if (length != this.currentChunk.getLength())
        	throw new RuntimeException("expected length " +
        			                   this.currentChunk.getLength() +
        			                   ", data node copy's length is " +
        			                   length + ", chunk " +
        			                   this.currentChunk.getId());

        this.buf.clear();
        this.buf.limit(length);
        while (this.buf.hasRemaining()) {
        	int nread = inChannel.read(this.buf);
        	if (nread == -1)
        		throw new IOException("Unexpected EOF from data node");
        }

        sock.close();

        // We successfully read a chunk, so prepare the buffer to
        // be read from
        this.buf.rewind();
	}

	@Override
	public boolean seekToNewSource(long targetPos) throws IOException {
		// This is a somewhat fraudulent implementation of this API,
		// but it should be sufficient
		updatePosition(targetPos);
		return true;
	}

	@Override
	public int read() throws IOException {
		byte[] tmpBuf = new byte[1];
		int result = read(tmpBuf, 0, 1);
		if (result == -1)
			return result;
		else
			return (int) tmpBuf[0];
	}

	@Override
	public int read(byte[] clientBuf, int offset, int len) throws IOException {
		if (this.isClosed)
			throw new IOException("cannot read from closed file");

		if (this.atEOF)
			return -1;

		// If we get here, there must be some data left to read in
		// the current chunk
		if (!this.buf.hasRemaining())
			throw new IllegalStateException();

		// If there's not enough data in the buffer to completely satisfy
		// the request, just read the remaining data. We then switch
		// to the next chunk in time for the next read() call.
		if (len > this.buf.remaining())
			len = this.buf.remaining();

		this.buf.get(clientBuf, offset, len);
		updatePosition(this.position + len);
		return len;
	}

	@Override
	public void close() throws IOException {
		if (this.isClosed)
			return;

		this.isClosed = true;
	}

	/*
	 * We don't support marks, for now.
	 */
	@Override
	public boolean markSupported() {
		return false;
	}

	@Override
	public void mark(int readLimit) {
		// Do nothing
	}

	@Override
	public void reset() throws IOException {
		throw new IOException("Mark not supported");
	}
}
