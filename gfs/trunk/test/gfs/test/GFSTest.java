package gfs.test;

import java.util.List;
import java.util.LinkedList;
import jol.types.basic.ValueList;
import junit.framework.Assert;
import gfs.Master;
import gfs.Shell;

import org.junit.Test;

public class GFSTest {

  @Test
  public void test1() {
    Shell shell;
    Master master;

    try { 
      
      master = new Master();
      shell = new Shell();

      master.start();    

      List<String> argList = new LinkedList<String>();
      argList.add("foo");
      shell.doCreateFile(argList,false);
      //shell.shutdown();

      //shell = new Shell();
      argList.clear();
      ValueList<String> list = shell.doListFiles(argList);
      boolean good = false;
      for (String item : list) {
        java.lang.System.out.println("got: "+item);
        if (item.compareTo("foo") == 0) {
          good = true;
        }
      }

      master.stop();
      shell.shutdown();
      Assert.assertTrue(good);

    } catch (Exception e) {
      java.lang.System.out.println("something went wrong: "+e);
      java.lang.System.exit(1);
    }
  }




  public static void main(String[] args) throws Exception {
    GFSTest t = new GFSTest();
    t.test1();
  }

}
