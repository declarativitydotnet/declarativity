package jol.test;

import java.net.URL;
import java.util.concurrent.SynchronousQueue;

import jol.core.Runtime;
import jol.core.System;
import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.basic.TypeList;
import jol.types.exception.JolRuntimeException;
import jol.types.exception.UpdateException;
import jol.types.table.Key;
import jol.types.table.ObjectTable;
import jol.types.table.TableName;
import jol.types.table.Table.Callback;
import junit.framework.Assert;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

public class PingPongTest {
	private static class NodeTable extends ObjectTable {
		public static final TableName TABLENAME = new TableName("pingpong", "nodes");
		
		public static final Key PRIMARY_KEY = new Key(0);
		
		public enum Field {
			ADDRESS
		};
		
		public static final Class[] SCHEMA = {
			String.class	// Address
		};
		
		NodeTable(Runtime context) {
			super(context, TABLENAME, PRIMARY_KEY, new TypeList(SCHEMA));
		}
	}
	
	private static class SelfTable extends ObjectTable {
		public static final TableName TABLENAME = new TableName("pingpong", "self");
		
		public static final Key PRIMARY_KEY = new Key(0);
		
		public enum Field {
			ADDRESS
		}
		
		public static final Class[] SCHEMA = {
			String.class	// Address
		};
		
		SelfTable(Runtime context) {
			super(context, TABLENAME, PRIMARY_KEY, new TypeList(SCHEMA));
		}
	}
	
	private static class InMessageTable extends ObjectTable {
		public static final TableName TABLENAME = new TableName("pingpong", "inmessage");
		
		public static final Key PRIMARY_KEY = new Key(0);
		
		public enum Field {
			ID,
			COUNT
		};
		
		public static final Class[] SCHEMA = {
			Integer.class,
			Integer.class
		};
		
		InMessageTable(Runtime context) {
			super(context, TABLENAME, PRIMARY_KEY, new TypeList(SCHEMA));
		}
	}

	private static final int PINGER_PORT = 5000;
	private static final int PONGER_PORT = 5001;

	private System[] systems;
	private System pinger;
	private System ponger;
	private int nextId = 0;

	@Before
    public void setup() throws JolRuntimeException, UpdateException, InterruptedException {
		this.systems = new System[2];
		this.systems[0] = this.pinger = Runtime.create(PINGER_PORT);
		this.systems[1] = this.ponger = Runtime.create(PONGER_PORT);
		
		for (System s : this.systems)
		{
			s.catalog().register(new NodeTable((Runtime) s));
			s.catalog().register(new SelfTable((Runtime) s));
			s.catalog().register(new InMessageTable((Runtime) s));
            s.evaluate();
		}

		URL u = ClassLoader.getSystemResource("jol/test/pingpong.olg");
		for (System s : this.systems) {
			s.install("pingpong", u);
            s.evaluate();
        }
		
		/* Tell each runtime about the set of nodes in the cluster */
		TupleSet nodes = new TupleSet();
		nodes.add(new Tuple(this.makeAddr(PINGER_PORT)));
		nodes.add(new Tuple(this.makeAddr(PONGER_PORT)));
		for (System s : this.systems)
			s.schedule("pingpong", NodeTable.TABLENAME, nodes, null);
		
		/* Tell each runtime its own address */
		TupleSet self = new TupleSet();
		self.add(new Tuple(this.makeAddr(PINGER_PORT)));
		this.pinger.schedule("pingpong", SelfTable.TABLENAME, self, null);
		
		self.clear();
		self.add(new Tuple(this.makeAddr(PONGER_PORT)));
		this.ponger.schedule("pingpong", SelfTable.TABLENAME, self, null);

        for (System s : this.systems)
            s.evaluate();
	}
	
	@After
	public void shutdown() {
        for (System s : this.systems)
            s.shutdown();
	}
	
	@Test
    public void simplePingPongTest() throws UpdateException, InterruptedException, JolRuntimeException {
        /* Arrange to block until the callback tells us we're done */
        final SynchronousQueue<String> queue = new SynchronousQueue<String>();

		Callback cb = new Callback() {
			public void deletion(TupleSet tuples) {}

			public void insertion(TupleSet tuples) {
                java.lang.System.out.println("Got insert!");
				Assert.assertEquals(1, tuples.size());
				
				Tuple t = tuples.iterator().next();
				Integer id = (Integer) t.value(3);
				Assert.assertEquals(1, id.intValue());
				
				try {
					queue.put("done!");
				} catch (InterruptedException e) {
					throw new RuntimeException(e);
				}
			}
		};

		TableName ping_tbl_name = new TableName("pingpong", "ping");
		this.ponger.catalog().table(ping_tbl_name).register(cb);

		/* Send a new message from pinger => ponger */
		TupleSet inmessage = new TupleSet();
		inmessage.add(new Tuple(this.getNewId(), 1));
		this.pinger.schedule("pingpong", InMessageTable.TABLENAME, inmessage, null);

        java.lang.System.out.println("test started, waiting for callback...");
		queue.take();
        java.lang.System.out.println("done!");
	}
	
	private int getNewId() {
		return this.nextId++;
	}
	
	private String makeAddr(int port) {
		return "tcp:localhost:" + port;
	}

    public static void main(String[] args) throws Exception {
        PingPongTest t = new PingPongTest();
        t.setup();
        t.simplePingPongTest();
        t.shutdown();
    }
}
