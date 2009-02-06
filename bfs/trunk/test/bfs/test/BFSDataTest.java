package bfs.test;

import java.io.File;
import java.io.FileInputStream;
import java.io.InputStream;
import bfs.test.TestCommon;
import bfs.Shell;

import org.junit.Test;

public class BFSDataTest extends TestCommon {

  @Test
    public void test() {
        try {
            startMany("localhost:5500");
            startManyDataNodes("testdata1", "testdata2", "testdata3","testdata4");


            shellCreate("foo");
            java.lang.System.out.println("ready to start shell\n");
            Shell s  = new Shell();

            java.lang.Runtime.getRuntime().exec("dd if=/dev/zero of=./hunk bs=1k count=10000");
            File f = new File("hunk");
            FileInputStream fis = new FileInputStream(f);
            appendFile(s,"foo",(InputStream)fis);
            fis.close();
            s.shutdown();

            new File("hunk").delete();
            assertTrue(shellLs("foo"));

            try {
                shutdown();
            } catch (Exception e) {

            }

            java.lang.System.out.println("now I am just not terminating\n");

        } catch (Exception e) {
            java.lang.System.out.println("something went wrong: "+e);
            java.lang.System.exit(1);
        }
    }


  public static void main(String[] args) throws Exception {
    BFSDataTest t = new BFSDataTest();
    t.test();
  }

}
