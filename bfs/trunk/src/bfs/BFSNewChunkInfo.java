package bfs;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Set;

public class BFSNewChunkInfo implements Serializable {
	private static final long serialVersionUID = 1L;

	private final int chunkId;
	private final List<String> nodeAddrs;

	public BFSNewChunkInfo(int chunkId, Set<String> nodeAddrs) {
		this.chunkId = chunkId;
		List<String> nodeList = new ArrayList<String>(nodeAddrs);
		this.nodeAddrs = Collections.unmodifiableList(nodeList);
	}

	public int getChunkId() {
		return this.chunkId;
	}

	public List<String> getCandidateNodes() {
		return this.nodeAddrs;
	}

	@Override
	public String toString() {
		return "BFSNewChunkInfo[#" + chunkId + "; " + nodeAddrs + "]";
	}
}
