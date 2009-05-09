package bfs.test;

import java.net.URL;
import java.util.concurrent.SynchronousQueue;

import jol.core.JolSystem;
import jol.core.Runtime;
import jol.types.basic.BasicTupleSet;
import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.exception.JolRuntimeException;
import jol.types.table.Key;
import jol.types.table.ObjectTable;
import jol.types.table.Table;
import jol.types.table.TableName;
import jol.types.table.Table.Callback;
import junit.framework.Assert;

public class PaxosCommon {

    protected static class IdTable extends ObjectTable {
        public static final TableName TABLENAME = new TableName("paxos_global", "id");

        public static final Key PRIMARY_KEY = new Key(0);

        public enum Field {
            ME
        };

        public static final Class<?>[] SCHEMA = {
        	String.class // Address
        };

        IdTable(Runtime context) {
            super(context, TABLENAME, PRIMARY_KEY, SCHEMA);
        }
    }

    protected static class NodeTable extends ObjectTable {
        public static final TableName TABLENAME = new TableName("paxos_global", "members");

        public static final Key PRIMARY_KEY = new Key(0);

        public enum Field {
            HIM
        };

        public static final Class<?>[] SCHEMA = {
            String.class // Address
        };

        NodeTable(Runtime context) {
            super(context, TABLENAME, PRIMARY_KEY, SCHEMA);
        }
    }

    protected static class InMessageTable extends ObjectTable {
        public static final TableName TABLENAME = new TableName("paxos_global",
                "decreeRequest");

        public static final Key PRIMARY_KEY = new Key(0, 1);

        public enum Field {
            ME, MESSAGE, FROM
        };

        public static final Class<?>[] SCHEMA = { String.class, String.class, String.class };

        InMessageTable(Runtime context) {
            super(context, TABLENAME, PRIMARY_KEY, SCHEMA);
        }
    }

    private static final int MASTER_PORT = 5000;

    protected JolSystem[] systems;
    protected int[] ports;
    private JolSystem pinger;

    // @Before

    public void setup(int... members) throws Exception {
        this.systems = new JolSystem[members.length];
        this.ports = new int[members.length];
        System.out.println("ml is " + members.length);
        for (int i = 0; i < members.length; i++) {
            this.systems[i] = Runtime.create(Runtime.DEBUG_ALL, System.err, members[i]);
            this.ports[i] = members[i];
        }
        this.pinger = this.systems[0];

        for (JolSystem s : this.systems) {
            s.catalog().register(new NodeTable((Runtime) s));
            s.catalog().register(new IdTable((Runtime) s));
            s.catalog().register(new InMessageTable((Runtime) s));
            s.evaluate();
            myInstall(s, "paxos/paxos_global.olg");
            myInstall(s, "paxos/paxos_p1.olg");
            myInstall(s, "paxos/paxos_p2.olg");
            myInstall(s, "paxos/paxos_instance.olg");
            // //myInstall(s, "bfs/test/olg/paxos/paxos_client_liveness.olg");
        }

        TupleSet nodes = new BasicTupleSet();
        for (int m : members) {
            nodes.add(new Tuple(this.makeAddr(m)));
        }
        for (int i = 0; i < members.length; i++) {
            TupleSet id = new BasicTupleSet();
            id.add(new Tuple(this.makeAddr(members[i])));
            this.systems[i].schedule("paxos_global", IdTable.TABLENAME, id, null);
            this.systems[i].schedule("paxos_global", NodeTable.TABLENAME, nodes, null);
        }

        for (JolSystem s : this.systems)
            s.evaluate();
    }

    void myInstall(JolSystem s, String file) {
        URL u = ClassLoader.getSystemResource(file);
        try {
            s.install("multipaxos", u);
            s.evaluate();
        } catch (JolRuntimeException e) {
            throw new RuntimeException("install failed for " + file, e);
        }
    }

