package bfs.test;

import org.junit.Test;

public class BFSData4Test extends DataCommon {
    private static final String TEST_FILE = "/usr/share/dict/words";

	@Test
    public void test1() {
        test(TEST_FILE, 3, 1, 9);
        //check_files();

        /* victimization of individual chunks.  requires deletion deltas, which
           are not yet implemented
        Victim v = new Victim(this.datanodes.size());
        v.pick_victim();
        System.out.println("victim: dir "+ v.getDir() + ", chunk " + v.getChunk());        
        v.do_victim();
        */

        killDataNode(0);
        
        //check_files();
        //cleanup_all();
    }

	public static void main(String[] args) throws Exception {
		BFSData4Test t = new BFSData4Test();
		t.test1();
	}
}
