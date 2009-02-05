package bfs.test;

import org.junit.Test;

public class BFSTest extends TestCommon {
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
        BFSTest t = new BFSTest();
        t.test1();
    }
}
