package bfs;

import java.io.Serializable;
import java.util.Collections;
import java.util.List;

public class BFSNewChunkInfo implements Serializable {
	private static final long serialVersionUID = 1L;

	private final int chunkId;
	private final List<String> nodeAddrs;

	public BFSNewChunkInfo(int chunkId, List<String> nodeAddrs) {
		this.chunkId = chunkId;
		this.nodeAddrs = Collections.unmodifiableList(nodeAddrs);
	}

	public int getChunkId() {
		return this.chunkId;
	}

	public List<String> getNodeCandidates() {
		return this.nodeAddrs;
	}

	@Override
	public String toString() {
		return "BFSNewChunkInfo[#" + chunkId + "; " + nodeAddrs + "]";
	}
}
