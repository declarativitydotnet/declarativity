package gfs.test;

import org.junit.Test;

public class GFSTest extends TestCommon {
    @Test
    public void test1() {
        try {
            startMany("localhost:5505");
            shellCreate("foo");
            shellLs("foo");
            shutdown();
        } catch (Exception e) {
            System.out.println("something went wrong: " + e);
            e.printStackTrace();
            System.exit(1);
        }
    }

    public static void main(String[] args) throws Exception {
        GFSTest t = new GFSTest();
        t.test1();
    }
}
