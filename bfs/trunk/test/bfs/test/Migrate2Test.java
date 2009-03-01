package bfs.test;

import org.junit.Test;

import java.io.File;

public class Migrate2Test extends DataCommon {
    private static final String TEST_FILE = "/usr/share/dict/words";

	@Test(timeout=60000) // XXX takes 36 sec to fail on my machine, but this said 24 sec RCS
    public void test1() throws Exception {
        test(TEST_FILE, 2, 1, 3);


	File foo = new File(".");
	for (String s : foo.list()) {
		if (s.startsWith("td")) {
			File nd = new File(s + File.separator + "chunks");
			for (String sf : nd.list()) {
				System.out.println("\t"+sf);
			}	
		}
	}

        killDataNode(0);
	System.out.println("CONTENTS OF VICTIM DIR:\n");
	File f = new File("td1/chunks");
	for  (String c : f.list()) {
		System.out.println("\t"+c);
	}
        cleanup("td1");

        // all the chunks from this datanode have dropped in

        try {
          Thread.sleep(20001);
        } catch (InterruptedException e) {

        }

        checkFiles();

        //cleanupAll();
    }

	public static void main(String[] args) throws Exception {
		Migrate2Test t = new Migrate2Test();
		t.test1();
	}
}
