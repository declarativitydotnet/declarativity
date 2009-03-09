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
	public synchronized void write(int b) throws IOException {
		if (this.isClosed)
			throw new IOException("cannot write to closed file");

		int bytesLeft = 1;
		int curOffset = 0;
		while (bytesLeft > 0) {
			int toWrite;
			if (bytesLeft > this.buf.remaining())
				toWrite = this.buf.remaining();
			else
				toWrite = bytesLeft;

			System.out.println("write(singleton) on " + this.path + ": raw b = " + b + ", low 8 of b = " + (b & 0xFF) + ", as a byte = " + (byte) (b & 0xFF));
			this.buf.put((byte) (b & 0xFF));
			curOffset += toWrite;
			bytesLeft -= toWrite;

			if (!this.buf.hasRemaining())
				flush();
		}
	}

	@Override
	public synchronized void write(byte[] clientBuf, int offset, int length) throws IOException {
		System.out.println("write() on " + this.path);
		for (int i = 0; i < length; i++) {
			System.out.println("Byte " + i + ": " + clientBuf[offset + i]);
		}
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
		BFSNewChunkInfo info = this.bfs.getNewChunk(this.path);

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
