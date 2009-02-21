package bfs.test;

import bfs.test.TestCommon;

import org.junit.Test;

public class MM4Test extends TestCommon {
    @Test(timeout=20000)
    public void test4() throws Exception {
        startMany("localhost:5500", "localhost:5502", "localhost:5503");

        /* kill one of the masters */
        killMaster(1);

        shellCreate("/foo");
        shellCreate("/bar");
        assertTrue(shellLs("/", "foo", "bar"));

        shellRm("/foo");
        assertTrue(!shellLs("/", "/foo"));

        shutdown();
    }

    public static void main(String[] args) throws Exception {
        MM4Test t = new MM4Test();
        t.test4();
    }
}
