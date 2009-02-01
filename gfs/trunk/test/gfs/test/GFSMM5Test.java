package gfs.test;

import java.util.LinkedList;

import jol.types.basic.ValueList;
import gfs.Master;
import gfs.Shell;
import gfs.Conf;
import gfs.test.TestCommon;

import org.junit.Test;

public class GFSMM5Test extends TestCommon {

  private void startOne() throws Exception {
    this.masters = new LinkedList<Master>();
    Master m = new Master(0); // XXX
    m.start();
    this.masters.add(m);
  }

  @Test
  public void test4() {
    try {
      startMany("localhost:5500","localhost:5502","localhost:5503");
      //startMany("localhost:5500");
      //startOne();

      Shell longRun = new Shell();

      long agg = 0;
      int count = 0;

      for (int i=0; i < 100; i++) {
        String file = "XACT"+i;
        long now = java.lang.System.currentTimeMillis();
        createFile(longRun,file);
        agg += (java.lang.System.currentTimeMillis() - now);
        count += 1;
        //Thread.sleep(100);
        //shellCreate(file);
      }
      longRun.shutdown();


      java.lang.System.out.println("OK, good then\n");

      long avg = agg / count;
      java.lang.System.out.println("average time to create metadata on "+ Conf.getNumMasters() + " is "+ avg);

        int cnt = shellLsCnt();
        java.lang.System.out.println("total files in ls: "+cnt);

      assertTrue(shellLs("XACT2","XACT18","XACT16","XACT12","XACT78","XACT59"));
      shutdown();
    } catch (Exception e) {
      java.lang.System.out.println("something went wrong: "+e);
      java.lang.System.exit(1);
    }
  }

  public static void main(String[] args) throws Exception {
    GFSMM5Test t = new GFSMM5Test();
    t.test4();
  }
}
