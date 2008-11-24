package gfs.test;

import jol.types.basic.ValueList;
import gfs.Master;
import gfs.test.TestCommon;

import org.junit.Test;

public class GFSMM5Test extends TestCommon {
  
  private void startOne() throws Exception {
    this.masters = new ValueList<Master>();
    Master m = new Master(0); // XXX
    m.start();
    this.masters.add(m);
  }

  @Test
  public void test4() {
    try { 
      startMany("5500","5502","5503");
      //startOne();
    
      //Shell longRun = new Shell();

      for (int i=0; i < 500; i++) {
        String file = "XACT"+i;
        //createFile(longRun,file);
        shellCreate(file);
      }      

      //longRun.shutdown();

      assertTrue(shellLs("XACT42","XACT18","XACT36","XACT12"));
  
      java.lang.System.out.println("OK, good then\n");
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
