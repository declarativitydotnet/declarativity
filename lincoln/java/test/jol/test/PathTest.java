package jol.test;

import java.net.URL;
import java.util.concurrent.SynchronousQueue;

import jol.core.Runtime;
import jol.core.System;
import jol.test.PathTable;
import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.exception.JolRuntimeException;
import jol.types.exception.UpdateException;
import jol.types.table.Table;
import jol.types.table.TableName;
import jol.types.table.Table.Callback;

import org.junit.After;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;

public class PathTest {
    private System sys;

    @Before
    public void setup() throws JolRuntimeException, UpdateException {
        this.sys = Runtime.create(5000);
        this.sys.catalog().register(new PathTable((Runtime) this.sys));

        URL u = ClassLoader.getSystemResource("jol/test/path.olg");
        this.sys.install("unit-test", u);
    }

    @After
    public void shutdown() {
        java.lang.System.err.println("shutdown() invoked");
        this.sys.shutdown();
    }

    @Test
    public void simplePathTest() throws UpdateException, InterruptedException {
        /* Arrange to block until the callback tells us we're done */
        final SynchronousQueue<String> queue = new SynchronousQueue<String>();

        Callback cb = new Callback() {
            private int count = 0;

        	@Override
        	public void deletion(TupleSet tuples) {
        	}
        	
        	@Override
        	public void insertion(TupleSet tuples) {
                java.lang.System.err.println("insertion() cb invoked");
        		for (Tuple t : tuples)
        		{
        			String src = (String) t.value(PathTable.Field.SOURCE.ordinal());
        			String dest = (String) t.value(PathTable.Field.DESTINATION.ordinal());
        			Integer hops = (Integer) t.value(PathTable.Field.HOPS.ordinal());

                    java.lang.System.err.println("Path from " + src + " => " + dest);
        			Assert.assertFalse(src.equals(dest));
                    this.count++;
        		}

                try {
                    if (this.count == 6)
                        queue.put("foo"); // done
                } catch (InterruptedException e) {
                    throw new RuntimeException(e);
                }
        	}
        };

        /* Fire the callback for each change to the "paths" table */
        Table path_tbl = this.sys.catalog().table(PathTable.TABLENAME);
        path_tbl.register(cb);

        /* Setup the initial links */
        TupleSet links = new TupleSet();
        links.add(new Tuple("1", "2"));
        links.add(new Tuple("2", "3"));
        links.add(new Tuple("3", "4"));

        TableName link_name = new TableName("path", "link");
        this.sys.schedule("path", link_name, links, null);
        
        /* Wait to be notified by the callback */
        queue.take();
    }
}
