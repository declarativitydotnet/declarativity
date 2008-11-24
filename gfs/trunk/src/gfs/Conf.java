package gfs;

public class Conf {
    private static String selfAddr = null;

    private static String[] masterHosts;
    private static int[] masterPorts; 
    private static Boolean seeded = false;

    /* NB: The following two arrays must have the same length! */
    private static final String[] seedMasterHosts = new String[] {
        "localhost",
        "localhost",
        "localhost"
    };
    private static final int[] seedMasterPorts = new int[] {
        5505,
        5506,
        5507
    };

    static {
        assert(masterHosts.length == masterPorts.length);
    }

    /* NB: This must be called before installing "gfs.olg" */
    public static void setSelfAddress(String addr) {
        selfAddr = addr;
    }

    public static String getSelfAddress() {
        if (selfAddr == null)
            throw new IllegalStateException();

        return selfAddr;
    }

    public static void setNewMasterList(String... args) {
        java.lang.System.out.println("yo yo yo\n");
        if (args.length == 0) {
            masterHosts = new String[seedMasterHosts.length];
            masterPorts = new int[seedMasterPorts.length];
            for (int i = 0; i < seedMasterHosts.length; i++) {
                masterHosts[i] = seedMasterHosts[i];
                masterPorts[i] = seedMasterPorts[i];
                java.lang.System.out.println("hit me: "+masterHosts[i]);
                java.lang.System.out.println("hit me: "+masterPorts[i]);
            }
        } else {
            masterHosts = new String[args.length];
            masterPorts = new int[args.length];
            for (int i = 0; i < args.length; i++) {
                String[] parts = args[i].split(":");
                masterHosts[i] = parts[0];
                masterPorts[i] = Integer.valueOf(parts[1]);
                java.lang.System.out.println("hit me: "+masterHosts[i]);
                java.lang.System.out.println("hit me: "+masterPorts[i]);
            }
        }
    }

    public static String getMasterAddress(int idx) {
        
        java.lang.System.out.println("idx: "+idx);
        return "tcp:" + masterHosts[idx] + ":" + masterPorts[idx];
    }

    public static int getMasterPort(int idx) {
        return masterPorts[idx];
    }

    public static int getNumMasters() {
        return masterHosts.length;
    }
}
