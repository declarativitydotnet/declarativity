package bfs.test;

import bfs.test.TestCommon;

import org.junit.Test;

public class BFSMM1Test extends TestCommon {
    @Test(timeout=12000)
    public void test1() throws Exception {
        startMany("localhost:5500", "localhost:5502", "localhost:5503");

        // shell = new Shell();
        // createFile(shell,"foo");
        shellCreate("foo");
        assertTrue(shellLs("foo"));
        // assertTrue(findInLs(shell,"foo"));
        shutdown();
    }

    public static void main(String[] args) throws Exception {
        BFSMM1Test t = new BFSMM1Test();
        t.test1();
    }
}
