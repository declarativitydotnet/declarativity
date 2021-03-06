package bfs.test;

import bfs.test.TestCommon;

import org.junit.Test;

public class MM1Test extends TestCommon {
    @Test(timeout=28000) // XXX was under 12 before
    public void test1() throws Exception {
        startMany("localhost:5500", "localhost:5502", "localhost:5503");
        shellCreate("/foo");
        assertTrue(shellLs("/", "foo"));
        shutdown();
    }

    public static void main(String[] args) throws Exception {
        MM1Test t = new MM1Test();
        t.test1();
    }
}
