package bfs.test;

import bfs.test.TestCommon;

import org.junit.Test;

public class BFSMM4Test extends TestCommon {
    @Test
    public void test2() {
        try {
            startMany("localhost:5500", "localhost:5502", "localhost:5503");

            // shell = new Shell();
            // createFile(shell,"foo");

            /* kill one of the masters */
            killMaster(1);

            // shell = new Shell();
            // createFile(shell,"foo");
            // createFile(shell,"bar");

            shellCreate("foo");
            shellCreate("bar");

            assertTrue(shellLs("foo", "bar"));

            // shellRm("foo");

            // /assertTrue(!shellLs("foo"));

            shutdown();
        } catch (Exception e) {
            System.out.println("something went wrong: " + e);
            System.exit(1);
        }
    }

    public static void main(String[] args) throws Exception {
        BFSMM4Test t = new BFSMM4Test();
        t.test2();
    }
}
