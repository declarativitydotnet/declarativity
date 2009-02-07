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

public class BFSData3Test extends DataCommon {

    private static String testFile = "/usr/share/dict/words";

	@Test
    public void test1() {
        test(testFile, 4, 1, 13);
    }


	public static void main(String[] args) throws Exception {
		BFSData3Test t = new BFSData3Test();
		t.test1();
	}
}
