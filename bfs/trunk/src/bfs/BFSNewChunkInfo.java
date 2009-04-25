package bfs;

import java.io.DataInput;
import java.io.DataOutput;
import java.io.IOException;
import java.io.Serializable;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Random;

import org.apache.hadoop.io.Writable;

public class BFSNewChunkInfo implements Serializable, Writable {
	private static final long serialVersionUID = 1L;

	private static final Random random = new Random();

	private int chunkId;
	private List<String> nodeAddrs;

	public BFSNewChunkInfo(int chunkId, List<String> nodeList) {
		this.chunkId = chunkId;
		this.nodeAddrs = Collections.unmodifiableList(BFSNewChunkInfo.randomSelection(nodeList, Conf.getRepFactor()));
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

	private static List<String> randomSelection(List<String> inList, int k) {
		if (inList.size() <= k)
			return inList;

		List<String> result = new ArrayList<String>(k);
		// java.util.Random#nextInt() might not be thread-safe, so be paranoid
		synchronized (random) {
			while (result.size() < k) {
				String candidate = inList.get(random.nextInt(inList.size()));
				if (!result.contains(candidate))
					result.add(candidate);
			}
		}
		return result;
	}

	@Override
	public void readFields(DataInput in) throws IOException {
		this.chunkId = in.readInt();
		int numAddrs = in.readInt();
		this.nodeAddrs = new ArrayList<String>(numAddrs);
		for (int i = 0; i < numAddrs; i++)
			this.nodeAddrs.add(in.readUTF());
	}

	@Override
	public void write(DataOutput out) throws IOException {
		out.writeInt(this.chunkId);
		out.writeInt(this.nodeAddrs.size());
		for (String addr : this.nodeAddrs)
			out.writeUTF(addr);
	}
}
