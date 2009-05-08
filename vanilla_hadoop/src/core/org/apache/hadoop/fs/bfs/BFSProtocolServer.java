package org.apache.hadoop.fs.bfs;

import java.io.IOException;
import java.util.LinkedList;
import java.util.List;

import org.apache.hadoop.fs.FileStatus;

import bfs.BFSChunkInfo;
import bfs.BFSNewChunkInfo;

public class BFSProtocolServer implements BFSClientProtocol {
	private List<BFSClientProtocol> workers;
	private Object lock;

	public BFSProtocolServer(int nworkers) {
		this.workers = new LinkedList<BFSClientProtocol>();
		this.lock = new Object();

		for (int i = 0; i < nworkers; i++) {
			this.workers.add(new BFSClientWrapper());
		}
	}

	@Override
	public boolean createDir(final String pathName) {
		Boolean result = (Boolean) workerInvoke(new WorkerCallback() {
			public Object invoke(BFSClientProtocol worker) {
				return new Boolean(worker.createDir(pathName));
			}
		});

		return result.booleanValue();
	}

	@Override
	public boolean createFile(final String pathName) {
		Boolean result = (Boolean) workerInvoke(new WorkerCallback() {
			public Object invoke(BFSClientProtocol worker) {
				return new Boolean(worker.createFile(pathName));
			}
		});

		return result.booleanValue();
	}

	@Override
	public BFSNewChunkInfo createNewChunk(final String pathName) {
		return (BFSNewChunkInfo) workerInvoke(new WorkerCallback() {
			public Object invoke(BFSClientProtocol worker) {
				return worker.createNewChunk(pathName);
			}
		});
	}

	@Override
	public boolean delete(final String pathName) {
		Boolean result = (Boolean) workerInvoke(new WorkerCallback() {
			public Object invoke(BFSClientProtocol worker) {
				return new Boolean(worker.delete(pathName));
			}
		});

		return result.booleanValue();
	}

	@Override
	public BFSChunkInfo[] getChunkList(final String path) {
		return (BFSChunkInfo[]) workerInvoke(new WorkerCallback() {
			public Object invoke(BFSClientProtocol worker) {
				return worker.getChunkList(path);
			}
		});
	}

	@Override
	public String[] getChunkLocations(final String path, final int chunkId) {
		return (String[]) workerInvoke(new WorkerCallback() {
			public Object invoke(BFSClientProtocol worker) {
				return worker.getChunkLocations(path, chunkId);
			}
		});
	}

	@Override
	public FileStatus[] getDirListing(final String pathName) {
		return (FileStatus[]) workerInvoke(new WorkerCallback() {
			public Object invoke(BFSClientProtocol worker) {
				return worker.getDirListing(pathName);
			}
		});
	}

	@Override
	public FileStatus getFileStatus(final String pathName) {
		return (FileStatus) workerInvoke(new WorkerCallback() {
			public Object invoke(BFSClientProtocol worker) {
				return worker.getFileStatus(pathName);
			}
		});
	}

	@Override
	public String getSelfDataNodeAddr() {
		return (String) workerInvoke(new WorkerCallback() {
			public Object invoke(BFSClientProtocol worker) {
				return worker.getSelfDataNodeAddr();
			}
		});
	}

	@Override
	public boolean rename(final String oldPath, final String newPath) {
		Boolean result = (Boolean) workerInvoke(new WorkerCallback() {
			public Object invoke(BFSClientProtocol worker) {
				return worker.rename(oldPath, newPath);
			}
		});

		return result.booleanValue();
	}

	@Override
	public void shutdown() {
		// XXX: no-op for now
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

	private Object workerInvoke(WorkerCallback cb) {
		BFSClientProtocol worker = null;

		try {
			worker = getWorker();
			return cb.invoke(worker);
		} finally {
			if (worker != null)
				returnWorker(worker);
		}
	}

	private BFSClientProtocol getWorker() {
		synchronized (lock) {
			if (this.workers.isEmpty())
				return new BFSClientWrapper();
			else
				return this.workers.remove(0);
		}
	}

	private void returnWorker(BFSClientProtocol worker) {
		synchronized (lock) {
			this.workers.add(worker);
		}
	}

	private interface WorkerCallback {
		Object invoke(BFSClientProtocol worker);
	}
}
