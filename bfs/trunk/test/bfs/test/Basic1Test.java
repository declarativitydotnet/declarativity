package bfs.test;

import org.junit.Test;

public class Basic1Test extends TestCommon {
    @Test(timeout=10000)
    public void test1() throws Exception {
        startMany("localhost:5505");
        shellCreate("/foo");
        shellLs("/", "foo");
        shutdown();
    }

    public static void main(String[] args) throws Exception {
        Basic1Test t = new Basic1Test();
        t.test1();
    }
}
