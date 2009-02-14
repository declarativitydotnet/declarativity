package bfs.test;

import org.junit.Test;

public class BFSData3Test extends DataCommon {
    private static final String TEST_FILE = "/usr/share/dict/words";

	@Test(timeout=28000)
    public void test1() {
        test(TEST_FILE, 4, 1, 13);
        check_files();
        cleanup_all();
    }

	public static void main(String[] args) throws Exception {
		BFSData3Test t = new BFSData3Test();
		t.test1();
	}
}
