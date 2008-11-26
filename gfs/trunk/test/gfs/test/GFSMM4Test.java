package gfs.test;

import gfs.test.TestCommon;

import org.junit.Test;

public class GFSMM4Test extends TestCommon {

  @Test
  public void test2() {
    try { 
      startMany("localhost:5500","localhost:5502","localhost:5503");
      
      //shell = new Shell();
      //createFile(shell,"foo");

      /* kill one of the masters */
      this.masters.get(1).stop();

     // shell = new Shell();
     // createFile(shell,"foo");
     // createFile(shell,"bar");

        shellCreate("foo");
        shellCreate("bar");


      assertTrue(shellLs("foo","bar"));
    
     //   shellRm("foo");
        
        ///assertTrue(!shellLs("foo"));

      shutdown();

    } catch (Exception e) {
      java.lang.System.out.println("something went wrong: "+e);
      java.lang.System.exit(1);
    }
  }


  public static void main(String[] args) throws Exception {
    GFSMM4Test t = new GFSMM4Test();
    t.test2();
  }

}
