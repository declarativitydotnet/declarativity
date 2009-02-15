package bfs.test;

import bfs.test.TestCommon;
import java.io.File;

import org.junit.Test;

public class BFSStasis1Test extends TestCommon {
    @Test
    public void test1() {
        try {
		    new File("storefile.txt").delete();
		    new File("logfile.txt").delete();

            startMany("localhost:5509");

            shellCreate("foo");
            assertTrue(shellLs("foo"));
		    this.killMaster(0);

	        System.out.println("--------------------------------------------------------------------------------------------------------\n");

            startMany("localhost:5509");

		    Thread.sleep(1000);
	
            assertTrue(shellLs("foo"));
		    shutdown();
        } catch (Exception e) {
            System.out.println("something went wrong!");
            e.printStackTrace();
            System.exit(1);
        }
    }

    public static void main(String[] args) throws Exception {
        BFSStasis1Test t = new BFSStasis1Test();
        t.test1();
    }
}
