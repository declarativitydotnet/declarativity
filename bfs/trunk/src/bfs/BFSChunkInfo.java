package bfs;

import java.io.Serializable;

public class BFSChunkInfo implements Serializable, Comparable<BFSChunkInfo> {
	private static final long serialVersionUID = 1L;

	private final int chunkId;
	private final int length;

	public BFSChunkInfo(int chunkId, int length) {
		this.chunkId = chunkId;
		this.length = length;
	}

	public int getId() {
		return this.chunkId;
	}

	public int getLength() {
		return this.length;
	}

	public int compareTo(BFSChunkInfo o) {
		if (this.chunkId < o.chunkId)
			return -1;
		if (this.chunkId > o.chunkId)
			return 1;
		if (this.length < o.length)
			return -1;
		if (this.length > o.length)
			return 1;

		return 0;
	}

	public boolean equals(Object o) {
		if (!(o instanceof BFSChunkInfo))
			return false;

		BFSChunkInfo other = (BFSChunkInfo) o;
		return (compareTo(other) == 0);
	}

	public int hashCode() {
		return this.chunkId ^ this.length;
	}

	public String toString() {
		return "Chunk #" + this.chunkId;
	}
}
