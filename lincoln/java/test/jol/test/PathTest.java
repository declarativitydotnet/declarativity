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
    public void setup() throws P2RuntimeException {
        this.sys = Runtime.create(5000);
    }

    @After
    public void shutdown() {
        this.sys.shutdown();
    }

    @Test
    public void simplePathTest() throws MalformedURLException, UpdateException {
        this.sys.install("gfs",
                         new URL("file:///home/neilconway/lincoln/java/test/jol/path.olg"));
    }
}
