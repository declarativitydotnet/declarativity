package bfs;

import java.io.DataInput;
import java.io.DataOutput;
import java.io.IOException;
import java.io.Serializable;

import org.apache.hadoop.io.Writable;

public class BFSFileInfo implements Comparable<BFSFileInfo>, Serializable, Writable {
	private static final long serialVersionUID = 1L;

	private int fileId;
	private String path;
	private String name;
	private long length;
	private boolean isDirectory;

	public BFSFileInfo(int fileId, String path, String name, Long length, boolean isDirectory) {
		this.fileId = fileId;
		this.path = path;
		this.name = name;
		this.length = length;
		this.isDirectory = isDirectory;
	}

	// XXX: Dummy no-arg constructor required by Hadoop's RPC implementation
	public BFSFileInfo() {
		;
	}

	public int getId() {
		return this.fileId;
	}

	public String getPath() {
		return this.path;
	}

	public String getName() {
		return this.name;
	}

	public boolean isDirectory() {
		return this.isDirectory;
	}

	public int getReplication() {
		return Conf.getRepFactor();
	}

	public int getChunkSize() {
		return Conf.getChunkSize();
	}

	public long getLength() {
		return length;
	}

	// TODO
	public String getOwner() {
		return "bfs";
	}

	// TODO
	public String getGroup() {
		return "bfs";
	}

	public int compareTo(BFSFileInfo o) {
		int result = this.path.compareTo(o.path);
		if (result != 0)
			return result;

		if (o.isDirectory && !this.isDirectory)
			return 1;
		if (!o.isDirectory && this.isDirectory)
			return -1;
		if (o.length > this.length)
			return 1;
		if (o.length < this.length)
			return -1;

		return 0;
	}

	@Override
	public boolean equals(Object o) {
		if (!(o instanceof BFSFileInfo))
			return false;

		BFSFileInfo fInfo = (BFSFileInfo) o;

		if (fInfo.length != this.length)
			return false;
		if (fInfo.isDirectory != this.isDirectory)
			return false;
		return fInfo.path.equals(this.path);
	}

	@Override
	public int hashCode() {
		int h = 71;
		h = (h * 31) + (int) (this.length ^ this.length >>> 32);
		h = (h * 31) + this.path.hashCode();
		if (this.isDirectory)
			h += 73;
		return h;
	}

	@Override
	public String toString() {
		return this.path + "; length = " + this.length;
	}

	@Override
	public void readFields(DataInput in) throws IOException {
		this.fileId = in.readInt();
		this.name = in.readUTF();
		this.path = in.readUTF();
		this.length = in.readLong();
		this.isDirectory = in.readBoolean();
	}

	@Override
	public void write(DataOutput out) throws IOException {
		out.writeInt(this.fileId);
		out.writeUTF(this.name);
		out.writeUTF(this.path);
		out.writeLong(this.length);
		out.writeBoolean(this.isDirectory);
	}
}
