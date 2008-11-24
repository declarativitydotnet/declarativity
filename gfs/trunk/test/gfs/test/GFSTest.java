package gfs.test;

import java.util.List;
import java.util.LinkedList;
import jol.types.basic.ValueList;
import junit.framework.Assert;
import gfs.Master;
import gfs.Shell;

import org.junit.Test;

public class GFSTest extends TestCommon{

  @Test
  public void test1() {
    Shell shell;
    Master master;

    try { 
        startMany("localhost:5505");
        shellCreate("foo");
        shellLs("foo");
        stopMany();
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
