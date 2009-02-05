package gfs.test;

import gfs.test.TestCommon;

import org.junit.Test;

public class GFSMM2Test extends TestCommon {
    @Test
    public void test2() {
        try {
            startMany("localhost:5500", "localhost:5502", "localhost:5503");

            shellCreate("foo");
            /* kill one of the masters */
            // this.masters.get(1).stop();
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
        GFSMM2Test t = new GFSMM2Test();
        t.test2();
    }
}
