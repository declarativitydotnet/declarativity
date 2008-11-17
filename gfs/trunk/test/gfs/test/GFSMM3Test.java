package gfs.test;

import java.net.URL;
import java.util.concurrent.SynchronousQueue;
import java.util.List;
import java.util.LinkedList;
import java.util.Arrays;

import jol.core.Runtime;
import jol.core.System;
import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.basic.TypeList;
import jol.types.basic.ValueList;
import jol.types.exception.JolRuntimeException;
import jol.types.exception.UpdateException;
import jol.types.table.Key;
import jol.types.table.ObjectTable;
import jol.types.table.Table;

import jol.types.table.EventTable;
import jol.types.table.TableName;
import jol.types.table.Table.Callback;
import junit.framework.Assert;
import gfs.Master;
import gfs.Shell;

import gfs.test.TestCommon;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

public class GFSMM3Test extends TestCommon {

  @Test
  public void test3() {
    try { 
      startMany("5500","5502","5503");
      
      shellCreate("foo");
      /* this time, kill the primary */
      this.masters.get(0).stop();

      /* these ops should timeout to the secondary but eventually work */
      shellCreate("bar");
      shellCreate("bas");

      assertTrue(shellLs("foo","bar","bas"));
  
      java.lang.System.out.println("OK, good then\n");
      shutdown();

    } catch (Exception e) {
      java.lang.System.out.println("something went wrong: "+e);
      java.lang.System.exit(1);
    }
  }


  public static void main(String[] args) throws Exception {
    GFSMM3Test t = new GFSMM3Test();
    t.test3();
  }

}
