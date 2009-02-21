package bfs.test;

import java.io.File;

import org.junit.Test;

public class Stasis1Test extends TestCommon {
    @Test
    public void test1() throws Exception {
	    new File("storefile.txt").delete();
	    new File("logfile.txt").delete();

        startMany("localhost:5509");

        shellCreate("/foo");
        assertTrue(shellLs("/", "foo"));
	    this.killMaster(0);

        System.out.println("--------------------------------------------------------------------------------------------------------\n");

        startMany("localhost:5509");

	    Thread.sleep(1000);

        assertTrue(shellLs("/", "foo"));
	    shutdown();
    }

    public static void main(String[] args) throws Exception {
        Stasis1Test t = new Stasis1Test();
        t.test1();
    }
}
