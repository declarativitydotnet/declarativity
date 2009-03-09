package bfs;

import java.io.Serializable;

public class BFSFileInfo implements Serializable, Comparable<BFSFileInfo> {
	private static final long serialVersionUID = 1L;

	private final int fileId;
	private final String path;
	private final String name;
	private final long length;
	private final boolean isDirectory;

	public BFSFileInfo(int fileId, String path, String name, Long length, boolean isDirectory) {
		this.fileId = fileId;
		this.path = path;
		this.name = name;
		this.length = length;
		this.isDirectory = isDirectory;
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
}
