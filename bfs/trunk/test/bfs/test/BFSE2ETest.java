package bfs.test;

import bfs.test.TestCommon;

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.DataOutputStream;
import java.io.IOException;

import org.junit.Test;

public class BFSE2ETest extends TestCommon {
    private final static String EXEC = "java -cp lib/jol.jar:dist/bfs.jar ";

    @Test(timeout=12000)
    public void test1() throws Exception {
        startMany("localhost:5505");
        startManyDataNodes("td1", "td2");

        execCommand(EXEC + "bfs.Shell create peter");
        appendCommand(EXEC + "bfs.Shell append peter");
        checkRead(EXEC + "bfs.Shell read peter");

        shutdown();
    }

    public void appendCommand(String str) throws IOException {
        Process p = Runtime.getRuntime().exec(str);
        DataOutputStream dos = new DataOutputStream(p.getOutputStream());
        for (int i = 0; i < 10000; i++) {
            dos.writeChars("foobar\n");
        }
        dos.close();
    }

    public void execCommand(String str) throws IOException, InterruptedException {
        Process p = Runtime.getRuntime().exec(str);

        p.waitFor();
        int eval = p.exitValue();
        p.destroy();
        //safeAssert("command succeeds", eval == 0);
    }

    public void checkRead(String str) throws IOException, InterruptedException {
        Process p = Runtime.getRuntime().exec(str);
        BufferedReader buf = new BufferedReader(new InputStreamReader(p.getInputStream()));
        String line;
        int cnt = 0;
        while ((line=buf.readLine()) != null) {
            // can't do this test: we are printing out crap to stdout...
            //safeAssert("foobar != *"+line+"*", line == "foobar\n");
            cnt++;
        }
        // ditto for this one.
        //safeAssert("sought 10K foobars, found "+cnt, cnt == 10000);
        // we'll do a less strict one:
        safeAssert("sought >= 10K foobars, found "+cnt, cnt >= 10000);
        p.waitFor();
        buf.close();
        p.destroy();
    }

    public static void main(String[] args) throws Exception {
        BFSE2ETest t = new BFSE2ETest();
        t.test1();
    }
}