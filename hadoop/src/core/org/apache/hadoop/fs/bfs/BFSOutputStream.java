package org.apache.hadoop.fs.bfs;

import java.io.IOException;
import java.io.OutputStream;
import java.nio.ByteBuffer;
import java.util.LinkedList;
import java.util.List;

import bfs.BFSClient;
import bfs.BFSNewChunkInfo;
import bfs.Conf;
import bfs.DataConnection;

public class BFSOutputStream extends OutputStream {
	private String path;
	private BFSClient bfs;
	private boolean isClosed;
	private ByteBuffer buf;

	public BFSOutputStream(String path, BFSClient bfs) {
		this.path = path;
		this.bfs = bfs;
		this.isClosed = false;
		this.buf = ByteBuffer.allocate(Conf.getChunkSize());
	}

	@Override
	public void write(int b) throws IOException {
		byte[] tmpBuf = new byte[1];
		tmpBuf[0] = (byte) b;
		write(tmpBuf, 0, 1);
	}

	@Override
	public void write(byte[] clientBuf, int offset, int length) throws IOException {
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
	public void flush() throws IOException {
		BFSNewChunkInfo info = this.bfs.getNewChunk(this.path);

        DataConnection conn = new DataConnection(info.getCandidateNodes());
        conn.sendRoutingData(info.getChunkId());
        conn.write(this.buf);
        conn.close();
        this.buf.clear();
	}

	@Override
	public void close() throws IOException {
		if (this.isClosed)
			return;

		flush();
		this.isClosed = true;
	}
}
