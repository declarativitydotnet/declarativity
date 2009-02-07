package bfs.test;

import java.io.File;
import java.io.FileInputStream;
import java.util.Hashtable;
import java.util.Iterator;
import java.util.Set;
import java.util.ArrayList;
import bfs.test.TestCommon;
import bfs.Shell;
import bfs.DataNode;
import bfs.Conf;

import org.junit.Test;

import static org.junit.Assert.*;

public class BFSData1Test extends DataCommon {

    private static String testFile = "/usr/share/dict/words";

	@Test
    public void test1() {
        test(testFile, 1, 1, 1);
    }


	public static void main(String[] args) throws Exception {
		BFSData1Test t = new BFSData1Test();
		t.test1();
	}
}
