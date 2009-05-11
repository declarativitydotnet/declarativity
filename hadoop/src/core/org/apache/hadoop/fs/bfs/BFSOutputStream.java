package org.apache.hadoop.fs.bfs;

import java.io.IOException;
import java.io.OutputStream;
import java.nio.ByteBuffer;

import bfs.BFSNewChunkInfo;
import bfs.Conf;
import bfs.DataConnection;

public class BFSOutputStream extends OutputStream {
	private String path;
	private BFSClientProtocol bfs;
	private boolean isClosed;
	private ByteBuffer buf;

	public BFSOutputStream(String path, BFSClientProtocol bfs) {
		this.path = path;
		this.bfs = bfs;
		this.isClosed = false;
		this.buf = ByteBuffer.allocate(Conf.getChunkSize());
	}

	@Override
	public synchronized void write(int b) throws IOException {
		byte[] tmpBuf = new byte[1];
		tmpBuf[0] = (byte) b;
		write(tmpBuf, 0, 1);
	}

	@Override
	public synchronized void write(byte[] clientBuf, int offset, int length) throws IOException {
		if (this.isClosed)
			throw new IOException("cannot write to closed file");

		int bytesLeft = length;
		int curOffset = offset;
		while (bytesLeft > 0) {
			int toWrite;
			if (bytesLeft > this.buf.remaining())
				toWrite = this.buf.remaining();
			else
				toWrite = bytesLeft;

			this.buf.put(clientBuf, curOffset, toWrite);
			curOffset += toWrite;
			bytesLeft -= toWrite;

			if (!this.buf.hasRemaining())
				flush();
		}
	}

	@Override
	public synchronized void flush() throws IOException {
		// If the buffer is empty, flush() is a no-op
		if (this.buf.remaining() == this.buf.capacity())
			return;

		BFSNewChunkInfo info = this.bfs.createNewChunk(this.path);

        DataConnection conn = new DataConnection(info.getCandidateNodes());
        conn.sendRoutingData(info.getChunkId());
        this.buf.flip();
        conn.write(this.buf);
        conn.close();
        this.buf.clear();
	}

	@Override
	public synchronized void close() throws IOException {
		if (this.isClosed)
			return;

		flush();
		this.isClosed = true;
	}
}
