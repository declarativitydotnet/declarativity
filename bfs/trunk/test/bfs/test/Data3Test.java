package bfs.test;

import org.junit.Test;

public class Data3Test extends DataCommon {
    private static final String TEST_FILE = "/usr/share/dict/words";

	@Test(timeout=28000)
    public void test3() throws Exception {
        test(TEST_FILE, 4, 1, 13);
        checkFiles();
        cleanupAll();
    }

	public static void main(String[] args) throws Exception {
		Data3Test t = new Data3Test();
		t.test3();
	}
}
