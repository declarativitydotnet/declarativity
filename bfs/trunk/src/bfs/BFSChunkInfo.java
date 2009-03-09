package bfs;

import java.io.Serializable;
import java.util.Set;

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

	@Override
	public boolean equals(Object o) {
		if (!(o instanceof BFSChunkInfo))
			return false;

		BFSChunkInfo other = (BFSChunkInfo) o;
		return (compareTo(other) == 0);
	}

	@Override
	public int hashCode() {
		return this.chunkId ^ this.length;
	}

	@Override
	public String toString() {
		return "Chunk #" + this.chunkId;
	}

    public static Long setSum(Set<BFSChunkInfo> chunkSet) {
        long result = 0;

        for (BFSChunkInfo cInfo : chunkSet) {
            System.out.println("length: " + cInfo.getLength());
            result += cInfo.getLength();
        }

        return result;
    }
}
