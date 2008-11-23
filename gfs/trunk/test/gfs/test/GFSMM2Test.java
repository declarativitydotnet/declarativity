package gfs.test;

import gfs.Shell;

import gfs.test.TestCommon;

import org.junit.Test;

public class GFSMM2Test extends TestCommon {

  @Test
  public void test2() {
    try { 
      startMany("5500","5502","5503");
      
      shell = new Shell();
      createFile(shell,"foo");

      /* kill one of the masters */
      this.masters.get(1).stop();

      createFile(shell,"bar");

      assertTrue(findInLs(shell,"foo","bar"));

      shutdown();

    } catch (Exception e) {
      java.lang.System.out.println("something went wrong: "+e);
      java.lang.System.exit(1);
    }
  }


  public static void main(String[] args) throws Exception {
    GFSMM2Test t = new GFSMM2Test();
    t.test2();
  }

}
