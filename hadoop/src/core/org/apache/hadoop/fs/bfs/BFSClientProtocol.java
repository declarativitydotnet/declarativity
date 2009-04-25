package org.apache.hadoop.fs.bfs;

import org.apache.hadoop.fs.FileStatus;
import org.apache.hadoop.ipc.VersionedProtocol;

public interface BFSClientProtocol extends VersionedProtocol {
	static final long versionID = 9L;

	boolean createFile(String pathName);
	boolean createDir(String pathName);
	boolean delete(String pathName);
	boolean rename(String oldPath, String newPath);
	FileStatus getFileStatus(String pathName);
	FileStatus[] getDirListing(String pathName);
}
