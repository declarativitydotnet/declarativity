package org.apache.hadoop.fs.bfs;

import java.io.IOException;

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

	FileStatus getFileStatus(String pathName);
	FileStatus[] getDirListing(String pathName);

	BFSChunkInfo[] getChunkList(String path);
	String[] getChunkLocations(String path, int chunkId);
	BFSNewChunkInfo createNewChunk(String pathName);

	String getSelfDataNodeAddr();
	void shutdown();
}
