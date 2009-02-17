package bfs.test;

import org.junit.Test;

public class BFSTest extends TestCommon {
    @Test(timeout=10000)
    public void test1() throws Exception {
        startMany("localhost:5505");
        shellCreate("/foo");
        shellLs("/foo");
        shutdown();
    }

    public static void main(String[] args) throws Exception {
        BFSTest t = new BFSTest();
        t.test1();
    }
}