    // @After
    public void shutdown() {
        for (JolSystem s : this.systems)
            s.shutdown();
    }

    public void simpleMultiPaxosTest() throws Exception {
        /* Arrange to block until the callback tells us we're done */
        final SynchronousQueue<String> queue = new SynchronousQueue<String>();

        Callback cb = new Callback() {
            @Override
            public void deletion(TupleSet tuples) {
            }

            @Override
            public void insertion(TupleSet tuples) {
                System.out.println("Got insert!");
                String msg = standardTest(tuples, 0, "foo", "bar", "bas");
                try {
                    queue.put(msg);
                } catch (InterruptedException e) {
                    System.out.println("some shit went down\n");
                    throw new RuntimeException(e);
                }
            }
        };

        TableName ping_tbl_name = new TableName("multipaxos", "success");
        Table tab = this.pinger.catalog().table(ping_tbl_name);
        tab.register(cb);

        sked(this.pinger, MASTER_PORT, "foo");
        for (JolSystem s : this.systems)
            s.start();

        System.out.println("test started, waiting for callback...");
        String pop = (String) queue.take();
        Assert.assertTrue(pop.compareTo("success") == 0);

        sked(this.pinger, MASTER_PORT, "bar");
        pop = (String) queue.take();
        Assert.assertTrue(pop.compareTo("success") == 0);

        sked(this.pinger, MASTER_PORT, "bas");
        pop = (String) queue.take();
        Assert.assertTrue(pop.compareTo("success") == 0);

        tab.unregister(cb);
    }

    public void complexMultiPaxosTest() throws Exception {
        /* Arrange to block until the callback tells us we're done */
        final SynchronousQueue<String> queue = new SynchronousQueue<String>();

        Callback cb = new Callback() {
            @Override
            public void deletion(TupleSet tuples) {
            }

            @Override
            public void insertion(TupleSet tuples) {
                System.out.println("Got insert!");
                String msg = standardTest(tuples, 0, "foo", "bar");
                try {
                    queue.put(msg);
                } catch (InterruptedException e) {
                    System.out.println("some shit went down\n");
                    throw new RuntimeException(e);
                }
            }
        };

        TableName ping_tbl_name = new TableName("multipaxos", "success");
        Table tab = this.pinger.catalog().table(ping_tbl_name);
        tab.register(cb);

        sked(this.systems[0], MASTER_PORT, "foo");
        for (JolSystem s : this.systems)
            s.start();

        System.out.println("test started, waiting for callback...");
        String pop = (String) queue.take();
        Assert.assertTrue(pop.compareTo("success") == 0);

        sked(this.systems[1], 5502, "bar");
        pop = (String) queue.take();
        Assert.assertTrue(pop.compareTo("success") == 0);

        tab.unregister(cb);
    }

    private String standardTest(TupleSet tuples, int round, String... slist)
            throws RuntimeException {
        String msg = "success";
        for (Tuple t : tuples) {
            System.out.println("tuple: " + t.toString());

            Integer instance = (Integer) t.value(1);
            Integer thisround = (Integer) t.value(2);
            String decree = (String) t.value(3);
            // last failure wins.
            if (round != thisround)
                msg = "failure: round=" + thisround;
            if (instance >= slist.length) {
                msg = "failure: oob instance " + instance;
            } else if (decree.compareTo(slist[instance]) != 0) {
                msg = "failure: instance[" + instance + "], decree=" + decree;
            }
        }
        return msg;
    }

    private void sked(JolSystem sys, int port, String message) throws Exception {
        TupleSet inmessage = new BasicTupleSet();
        inmessage.add(new Tuple(this.makeAddr(port), message, this.makeAddr(port)));
        sys.schedule("paxos_global", InMessageTable.TABLENAME, inmessage, null);
        sys.evaluate();
    }

    protected String makeAddr(int port) {
        return "tcp:localhost:" + port;
    }
}
