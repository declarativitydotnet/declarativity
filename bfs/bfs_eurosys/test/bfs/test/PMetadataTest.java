package bfs.test;

import java.util.Set;

import org.junit.Test;

import bfs.BFSClient;
import bfs.BFSFileInfo;

public class PMetadataTest extends TestCommon {
	private final static String TEST_FILENAME = "/foo/e2e_f";

	/* XXX: refactor this to avoid duplicated code */
	@Test(timeout=240000) // XXX was 12000 before perf regresssion.  This used to take ~ 6 sec
	public void test1() throws Exception {
		startManyPartitioned(new String[][] { {"localhost:5505"}, {"localhost:5506"}, {"localhost:5507"}});
		startManyDataNodes("td1", "td2");

		BFSClient bfs = new BFSClient(10001);
		safeAssert(bfs.createDir("/foo"));
		safeAssert(bfs.createFile(TEST_FILENAME+1));
		safeAssert(bfs.createFile(TEST_FILENAME+2));
		safeAssert(bfs.createFile(TEST_FILENAME+3));
		safeAssert(bfs.createFile(TEST_FILENAME+4));

		Set<BFSFileInfo> ls = bfs.getDirListing("/foo");
		System.out.println(ls);
		safeAssert(ls.size() == 4);
		
	}

    public static void main(String[] args) throws Exception {
        PMetadataTest t = new PMetadataTest();
        t.test1();
    }
}