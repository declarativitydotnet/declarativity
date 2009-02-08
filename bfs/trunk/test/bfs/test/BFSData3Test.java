package bfs.test;

import org.junit.Test;

public class BFSData3Test extends DataCommon {
    private static final String TEST_FILE = "/usr/share/dict/words";

	@Test
    public void test1() {
        test(TEST_FILE, 4, 1, 13);
    }

	public static void main(String[] args) throws Exception {
		BFSData3Test t = new BFSData3Test();
		t.test1();
	}
}
