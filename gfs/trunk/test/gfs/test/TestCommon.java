package gfs.test;

import java.util.List;
import java.util.LinkedList;
import jol.types.basic.ValueList;
import jol.types.exception.JolRuntimeException;
import jol.types.exception.UpdateException;
import junit.framework.Assert;
import gfs.Master;
import gfs.Shell;

import org.junit.After;
import org.junit.Before;

public class TestCommon {
  //private Master[] masters;
  protected ValueList<Master> masters;
  protected Shell shell;

  @Before
  public void doNothing() {}
  
  @After
  public void shutdown() {
    for (Master m : masters) {
      m.stop();
    }
    //shell.shutdown();
  }

  protected Boolean shellLs(String... list) throws JolRuntimeException,UpdateException,InterruptedException {
    Shell shell = new Shell();
    Boolean ret = findInLs(shell,list);
    shell.shutdown();
    return ret;
  }

  protected void shellCreate(String name) throws JolRuntimeException,UpdateException,InterruptedException {
    Shell shell = new Shell();
    createFile(shell,name);
    shell.shutdown();
  }

  protected void assertTrue(Boolean b) {
    /* weird huh?  Assert.assertTrue raises an error without calling the @After method,
       so we never terminate.
    */
    if (!b) {
      shutdown();
    }
    Assert.assertTrue(b);
  }

  protected Boolean findInLs(Shell shell,String... files) throws JolRuntimeException,UpdateException,InterruptedException {
    ValueList<String> list = lsFile(shell);

    // obviously not an efficient way to do this.
    for (String item : files) {
        if (!list.contains(item)) {
          return false;
        } else {
          java.lang.System.out.println("found "+item);
        }
    }
    return true;
  }

  protected void stopMany() {
    for (Master sys : this.masters) {
      sys.stop();
    }
  } 
  protected void startMany(String... args) throws JolRuntimeException,UpdateException,InterruptedException {
    this.masters = new ValueList<Master>(); 
    for (int i=0, s=0; i < args.length; i++,s=0) {
      String[] sig = new String[args.length];
      sig[s++] = args[i];
      for (int j=0; j < args.length; j++) {
        java.lang.System.out.println("J = "+j);
        if (j != i) {
          sig[s++] = args[j];
        }
      } 
      Master m = new Master(sig);
      m.start();
      this.masters.add(m);
    }
  }

  protected ValueList<String> lsFile(Shell shell) throws UpdateException,InterruptedException,JolRuntimeException {
      List<String> argList = new LinkedList<String>();
      return shell.doListFiles(argList);
  }
  protected void createFile(Shell shell,String name) throws UpdateException,InterruptedException,JolRuntimeException  {
      List<String> argList = new LinkedList<String>();
      argList.add(name);
      shell.doCreateFile(argList,false);
  }

/*
  public static void main(String[] args) throws Exception {
    TestC t = new GFSMMTest();
    t.test1();
    t.test2();
    t.test3();
  }
*/
}
