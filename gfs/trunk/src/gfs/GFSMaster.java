package gfs;

import jol.core.System;
import jol.core.Runtime;
import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.exception.JolRuntimeException;
import jol.types.exception.UpdateException;
import jol.types.table.TableName;
import jol.types.table.Table.Callback;

public class GFSMaster {
    public static int PORT;
    public static String ADDRESS = "tcp:localhost:5500";
    //public static String ADDRESS;
    public static String[] clique;

    public static void main(String[] args) throws JolRuntimeException, UpdateException {
        if (args.length < 1) {
          clique = null;
          PORT = 5500;
          ADDRESS="tcp:localhost:5500";
        }else {
          clique = new String[args.length];
          PORT = Integer.valueOf(args[0]).intValue();
          ADDRESS = "tcp:localhost:" + args[0];
        }

        for (int i=0; i < args.length; i++) {
          clique[i] = "tcp:localhost:"+args[i];
        }
        GFSMaster m = new GFSMaster();
        m.start();
    }

    private static void usage() {
        java.lang.System.err.println("Usage: gfs.GFSMaster port");
        java.lang.System.exit(1);
    }

    private System system;

    public GFSMaster() {}

    public void start() throws JolRuntimeException, UpdateException {
        this.system = Runtime.create(GFSMaster.PORT);

        this.system.catalog().register(new SelfTable((Runtime) this.system));
        this.system.install("gfs_global", ClassLoader.getSystemResource("gfs/gfs_global.olg"));
        this.system.evaluate();
        this.system.install("gfs", ClassLoader.getSystemResource("gfs/gfs.olg"));
        this.system.evaluate();

        if (clique == null) {
          java.lang.System.out.println("TRIVIAL\n");
          this.system.install("gfs", ClassLoader.getSystemResource("gfs/trivial_glue.olg"));
        } else { 
          paxos_setup();
        }

        Callback cb = new Callback() {
            @Override
            public void deletion(TupleSet tuples) {}

            @Override
            public void insertion(TupleSet tuples) {
                java.lang.System.out.println("Got file @ master: " + tuples.toString());
                   
            }

                
        };

        this.system.catalog().table(new TableName("gfs_global", "file")).register(cb);

        /* Identify which address the local node is at */
        TupleSet self = new TupleSet();
        self.add(new Tuple(GFSMaster.ADDRESS));
        this.system.schedule("gfs", SelfTable.TABLENAME, self, null);
        this.system.evaluate();
        this.system.start();

        java.lang.System.out.println("Server ready!");
    }

    private void paxos_setup() throws JolRuntimeException , UpdateException {
        this.system.install("gfs", ClassLoader.getSystemResource("gfs/olg/paxos/paxos_global.olg"));

        this.system.install("gfs", ClassLoader.getSystemResource("gfs/olg/paxos/paxos_p1.olg"));
        this.system.install("gfs", ClassLoader.getSystemResource("gfs/olg/paxos/paxos_p2.olg"));
        this.system.install("gfs", ClassLoader.getSystemResource("gfs/paxos_gfs_glue.olg"));
        //this.system.install("gfs", ClassLoader.getSystemResource("gfs/trivial_glue.olg"));
        this.system.evaluate();

        TupleSet id = new TupleSet();
        id.add(new Tuple(GFSMaster.ADDRESS));
        this.system.schedule("paxos",PaxosIdTable.TABLENAME,id,null);

        for (String s : this.clique) {
          TupleSet member = new TupleSet();
          member.add(new Tuple(s));
          this.system.schedule("paxos",PaxosMemberTable.TABLENAME,member,null);
        }
        this.system.evaluate();
    }
}
