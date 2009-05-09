package bfs;

import java.io.DataInput;
import java.io.DataOutput;
import java.io.IOException;
import java.io.Serializable;
import java.util.Set;

import org.apache.hadoop.io.Writable;

public class BFSChunkInfo implements Comparable<BFSChunkInfo>, Serializable, Writable {
	private static final long serialVersionUID = 1L;

	private int chunkId;
	private int length;

	public BFSChunkInfo(int chunkId, int length) {
		this.chunkId = chunkId;
		this.length = length;
	}

	// XXX: Dummy no-arg constructor required by Hadoop's RPC implementation
	public BFSChunkInfo() {
		this.chunkId = -1;
		this.length = -1;
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
            result += cInfo.getLength();
        }

        return result;
    }

	@Override
	public void readFields(DataInput in) throws IOException {
		this.chunkId = in.readInt();
		this.length = in.readInt();
	}

	@Override
	public void write(DataOutput out) throws IOException {
		out.writeInt(this.chunkId);
		out.writeInt(this.length);
	}
}
