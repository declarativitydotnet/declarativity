package bfs.test;

import org.junit.Test;

public class MigrateTest extends DataCommon {
    private static final String TEST_FILE = "/usr/share/dict/words";

	@Test(timeout=60000) // XXX takes 36 sec to fail on my machine, but this said 24 sec RCS
    public void test1() throws Exception {
        test(TEST_FILE, 3, 1, 6);

        /* victimization of individual chunks.  requires deletion deltas, which
           are not yet implemented
        Victim v = new Victim(this.datanodes.size());
        v.pick_victim();
        System.out.println("victim: dir "+ v.getDir() + ", chunk " + v.getChunk());
        v.do_victim();
        */

        killDataNode(0);
        cleanup("td1");

        // all the chunks from this datanode have dropped in

        try {
          Thread.sleep(20001);
        } catch (InterruptedException e) {

        }

        checkFiles();
        cleanupAll();
    }

	public static void main(String[] args) throws Exception {
		MigrateTest t = new MigrateTest();
		t.test1();
	}
}
