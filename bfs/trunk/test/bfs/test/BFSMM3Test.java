package bfs.test;

import bfs.test.TestCommon;

import org.junit.Test;

public class BFSMM3Test extends TestCommon {
    @Test(timeout=120000)
    public void test3() {
        try {
            startMany("localhost:5500", "localhost:5502", "localhost:5503");

            shellCreate("foo");
            /* this time, kill the primary */
            this.killMaster(0);

            /* these ops should timeout to the secondary but eventually work */
            shellCreate("bar");
            shellCreate("bas");

            assertTrue(shellLs("foo", "bar", "bas"));

            System.out.println("OK, good then\n");
            shutdown();
        } catch (Exception e) {
            System.out.println("something went wrong: " + e);
            System.exit(1);
        }
    }

    public static void main(String[] args) throws Exception {
        BFSMM3Test t = new BFSMM3Test();
        t.test3();
    }
}
