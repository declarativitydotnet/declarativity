package bfs;

public class BfsFileInfo {
	private boolean isDirectory;
	private String fileName;

	public BfsFileInfo(String name) {
		this.fileName = name;
		this.isDirectory = false;
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
}
