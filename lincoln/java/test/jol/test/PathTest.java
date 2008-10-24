package jol.test;

import java.net.MalformedURLException;
import java.net.URL;

import jol.core.Runtime;
import jol.core.System;
import jol.types.exception.P2RuntimeException;
import jol.types.exception.UpdateException;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

public class PathTest {
    private System sys;

    @Before
    public void setup() throws P2RuntimeException, MalformedURLException {
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
        TupleSet links = new TupleSet(link_tbl);
        links.add(new Tuple("1", "2"));
        links.add(new Tuple("2", "3"));
        links.add(new Tuple("3", "4"));

        system.schedule("path", link_tbl, links, null);
    }
}
