package jol.test;

import java.net.URL;

import jol.core.Runtime;
import jol.core.System;
import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.exception.JolRuntimeException;
import jol.types.exception.UpdateException;
import jol.types.table.Key;
import jol.types.table.ObjectTable;
import jol.types.table.Table;
import jol.types.table.TableName;

import org.junit.After;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;

public class PathTest {
    private static class PathTable extends ObjectTable {
        public static final TableName TABLENAME = new TableName("path", "path");
        
        public static final Key PRIMARY_KEY = new Key(0, 1, 2);
        
        public enum Field {
            SOURCE,
            DESTINATION,
            HOPS
        };
        
        public static final Class[] SCHEMA = {
            String.class,   // Source
            String.class,   // Destination
            Integer.class   // # of hops from source => dest on this path
        };
        
        public PathTable(Runtime context) {
            super(context, TABLENAME, PRIMARY_KEY, SCHEMA);
        }
    }

    private System sys;

    @Before
    public void setup() throws JolRuntimeException, UpdateException {
        this.sys = Runtime.create(5000);
        this.sys.catalog().register(new PathTable((Runtime) this.sys));

        URL u = ClassLoader.getSystemResource("jol/test/path.olg");
        this.sys.install("unit-test", u);
        this.sys.evaluate();    // XXX: temporary workaround for runtime bug
    }

    @After
    public void shutdown() {
        this.sys.shutdown();
    }

    @Test
    public void simplePathTest() throws UpdateException, JolRuntimeException {
        /* Setup the initial links */
        TupleSet links = new TupleSet();
        links.add(new Tuple("1", "2"));
        links.add(new Tuple("2", "3"));
        links.add(new Tuple("3", "4"));

        TableName link_name = new TableName("path", "link");
        this.sys.schedule("path", link_name, links, null);
        this.sys.evaluate();

        /* We expect to find 6 paths */
        int nPaths = countPaths();
        Assert.assertEquals(6, nPaths);

        /* Add a link from 2 => 4 */
        links.clear();
        links.add(new Tuple("2", "4"));
        this.sys.schedule("path", link_name, links, null);
        this.sys.evaluate();

        /* We expect to find 8 paths */
        nPaths = countPaths();
        Assert.assertEquals(8, nPaths);

        /* Delete the link from 1 => 2 */
        links.clear();
        links.add(new Tuple("1", "2"));
        this.sys.schedule("path", link_name, null, links);
        /* XXX: hack -- we need to call evaluate() until all pending deletes are done */
        this.sys.evaluate();
        this.sys.evaluate();
        this.sys.evaluate();
        this.sys.evaluate();

        /* We expect to find 4 paths */
        nPaths = countPaths();
        Assert.assertEquals(4, nPaths);
    }

    private int countPaths() {
        Table path_tbl = this.sys.catalog().table(PathTable.TABLENAME);
        int nPaths = 0;

        for (Tuple t : path_tbl.tuples()) {
            String src = (String) t.value(PathTable.Field.SOURCE.ordinal());
            String dest = (String) t.value(PathTable.Field.DESTINATION.ordinal());

            Assert.assertFalse(src.equals(dest));
            nPaths++;
        }

        return nPaths;
    }

    public static void main(String[] args) throws Exception {
        PathTest t = new PathTest();
        t.setup();
        t.simplePathTest();
        t.shutdown();
    }
}
