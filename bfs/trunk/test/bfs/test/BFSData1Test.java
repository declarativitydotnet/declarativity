package bfs.test;

import java.io.File;

import org.junit.Test;

public class BFSData1Test extends DataCommon {
    private static final String TEST_FILE = "/usr/share/dict/words";

	@Test(timeout=16000)
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
