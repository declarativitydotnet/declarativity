package bfs.test;

import java.io.File;

import org.junit.Test;

public class BFSStasis2Test extends TestCommon {
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

	    
	    shutdown();
    }

    public static void main(String[] args) throws Exception {
        BFSStasis2Test t = new BFSStasis2Test();
        t.test1();
    }
}
