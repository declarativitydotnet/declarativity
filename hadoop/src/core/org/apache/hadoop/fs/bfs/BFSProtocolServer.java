package org.apache.hadoop.fs.bfs;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.util.List;
import java.util.Set;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.FileStatus;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.fs.permission.FsPermission;
import org.apache.hadoop.ipc.RPC;
import org.apache.hadoop.net.NetUtils;

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

	/**
	 * Get a new instance of the BFS client library. If this is invoked inside a
	 * process that runs tasks, the client library is implemented by talking to
	 * a JOL instance running in the task tracker; otherwise, the library is
	 * implemented with an in-process JOL instance. The interface is the same
	 * regardless.
	 */
	public static BFSClientProtocol getInstance(Configuration conf) {
		String taskJVM = System.getProperty("hadoop.taskJVM");
		if (taskJVM == null) {
			System.out.println("taskJVM is null; creating in-process BFSClientProtocol");
			return new BFSProtocolServer();
		} else {
			System.out.println("taskJVM = " + taskJVM);
			String bfsAddress = conf.get("fs.bfs.jolServer.address");
			InetSocketAddress addr = NetUtils.createSocketAddr(bfsAddress);
			try {
				return (BFSClientProtocol) RPC.getProxy(BFSClientProtocol.class,
						                                BFSClientProtocol.versionID,
						                                addr, conf);
			} catch (IOException e) {
				throw new RuntimeException(e);
			}
		}
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
	public FileStatus[] getDirListing(String pathName) {
		Set<BFSFileInfo> bfsListing = this.bfs.getDirListing(pathName);
		if (bfsListing == null)
			return null;

		// XXX: ugly. We need to convert the BFS data structure to the Hadoop
		// file info format manually
		FileStatus[] result = new FileStatus[bfsListing.size()];
		int i = 0;
		for (BFSFileInfo bfsInfo : bfsListing) {
			// XXX: terrible hack. "Ls" file sizes are broken right now.
			FileStatus fStatus = getFileStatus(bfsInfo.getPath());
			if (fStatus == null)
				throw new RuntimeException("race condition in getDirListing()");
			result[i++] = fStatus;
		}

		return result;
	}

	@Override
	public FileStatus getFileStatus(String pathName) {
		BFSFileInfo bfsInfo = this.bfs.getFileInfo(pathName);
		if (bfsInfo == null)
			return null;

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
		List<BFSChunkInfo> chunkList = this.bfs.getChunkList(path);
		BFSChunkInfo[] result = new BFSChunkInfo[chunkList.size()];
		return chunkList.toArray(result);
	}

	@Override
	public String[] getChunkLocations(String path, int chunkId) {
		Set<String> chunkLocs = this.bfs.getChunkLocations(path, chunkId);
		String[] result = new String[chunkLocs.size()];
		return chunkLocs.toArray(result);
	}

	@Override
	public void shutdown() {
		this.bfs.shutdown();
	}

	@Override
	public String getSelfDataNodeAddr() {
		return this.bfs.getSelfDataNodeAddr();
	}
}
