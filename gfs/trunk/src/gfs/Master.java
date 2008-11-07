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
    public static final int PORT = 5500;
    public static final String ADDRESS = "tcp:localhost:" + PORT;

    public static void main(String[] args) throws JolRuntimeException, UpdateException {
        if (args.length != 0)
            usage();

        Master m = new Master();
        m.start();
    }

    private static void usage() {
        java.lang.System.err.println("Usage: gfs.Master");
        java.lang.System.exit(1);
    }

    private System system;

    public Master() {}

    public void start() throws JolRuntimeException, UpdateException {
        this.system = Runtime.create(Master.PORT);

        this.system.catalog().register(new SelfTable((Runtime) this.system));
        this.system.install("gfs", ClassLoader.getSystemResource("gfs/gfs.olg"));
        this.system.evaluate();

        Callback cb = new Callback() {
            @Override
            public void deletion(TupleSet tuples) {}

            @Override
            public void insertion(TupleSet tuples) {
                java.lang.System.out.println("Got request @ master: " + tuples.toString());
            }
        };

        this.system.catalog().table(new TableName("gfs", "request")).register(cb);

        /* Identify which address the local node is at */
        TupleSet self = new TupleSet();
        self.add(new Tuple(Master.ADDRESS));
        this.system.schedule("gfs", SelfTable.TABLENAME, self, null);
        this.system.evaluate();
        this.system.start();

        java.lang.System.out.println("Server ready!");
    }
}
