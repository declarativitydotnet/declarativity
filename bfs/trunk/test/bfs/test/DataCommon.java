package bfs.test;

import java.io.File;
import java.io.FileInputStream;
import java.io.BufferedReader;
import java.io.FileReader;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.Set;
import java.util.Random;

import bfs.test.TestCommon;
import bfs.Shell;
import bfs.Conf;

public class DataCommon extends TestCommon {
    String fName;
    String dnList[];

	public void test(String fileName, int repFactor, int masters, int datanodes) throws Exception {
        fName = fileName;

        dnList = new String[datanodes];
        for (int i = 0; i < datanodes; i++) {
            dnList[i] = "td" + (i+1);
        }

        String masterList[] = new String[masters];
        for (int i = 0; i < masters; i++) {
		    masterList[i] = "localhost:" + (5500 + i);
        }

        Conf.setRepFactor(repFactor);
        startMany(masterList);

        startManyDataNodes(dnList);

        shellCreate("foo");
		Shell s = new Shell();

		FileInputStream fis = new FileInputStream(fName);
		appendFile(s, "foo", fis);
		fis.close();
		s.shutdown();
		assertTrue(shellLs("foo"));

	}

    protected void cleanupAll() {
        for (String d : dnList) {
            cleanup(d);
        }
        shutdown();
    }

    private long getChecksum(int i, String name) {
        File csFile = new File("td" + i + "/checksums/" + name + ".cksum");
        System.out.println("File td" + i + "/checksums/" + name);
        safeAssert("checksum file exists", csFile.exists());
        try {
            BufferedReader input = new BufferedReader(new FileReader(csFile));
            String csum = input.readLine();
            return Long.valueOf(csum);
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
    }

    protected void checkFiles() {
        long len = new File(fName).length();
        long appropriateNumberOfChunks = (len / Conf.getChunkSize()) + 1;

        Map<String, Integer> ht = new HashMap<String, Integer>();
        Map<String, Integer> nodecnts = new HashMap<String, Integer>();
        Map<String, Long> csums = new HashMap<String, Long>();

        for (int i = 1; i <= this.datanodes.size(); i++) {
            String str = "td" + i + "/chunks";
            File dir = new File(str);

            if (!dir.exists()) {
                continue;
            }
            System.out.println("open dir "+i);
            for (File f : dir.listFiles()) {
                counter(nodecnts, str);
                counter(ht, f.getName());
                long csum = getChecksum(i, f.getName());
                Long oldCsum = (Long) csums.get(f.getName());
                if (oldCsum == null) {
                    csums.put(f.getName(), new Long(csum));
                } else {
                    System.out.println("csum " + oldCsum + " matches " + csum);
                    safeAssert("checksums match", (oldCsum.longValue() == csum));
                }
            }
        }

        Set<String> set = ht.keySet();
        Iterator<String> it = set.iterator();
        String key;
        long chunks = 0;
        while (it.hasNext()) {
            key = it.next();
            Integer cnt = (Integer) ht.get(key);

            safeAssert("for chunk "+ key + " with a rep factor of " + Conf.getRepFactor() + ", we expected as many replicas, but found "+ cnt, (cnt == Conf.getRepFactor()));
            chunks++;

            System.out.println(key + "\t" + cnt.toString());
        }
        safeAssert("expected " + appropriateNumberOfChunks + " chunks given file size, but found " + chunks, (chunks == appropriateNumberOfChunks));

        set = nodecnts.keySet();
        it = set.iterator();
        while (it.hasNext()) {
            key = it.next();
            Integer cnt = (Integer) nodecnts.get(key);
            System.out.println(key + "\t" + cnt.toString());
        }
    }

	private void counter(Map<String, Integer> m, String s) {
        Integer cnt = m.get(s);

        if (cnt == null)
            cnt = new Integer(1);
        else
            cnt++;

        m.put(s, cnt);
    }

    protected class Victim {
        String directory;
        String chunk;
        int datanodes;

        public Victim(int dn) {
            datanodes = dn;
        }

        public void pickVictim() {
            // pick a random directory
            Random r = new Random();
            System.out.println("this.datanodes = " + this.datanodes);
            if (this.datanodes == 1) {
                pickVictim("td1");
            } else {
                int indx = r.nextInt(this.datanodes-1);
                pickVictim("td" + (indx+1));
            }
        }

        public void pickVictim(String dir) {
            // pick a random chunk
            String longDir = dir + File.separator + "chunks";
            File dirObj = new File(longDir);
            safeAssert("dir exists " + dir, dirObj.exists());
            File[] fileList = dirObj.listFiles();
            Random r = new Random();
            int victim = r.nextInt(fileList.length);
            String thisChunk = fileList[victim].getName();

            pickVictim(longDir, thisChunk);
        }

        public void doVictim() {
            File victim = new File(directory + File.separator + chunk);
            safeAssert("victim file ("+ directory + File.separator + chunk  +") exists?", victim.exists());
            victim.delete();
        }

        public void pickVictim(String dir, String c) {
            directory = dir;
            chunk = c;
        }

        public String getChunk() {
            return chunk;
        }

        public String getDir() {
            return directory;
        }
    }
}
