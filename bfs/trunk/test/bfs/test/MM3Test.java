package bfs.test;

import bfs.test.TestCommon;

import org.junit.Test;

public class MM3Test extends TestCommon {
    @Test(timeout=120000)
    public void test3() throws Exception {
        startMany("localhost:5500", "localhost:5502", "localhost:5503");

        shellCreate("/foo");
        /* this time, kill the primary */
        killMaster(0);

        /* these ops should timeout to the secondary but eventually work */
        shellCreate("/bar");
        shellCreate("/bas");

        assertTrue(shellLs("/", "foo", "bar", "bas"));

        System.out.println("OK, good then\n");
        shutdown();
    }

    public static void main(String[] args) throws Exception {
        MM3Test t = new MM3Test();
        t.test3();
    }
}
