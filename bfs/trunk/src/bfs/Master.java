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
        if (args.length != 1)
            usage();

        int masterIdx = Integer.parseInt(args[0]);
        if (masterIdx < 0 || masterIdx >= Conf.getNumMasters()) {
            System.err.println("Illegal master index: " + masterIdx);
            usage();
        }

        Master m = new Master(masterIdx);
        m.start();
    }

    private static void usage() {
        System.err.println("Usage: bfs.Master index");
        System.err.println("    where 0 <= \"index\" <= " + (Conf.getNumMasters() - 1));
        System.exit(1);
    }

    public Master(int masterIdx) {
        this.address = Conf.getMasterAddress(masterIdx);
        this.port = Conf.getMasterPort(masterIdx);
    }

    public void stop() {
        this.system.shutdown();
    }

    public void start() throws JolRuntimeException, UpdateException {
        /* Identify the address of the local node */
        Conf.setSelfAddress(this.address);

        this.system = Runtime.create(Runtime.DEBUG_WATCH, System.err, this.port);

        OlgAssertion olgAssert = new OlgAssertion(this.system, true);

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
        String fName = "/";
        int fileId = fName.hashCode();
        newFile.add(new Tuple(Conf.getSelfAddress(), fileId, null, fName, true));
        this.system.schedule("bfs", new TableName("bfs", "file"), newFile, null);
        this.system.evaluate();

        // Hack: insert a bfs::fpath tuple for the root path, because that is somehow
        // not automatically inferred
        TupleSet newPath = new TupleSet();
        newPath.add(new Tuple(Conf.getSelfAddress(), "/", fileId));
        this.system.schedule("bfs", new TableName("bfs", "fpath"), newPath, null);
        this.system.evaluate();

        this.system.start();
        System.out.println("Master node @ " + this.port + " ready!");
    }

    private void setupPaxos() throws JolRuntimeException, UpdateException {
        this.system.install("bfs", ClassLoader.getSystemResource("paxos/paxos_global.olg"));

        this.system.install("bfs", ClassLoader.getSystemResource("paxos/paxos_p1.olg"));
        this.system.install("bfs", ClassLoader.getSystemResource("paxos/paxos_p2.olg"));
        this.system.install("bfs", ClassLoader.getSystemResource("paxos/paxos_instance.olg"));

        //this.system.install("bfs", ClassLoader.getSystemResource("alive.olg"));
        //this.system.install("bfs", ClassLoader.getSystemResource("paxos/paxos_client_liveness.olg"));

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
