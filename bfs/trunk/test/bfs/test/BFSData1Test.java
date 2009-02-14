package bfs.test;

import org.junit.Test;

public class BFSData1Test extends DataCommon {
    private static final String TEST_FILE = "/usr/share/dict/words";

	@Test
    public void test1() {
        test(TEST_FILE, 1, 1, 1);
        check_files();
        cleanup_all();
    }

	public static void main(String[] args) throws Exception {
	    BFSData1Test t = new BFSData1Test();
		t.test1();
	}
}
