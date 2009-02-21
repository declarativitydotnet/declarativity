package bfs.test;

import org.junit.Test;

public class Data2Test extends DataCommon {
    private static final String TEST_FILE = "/usr/share/dict/words";

	@Test(timeout=24000)
    public void test2() throws Exception {
        test(TEST_FILE, 3, 1, 9);
        checkFiles();
        cleanupAll();
    }

	public static void main(String[] args) throws Exception {
		Data2Test t = new Data2Test();
		t.test2();
	}
}
