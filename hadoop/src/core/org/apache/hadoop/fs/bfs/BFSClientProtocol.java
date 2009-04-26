package org.apache.hadoop.fs.bfs;

import java.io.IOException;
import java.util.List;
import java.util.Set;

import org.apache.hadoop.fs.FileStatus;
import org.apache.hadoop.ipc.VersionedProtocol;

import bfs.BFSChunkInfo;
import bfs.BFSNewChunkInfo;

public interface BFSClientProtocol extends VersionedProtocol {
	static final long versionID = 9L;

	boolean createFile(String pathName);
	boolean createDir(String pathName);
	boolean delete(String pathName);
	boolean rename(String oldPath, String newPath);
	FileStatus getFileStatus(String pathName) throws IOException;
	FileStatus[] getDirListing(String pathName) throws IOException;
	BFSNewChunkInfo createNewChunk(String pathName);
	BFSChunkInfo[] getChunkList(String path);
	String[] getChunkLocations(String path, int id);
	String getSelfDataNodeAddr();
	void shutdown();
}
