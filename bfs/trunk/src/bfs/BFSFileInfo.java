package bfs;

import java.io.Serializable;
import java.util.List;

public class BFSFileInfo implements Serializable {
	private static final long serialVersionUID = 1L;

	private final boolean isDirectory;
	private final String fileName;
	private final List<BFSChunkInfo> chunkList;

	public BFSFileInfo(String name, List<BFSChunkInfo> chunkList) {
		this.fileName = name;
		this.isDirectory = false;
		this.chunkList = chunkList;
	}

	public boolean isDirectory() {
		return this.isDirectory;
	}

	public String getPath() {
		return "/" + this.fileName;
	}

	public String getName() {
		return this.fileName;
	}

	public int getReplication() {
		return Conf.getRepFactor();
	}

	public long getChunkSize() {
		return Conf.getChunkSize();
	}

	// TODO
	public long getLength() {
		return 0;
	}
}
