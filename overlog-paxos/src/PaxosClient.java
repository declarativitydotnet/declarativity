import jol.core.JolSystem;
import jol.types.basic.BasicTupleSet;
import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.exception.JolRuntimeException;
import jol.types.table.TableName;

public class PaxosClient extends Thread {
	private final int id;
	private final int nClients;
	private JolSystem sys;

	PaxosClient(int id, int nClients) {
		this.id = id;
		this.nClients = nClients;
	}

	public void run() {
		try {
			this.sys = jol.core.Runtime.create(jol.core.Runtime.DEBUG_ALL,
					   System.err, Main.makePort(this.id));

			this.sys.install("paxos", ClassLoader.getSystemResource("olg/supplementary/glue1.olg"));
			this.sys.install("paxos", ClassLoader.getSystemResource("olg/supplementary/ident.olg"));
			this.sys.install("paxos", ClassLoader.getSystemResource("olg/core/election.olg"));

			this.sys.install("paxos", ClassLoader.getSystemResource("olg/core/prepare.olg"));
			this.sys.install("paxos", ClassLoader.getSystemResource("olg/core/propose.olg"));
            this.sys.evaluate();
			this.sys.install("paxos", ClassLoader.getSystemResource("olg/supplementary/insertions.olg"));
			this.sys.evaluate();

//			this.sys.install("paxos", ClassLoader.getSystemResource("olg/supplementary/assertions.olg"));
			this.sys.evaluate();
			addMemberList();
			addSelfAddr();

		} catch (JolRuntimeException e) {
			throw new RuntimeException(e);
		}

		this.sys.start();
	}

	private void addMemberList() throws JolRuntimeException {
		TupleSet ts = new BasicTupleSet();

		for (int i = 0; i < this.nClients; i++) {
			Tuple t = new Tuple("tcp:localhost:" + Main.makePort(i), i);
			ts.add(t);
		}

		this.sys.schedule("paxos", new TableName("paxos", "member"), ts, null);
		this.sys.evaluate();
	}

	private void addSelfAddr() throws JolRuntimeException {
		TupleSet ts = new BasicTupleSet();
		ts.add(new Tuple("tcp:localhost:" + Main.makePort(this.id)));
		this.sys.schedule("paxos", new TableName("paxos", "self"), ts, null);
		this.sys.evaluate();
	}
}
