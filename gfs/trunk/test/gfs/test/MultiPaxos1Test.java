package gfs.test;

import jol.types.exception.JolRuntimeException;
import jol.types.exception.UpdateException;
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
