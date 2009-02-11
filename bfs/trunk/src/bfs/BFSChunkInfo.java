package bfs;

import java.io.Serializable;

public class BFSChunkInfo implements Serializable {
	private static final long serialVersionUID = 1L;

	private final int chunkId;
	private final int length;

	public BFSChunkInfo(int chunkId, int length) {
		this.chunkId = chunkId;
		this.length = length;
	}

	public int getLength() {
		return this.length;
	}

	public int getId() {
		return this.chunkId;
	}
}
