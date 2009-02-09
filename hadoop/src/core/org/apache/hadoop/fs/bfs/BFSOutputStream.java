package org.apache.hadoop.fs.bfs;

import java.io.IOException;
import java.io.OutputStream;

import bfs.BFSClient;

public class BFSOutputStream extends OutputStream {
	private String path;
	private BFSClient bfs;
	private boolean isClosed;

	public BFSOutputStream(String path, BFSClient bfs) {
		this.path = path;
		this.bfs = bfs;
		this.isClosed = false;
	}

	@Override
	public void write(int b) throws IOException {
		// TODO Auto-generated method stub
	}

	@Override
	public void write(byte[] b, int off, int len) throws IOException {
		;
	}

	@Override
	public void flush() throws IOException {
		;
	}

	@Override
	public void close() throws IOException {
		if (this.isClosed)
			return;

		this.isClosed = true;
	}
}
