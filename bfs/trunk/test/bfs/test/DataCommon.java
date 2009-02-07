package bfs.test;

import java.io.File;
import java.io.FileInputStream;
import java.util.Hashtable;
import java.util.Iterator;
import java.util.Set;
import java.util.ArrayList;
import bfs.test.TestCommon;
import bfs.Shell;
import bfs.DataNode;
import bfs.Conf;

import org.junit.Test;

import static org.junit.Assert.*;

public class DataCommon extends TestCommon {

    String fName;

	public void test(String fileName, int repFactor, int masters, int datanodes) {
        fName = fileName;
		try {

            //ArrayList<String> masterList = new ArrayList<String>[masters];
            String masterList[] = new String[masters];
            for (int i = 0; i < masters; i++) {
			    masterList[i] = "localhost:"+ (5500 + i);
                System.out.println("master: "+masterList[i]);
            }

            String dnList[] = new String[datanodes];
            for (int i = 0; i < datanodes; i++) {
                dnList[i] = "td" + (i+1);
            }

            Conf.setRepFactor(repFactor);
            startMany(masterList);
            startManyDataNodes(dnList);
			System.out.println("done with datanodes\n");

			shellCreate("foo");
			System.out.println("ready to start shell\n");
			Shell s = new Shell();

			FileInputStream fis = new FileInputStream(fName);
			appendFile(s, "foo", fis);
			fis.close();
			s.shutdown();

            assertTrue(shellLs("foo"));

            check_files();
            new File("hunk").delete();

            shutdown();

            java.lang.System.out.println("now I am just not terminating\n");

        } catch (Exception e) {
            java.lang.System.out.println("something went wrong: "+e);
            java.lang.System.exit(1);
        }
    }

    private void check_files() {
        long len = new File(fName).length();
        long appropriateNumberOfChunks = (len / Conf.getChunkSize()) + 1;

        Hashtable ht = new Hashtable();
        Hashtable nodecnts = new Hashtable();

        System.out.println("datanodes.size = " + this.datanodes.size());

        for (int i=1; i <= this.datanodes.size(); i++) {
            System.out.println("-- i is "+i);
            String str = "td" + i + "/chunks";
            File dir = new File(str);
            
            System.out.println("umm" + str );
            for (File f : dir.listFiles()) {
                System.out.println("try "+f.toString());
                counter(nodecnts, str);
                counter(ht, f.getName());                
            }
            System.out.println("yo son\n");
        }

        Set<String> set = ht.keySet();
        Iterator<String> it = set.iterator();
        String key;
        long chunks = 0;
        while (it.hasNext()) {
            key = it.next();
            Integer cnt = (Integer) ht.get(key);

            assertTrue(cnt == Conf.getRepFactor());
            chunks++;

            System.out.println(key+"\t"+cnt.toString());
        }
        //assertTrue( chunks == appropriateNumberOfChunks );
        System.out.println("anc = "+appropriateNumberOfChunks);
        assertEquals(chunks, appropriateNumberOfChunks);

        set = nodecnts.keySet();
        it = set.iterator();
        while (it.hasNext()) {
            key = it.next();
            Integer cnt = (Integer) nodecnts.get(key);
            System.out.println(key+"\t"+cnt.toString());
        }
     

        
    }
    private void counter(Hashtable h, String s) {
        Integer cnt = (Integer) h.get((Object)s);
        if (cnt == null) {
            cnt = new Integer(1);
        } else {
            cnt += 1;
        }
        h.put(s, cnt); 
    }


}
