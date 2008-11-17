package gfs.test;

import java.net.URL;
import java.util.concurrent.SynchronousQueue;

import jol.core.Runtime;
import jol.core.System;
import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.basic.TypeList;
import jol.types.exception.JolRuntimeException;
import jol.types.exception.UpdateException;
import jol.types.table.Key;
import jol.types.table.ObjectTable;
import jol.types.table.Table;

import jol.types.table.EventTable;
import jol.types.table.TableName;
import jol.types.table.Table.Callback;
import junit.framework.Assert;
//import gfs.test.Assert;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

public class MultiPaxos1Test extends PaxosCommon {

    @Test
        public void test1() throws UpdateException, InterruptedException,JolRuntimeException {
          //MultiPaxosTest t = new MultiPaxosTest();
          setup(5000,5002,5003);
          simpleMultiPaxosTest();
          shutdown();
        }
      
      @Test
        public void test2() throws UpdateException, InterruptedException,JolRuntimeException {
          /* in this test, we kill one of the (non-master) servers and confirm that we can still move forward */
          //MultiPaxosTest t = new MultiPaxosTest();
          setup(5000,5002,5003);
          systems[2].shutdown();
          simpleMultiPaxosTest();
          shutdown();
        }

      //@Test
        public void test3() throws UpdateException, InterruptedException,JolRuntimeException {
          /* in this test, we kill one of the (non-master) servers and confirm that we can still move forward */
          //MultiPaxosTest t = new MultiPaxosTest();
          setup(5000,5002);
          complexMultiPaxosTest();
          shutdown();
        }




    public static void main(String[] args) throws Exception {
        MultiPaxos1Test t = new MultiPaxos1Test();
        t.test1();
        t.test2();
        //t.test3();
    }
}
