package org.apache.hadoop.fs.bfs;

import java.io.IOException;

import org.apache.hadoop.fs.FileStatus;
import org.apache.hadoop.ipc.RPC.Server;
import org.apache.hadoop.mapred.TaskUmbilicalProtocol;

import bfs.BFSClient;

public class BFSProtocolServer implements BFSClientProtocol {
	private BFSClient bfs = null;

	@Override
	public long getProtocolVersion(String protocol, long clientVersion)
			throws IOException {
		if (protocol.equals(BFSClientProtocol.class.getName())) {
			return BFSClientProtocol.versionID;
		} else {
			throw new IOException("Unknown protocol for task tracker: " + protocol);
		}
	}

	@Override
	public boolean createDir(String pathName) {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean createFile(String pathName) {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean delete(String pathName) {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public FileStatus[] getDirListing(String pathName) {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FileStatus getFileStatus(String pathName) {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public boolean rename(String oldPath, String newPath) {
		// TODO Auto-generated method stub
		return false;
	}
}
