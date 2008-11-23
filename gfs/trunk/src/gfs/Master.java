package gfs;

import jol.core.System;
import jol.core.Runtime;
import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.exception.JolRuntimeException;
import jol.types.exception.UpdateException;
import jol.types.table.TableName;
import jol.types.table.Table.Callback;

public class Master {
    private int port;
    private String address;
    private String[] clique;

    public static void main(String[] args) throws JolRuntimeException, UpdateException {
        Master m = new Master(args);
        m.start();
    }

    private static void usage() {
        java.lang.System.err.println("Usage: gfs.Master port");
        java.lang.System.exit(1);
    }

    private System system;

    public Master(String... args) {
        if (args.length < 1) {
            clique = null;
            port = 5500;
            address = "tcp:localhost:" + port;
        } else {
            clique = new String[args.length];
            port = Integer.parseInt(args[0]);
            address = "tcp:localhost:" + args[0];
        }

        for (int i = 0; i < args.length; i++) {
            clique[i] = "tcp:localhost:" + args[i];
        }
    }

    public void stop() {
      this.system.shutdown();
    }

    public void start() throws JolRuntimeException, UpdateException {
        /* Identify the address of the local node */
        Conf.setSelfAddress(this.address);

        this.system = Runtime.create(port);

        this.system.install("gfs_global", ClassLoader.getSystemResource("gfs/gfs_global.olg"));
        this.system.evaluate();

        if (clique == null) {
            java.lang.System.out.println("TRIVIAL\n");
            this.system.install("gfs", ClassLoader.getSystemResource("gfs/gfs.olg"));
            this.system.install("gfs", ClassLoader.getSystemResource("gfs/trivial_glue.olg"));
        } else {
            setupPaxos();
        }

        this.system.evaluate();

        Callback cb = new Callback() {
            @Override
            public void deletion(TupleSet tuples) {}

            @Override
            public void insertion(TupleSet tuples) {
                java.lang.System.out.println("Got file @ master: " + tuples.toString());
                   
            }
        };

        this.system.catalog().table(new TableName("gfs_global", "file")).register(cb);

        this.system.start();
        java.lang.System.out.println("Server ready!");
    }

    private void setupPaxos() throws JolRuntimeException, UpdateException {
        this.system.install("gfs", ClassLoader.getSystemResource("paxos/paxos_global.olg"));

        this.system.install("gfs", ClassLoader.getSystemResource("paxos/paxos_p1.olg"));
        //this.system.install("gfs", ClassLoader.getSystemResource("paxos/paxos_pruned2.old"));
        this.system.install("gfs", ClassLoader.getSystemResource("paxos/paxos_p2.olg"));
        this.system.install("gfs", ClassLoader.getSystemResource("paxos/paxos_instance.olg"));

        //this.system.install("gfs", ClassLoader.getSystemResource("alive.olg"));
        //this.system.install("gfs", ClassLoader.getSystemResource("paxos/paxos_client_liveness.olg"));

        this.system.install("gfs", ClassLoader.getSystemResource("gfs/gfs.olg"));
        this.system.install("gfs", ClassLoader.getSystemResource("gfs/paxos_gfs_glue.olg"));
        this.system.evaluate();

        TupleSet id = new TupleSet();
        id.add(new Tuple(address));
        this.system.schedule("paxos", PaxosIdTable.TABLENAME, id, null);

        for (String s : this.clique) {
            TupleSet member = new TupleSet();
            member.add(new Tuple(s));
            this.system.schedule("paxos", PaxosMemberTable.TABLENAME, member, null);
        }
        this.system.evaluate();
    }
}
