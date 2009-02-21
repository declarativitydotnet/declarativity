package bfs.test;

import bfs.test.TestCommon;

import org.junit.Test;

public class MM6Test extends TestCommon {
    @Test(timeout=120000)
    public void test() throws Exception {
        startMany("localhost:5500", "localhost:5502", "localhost:5503");

        shellCreate("/foo");
        shellRm("/foo");
        safeAssert("foo found??", !shellLs("/", "foo"));

        /* kill the primary */
        killMaster(0);

        /* we need to ensure that the deletion performed on the primary master
           is reflected on the secondary after a master failure
        */
        safeAssert("foo found2??", !shellLs("/", "foo"));

        shutdown();
    }

    public static void main(String[] args) throws Exception {
        MM6Test t = new MM6Test();
        t.test();
    }
}
