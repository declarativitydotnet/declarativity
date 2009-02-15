package bfs.test;

import java.util.LinkedList;

import bfs.Conf;
import bfs.Master;
import bfs.Shell;
import bfs.test.TestCommon;

import org.junit.Test;

public class BFSMM5Test extends TestCommon {

    private void startOne() throws Exception {
        this.masters = new LinkedList<Master>();
        Master m = new Master(0); // XXX
        m.start();
        this.masters.add(m);
    }

    @Test(timeout=60000)
    public void test5() {
        try {
            startMany("localhost:5500", "localhost:5502", "localhost:5503");
            // startMany("localhost:5500");
            // startOne();

            Shell longRun = new Shell();

            long agg = 0;
            int count = 0;

            for (int i = 0; i < 100; i++) {
                String file = "XACT" + i;
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

            int cnt = shellLsCnt();
            System.out.println("total files in ls: " + cnt);

            assertTrue(shellLs("XACT2", "XACT18", "XACT16", "XACT12", "XACT78", "XACT59"));
            shutdown();
        } catch (Exception e) {
            System.out.println("something went wrong: " + e);
            e.printStackTrace();
            System.exit(1);
        }
    }

    public static void main(String[] args) throws Exception {
        BFSMM5Test t = new BFSMM5Test();
        t.test5();
    }
}
