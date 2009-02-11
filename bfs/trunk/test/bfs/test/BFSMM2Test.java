package bfs.test;

import bfs.test.TestCommon;

import org.junit.Test;

public class BFSMM2Test extends TestCommon {
    @Test
    public void test2() {
        try {
            startMany("localhost:5500", "localhost:5502", "localhost:5503");

            shellCreate("foo");
            /* kill one of the masters */
            this.killMaster(1);

            shellCreate("bar");
            assertTrue(shellLs("foo", "bar"));

            shutdown();
        } catch (Exception e) {
            System.out.println("something went wrong: " + e);
            System.exit(1);
        }
    }

    public static void main(String[] args) throws Exception {
        BFSMM2Test t = new BFSMM2Test();
        t.test2();
    }
}
