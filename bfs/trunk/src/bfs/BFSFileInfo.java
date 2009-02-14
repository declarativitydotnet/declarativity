package bfs;

import java.io.Serializable;

public class BFSFileInfo implements Serializable, Comparable<BFSFileInfo> {
	private static final long serialVersionUID = 1L;

	private final int fileId;
	private final String fileName;
	private final boolean isDirectory;

	public BFSFileInfo(int fileId, String name) {
		this.fileId = fileId;
		this.fileName = name;
		this.isDirectory = false;
	}

	public int getId() {
		return this.fileId;
	}

	public String getPath() {
		return "/" + this.fileName;
	}

	public String getName() {
		return this.fileName;
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
		return this.fileName.compareTo(o.fileName);
	}

	@Override
	public boolean equals(Object o) {
		if (!(o instanceof BFSFileInfo))
			return false;

		BFSFileInfo fInfo = (BFSFileInfo) o;
		return fInfo.fileName.equals(fInfo.fileName);
	}

	@Override
	public int hashCode() {
		return this.fileName.hashCode();
	}
}
