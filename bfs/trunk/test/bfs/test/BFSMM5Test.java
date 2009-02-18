package bfs.test;

import bfs.Conf;
import bfs.Shell;
import bfs.test.TestCommon;

import org.junit.Test;

public class BFSMM5Test extends TestCommon {
    @Test(timeout=60000)
    public void test5() throws Exception {
        startMany("localhost:5500", "localhost:5502", "localhost:5503");

        Shell longRun = new Shell();

        long agg = 0;
        int count = 0;

        for (int i = 0; i < 100; i++) {
            String file = "/XACT" + i;
            long now = System.currentTimeMillis();
            createFile(longRun, file);
            agg += (System.currentTimeMillis() - now);
            count += 1;
            // Thread.sleep(100);
            // shellCreate(file);
        }
        longRun.shutdown();

        System.out.println("OK, good then\n");

        long avg = agg / count;
        System.out.println("average time to create metadata on "
                + Conf.getNumMasters() + " is " + avg);

        int cnt = shellLsCnt("/");
        System.out.println("total files in ls: " + cnt);

        assertTrue(shellLs("/", "XACT2", "XACT18", "XACT16", "XACT12", "XACT78", "XACT59"));
        shutdown();
    }

    public static void main(String[] args) throws Exception {
        BFSMM5Test t = new BFSMM5Test();
        t.test5();
    }
}
