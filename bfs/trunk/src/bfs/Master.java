package bfs;

import jol.core.JolSystem;
import jol.core.Runtime;
import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.exception.JolRuntimeException;
import jol.types.exception.UpdateException;
import jol.types.table.TableName;


public class Master {
    private final String address;
    private final int port;
    private JolSystem system;

    public static void main(String[] args) throws JolRuntimeException, UpdateException {
        int masterIndex = Conf.findSelfIndex(true);
        Master m = new Master(masterIndex);
        m.start();
    }

    public Master(int masterIdx) {
        this.address = Conf.getMasterAddress(masterIdx);
        this.port = Conf.getMasterPort(masterIdx);
    }

    public void stop() {
        this.system.shutdown();
    }

    public void start() throws JolRuntimeException, UpdateException {
        this.system = Runtime.create(Runtime.DEBUG_WATCH, System.err, this.port);

        OlgAssertion olgAssert = new OlgAssertion(this.system, true);
        Tap tap = new Tap(this.system, "tcp:localhost:5678");

        this.system.install("bfs", ClassLoader.getSystemResource("bfs/bfs_global.olg"));
        this.system.evaluate();
        this.system.install("bfs", ClassLoader.getSystemResource("bfs/heartbeats.olg"));
        this.system.evaluate();

        this.system.install("bfs", ClassLoader.getSystemResource("bfs/chunks_global.olg"));
        this.system.evaluate();
        this.system.install("bfs", ClassLoader.getSystemResource("bfs/chunks.olg"));
        this.system.evaluate();

        this.system.install("bfs", ClassLoader.getSystemResource("bfs/bfs.olg"));
        this.system.evaluate();
        this.system.install("bfs", ClassLoader.getSystemResource("bfs/placement.olg"));
        this.system.evaluate();

        setupPaxos();

        this.system.install("bfs", ClassLoader.getSystemResource("bfs/paxos_bfs_glue.olg"));
        this.system.evaluate();

        // Hack: insert a bfs::file tuple to represent the root of the file system
        TupleSet newFile = new TupleSet();
        int fileId = -1;  // File IDs assigned by the system start at 0
        newFile.add(new Tuple(this.address, -1, null, "/", true));
        this.system.schedule("bfs", new TableName("bfs", "stasis_file"), newFile, null);
        this.system.evaluate();

        // Hack: insert a bfs::fpath tuple for the root path, because that is somehow
        // not automatically inferred
        TupleSet newPath = new TupleSet();
        newPath.add(new Tuple(this.address, "/", fileId));
        this.system.schedule("bfs", new TableName("bfs", "fpath"), newPath, null);
        this.system.evaluate();

        if (Conf.isTapped()) {
            tap.doRewrite("bfs");
            tap.doRewrite("bfs_chunks");
            tap.doRewrite("bfs_heartbeat");
            tap.doRewrite("paxos");
            tap.doRewrite("multipaxos");
        }

        this.system.start();
        System.out.println("Master node @ " + this.port + " ready!");
    }

    private void setupPaxos() throws JolRuntimeException, UpdateException {
        this.system.install("bfs", ClassLoader.getSystemResource("paxos/paxos_global.olg"));

        this.system.install("bfs", ClassLoader.getSystemResource("paxos/paxos_p1.olg"));
        this.system.install("bfs", ClassLoader.getSystemResource("paxos/paxos_p2.olg"));
        this.system.install("bfs", ClassLoader.getSystemResource("paxos/paxos_instance.olg"));

        //this.system.install("bfs", ClassLoader.getSystemResource("alive.olg"));
        this.system.install("bfs", ClassLoader.getSystemResource("paxos/paxos_client_liveness.olg"));

        this.system.evaluate();

        TupleSet id = new TupleSet();
        id.add(new Tuple(address));
        this.system.schedule("paxos", PaxosIdTable.TABLENAME, id, null);

        for (int i = 0; i < Conf.getNumMasters(); i++) {
            String addr = Conf.getMasterAddress(i);
            TupleSet member = new TupleSet();
            member.add(new Tuple(addr, i));
            this.system.schedule("paxos", PaxosMemberTable.TABLENAME, member, null);
        }
        this.system.evaluate();
    }
}
