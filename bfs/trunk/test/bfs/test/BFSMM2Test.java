package bfs.test;

import bfs.test.TestCommon;

import org.junit.Test;

public class BFSMM2Test extends TestCommon {
    @Test(timeout=16000)
    public void test2() throws Exception {
        startMany("localhost:5500", "localhost:5502", "localhost:5503");

        shellCreate("/foo");
        /* kill one of the masters */
        killMaster(1);

        shellCreate("/bar");
        assertTrue(shellLs("/", "foo", "bar"));

        shutdown();
    }

    public static void main(String[] args) throws Exception {
        BFSMM2Test t = new BFSMM2Test();
        t.test2();
    }
}
