package org.apache.hadoop.fs.bfs;

import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.Set;

import org.apache.hadoop.fs.FileStatus;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.fs.permission.FsPermission;
import org.apache.hadoop.ipc.RPC.Server;
import org.apache.hadoop.mapred.TaskUmbilicalProtocol;

import bfs.BFSChunkInfo;
import bfs.BFSClient;
import bfs.BFSFileInfo;
import bfs.BFSNewChunkInfo;

public class BFSProtocolServer implements BFSClientProtocol {
	private BFSClient bfs;

	public BFSProtocolServer() {
        // Start JOL on an ephemeral local TCP port
		// XXX: make the JOL driver thread a daemon thread. This is to
		// workaround the apparent fact that the Hadoop task execution code
		// does not always invoke FileSystem#close(). Not clear that this is
		// necessary if JOL is running in the TaskTracker...
		this.bfs = new BFSClient(true);
	}

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
		return this.bfs.createDir(pathName);
	}

	@Override
	public boolean createFile(String pathName) {
		return this.bfs.createFile(pathName);
	}

	@Override
	public boolean delete(String pathName) {
		return this.bfs.delete(pathName);
	}

	@Override
	public FileStatus[] getDirListing(String pathName) throws IOException {
		Set<BFSFileInfo> bfsListing = this.bfs.getDirListing(pathName);
		// XXX: illegal per hadoop rpc?
		if (bfsListing == null)
			return null;

		// XXX: ugly. We need to convert the BFS data structure to the Hadoop
		// file info format manually
		FileStatus[] result = new FileStatus[bfsListing.size()];
		int i = 0;
		for (BFSFileInfo bfsInfo : bfsListing) {
			// XXX: terrible hack. "Ls" file sizes are broken right now.
			FileStatus fStatus = getFileStatus(bfsInfo.getPath());
			result[i++] = fStatus;
		}

		return result;
	}

	@Override
	public FileStatus getFileStatus(String pathName) throws IOException {
		BFSFileInfo bfsInfo = this.bfs.getFileInfo(pathName);
		if (bfsInfo == null)
			throw new FileNotFoundException("File does not exist: " + pathName);

		// XXX: ugly. Need to convert from BFS struct to Hadoop struct
		return new FileStatus(bfsInfo.getLength(),
							  bfsInfo.isDirectory(),
							  bfsInfo.getReplication(),
							  bfsInfo.getChunkSize(),
							  0,    // modification time
							  FsPermission.getDefault(),
							  bfsInfo.getOwner(),
							  bfsInfo.getGroup(),
							  new Path(bfsInfo.getPath()));
	}

	@Override
	public boolean rename(String oldPath, String newPath) {
		return this.bfs.rename(oldPath, newPath);
	}

	@Override
	public BFSNewChunkInfo createNewChunk(String pathName) {
		return this.bfs.createNewChunk(pathName);
	}

	@Override
	public BFSChunkInfo[] getChunkList(String path) {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public String[] getChunkLocations(String path, int id) {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public void shutdown() {
		// TODO Auto-generated method stub
	}

	@Override
	public String getSelfDataNodeAddr() {
		return this.bfs.getSelfDataNodeAddr();
	}
}
