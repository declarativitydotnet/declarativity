package bfs.test;

import bfs.test.TestCommon;

import org.junit.Test;

public class BFSMM1Test extends TestCommon {
    @Test
    public void test1() {
        try {
            startMany("localhost:5500", "localhost:5502", "localhost:5503");

            // shell = new Shell();
            // createFile(shell,"foo");
            shellCreate("foo");
            assertTrue(shellLs("foo"));
            // assertTrue(findInLs(shell,"foo"));
            shutdown();
        } catch (Exception e) {
            System.out.println("something went wrong!");
            e.printStackTrace();
            System.exit(1);
        }
    }

    public static void main(String[] args) throws Exception {
        BFSMM1Test t = new BFSMM1Test();
        t.test1();
    }
}
