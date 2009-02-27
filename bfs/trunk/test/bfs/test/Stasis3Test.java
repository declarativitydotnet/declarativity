package bfs.test;

import java.io.File;

import org.junit.Test;

public class Stasis3Test extends TestCommon {
	/**
	 * Test multi-master stasis setups w/ shutdown, but no catch-up
	 * @throws Exception
	 */
	@Test
    public void test1() throws Exception {
	    new File("storefile.txt").delete();
	    new File("logfile.txt").delete();

        startMany("localhost:5509", "localhost:5510", "localhost:5511");

        shellCreate("/foo");
        assertTrue(shellLs("/", "foo"));

        // XXX Do not call shutdown(); not handling data nodes yet.
	    this.killMaster(0);
	    this.killMaster(1);
	    this.killMaster(2);

        System.out.println("--------------------------------------------------------------------------------------------------------\n");

        startMany("localhost:5509", "localhost:5510", "localhost:5511");

	    Thread.sleep(1000);

	    assertTrue(shellLs("/", "foo"));

	    this.killMaster(0);

	    shellCreate("/bar");

	    assertTrue(shellLs("/", "foo"));
	    assertTrue(shellLs("/", "bar"));

	    this.killMaster(1);

	    assertTrue(shellLs("/", "foo"));
	    assertTrue(shellLs("/", "bar"));

	    this.killMaster(2);

	    startMany("localhost:5509", "localhost:5510", "localhost:5511");
	    shellCreate("/baz"); // hopefully this will kick over to the master, and block until it's done catching up.

	    assertTrue(shellLs("/", "foo"));
	    assertTrue(shellLs("/", "bar"));
	    assertTrue(shellLs("/", "baz"));

	    this.killMaster(2);
	    this.killMaster(1);

	    assertTrue(shellLs("/", "foo"));
	    assertTrue(shellLs("/", "bar"));
	    assertTrue(shellLs("/", "baz"));

	    shutdown();
    }

    public static void main(String[] args) throws Exception {
        Stasis3Test t = new Stasis3Test();
        t.test1();
    }
}
