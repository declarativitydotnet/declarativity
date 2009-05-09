package bfs.test;

import org.junit.Test;

public class Data1Test extends DataCommon {
    private static final String TEST_FILE = "/usr/share/dict/words";

	@Test(timeout=16000)
    public void test1() throws Exception {
	    test(TEST_FILE, 1, 1, 1);
        checkFiles();
        cleanupAll();
    }

	public static void main(String[] args) throws Exception {
	    Data1Test t = new Data1Test();
		t.test1();
	}
}
