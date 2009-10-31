import java.util.LinkedList;
import java.util.List;

public class PerfRunner {
	private final int nClients;
	private final List<PaxosClient> clients;

	public PerfRunner(int nClients) {
		this.nClients = nClients;
		this.clients = new LinkedList<PaxosClient>();

		for (int i = 0; i < this.nClients; i++) {
			this.clients.add(new PaxosClient(i, this.nClients));
		}
	}

	public void run() {
		for (PaxosClient c : this.clients) {
			c.start();
		}
	}
}
