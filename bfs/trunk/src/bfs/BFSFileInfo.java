package bfs;

import java.io.Serializable;

public class BFSFileInfo implements Serializable, Comparable<BFSFileInfo> {
	private static final long serialVersionUID = 1L;

	private final int fileId;
	private final String path;
	private final String name;
	private final boolean isDirectory;

	public BFSFileInfo(int fileId, String path, String name, boolean isDirectory) {
		this.fileId = fileId;
		this.path = path;
		this.name = name;
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

	// TODO
	public long getLength() {
		return 0;
	}

	public int compareTo(BFSFileInfo o) {
		return this.path.compareTo(o.path);
	}

	// XXX: should probably consider "isDirectory"
	@Override
	public boolean equals(Object o) {
		if (!(o instanceof BFSFileInfo))
			return false;

		BFSFileInfo fInfo = (BFSFileInfo) o;
		return fInfo.path.equals(fInfo.path);
	}

	@Override
	public int hashCode() {
		return this.path.hashCode();
	}

	@Override
	public String toString() {
		return this.path;
	}
}
