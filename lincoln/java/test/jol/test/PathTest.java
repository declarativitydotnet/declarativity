package jol.test;

import java.net.URL;

import jol.core.Runtime;
import jol.core.System;
import jol.test.PathTable;
import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.exception.P2RuntimeException;
import jol.types.exception.UpdateException;
import jol.types.table.TableName;
import jol.types.table.Table.Callback;

import org.junit.After;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;

public class PathTest {
    private System sys;

    @Before
    public void setup() throws P2RuntimeException, UpdateException {
        this.sys = Runtime.create(5000);
        URL u = ClassLoader.getSystemResource("jol/test/path.olg");
        this.sys.install("gfs", u);
    }

    @After
    public void shutdown() {
        this.sys.shutdown();
    }

    @Test
    public void simplePathTest() throws UpdateException {
        /* Setup the initial links */
        TableName link_tbl = new TableName("path", "link");
        TupleSet links = new TupleSet();
        links.add(new Tuple("1", "2"));
        links.add(new Tuple("2", "3"));
        links.add(new Tuple("3", "4"));
        
        Callback cb = new Callback() {
        	@Override
        	public void deletion(TupleSet tuples) {
        	}
        	
        	@Override
        	public void insertion(TupleSet tuples) {
        		for (Tuple t : tuples)
        		{
        			String src = (String) t.value(PathTable.Field.SOURCE.ordinal());
        			String dest = (String) t.value(PathTable.Field.DESTINATION.ordinal());
        			Integer hops = (Integer) t.value(PathTable.Field.HOPS.ordinal());
        			
        			Assert.assertFalse(src.equals(dest));
        			Assert.assertEquals(hops.intValue(), 1);
        		}
        	}
        };

        this.sys.schedule("path", link_tbl, links, null);
    }
}
