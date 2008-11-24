package gfs;

public class Conf {
    private static String selfAddr = null;

    /* NB: The following two arrays must have the same length! */
    private static String[] masterHosts = new String[] {
        "localhost",
        "localhost",
        "localhost"
    };
    private static int[] masterPorts = new int[] {
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
        masterHosts = new String[args.length];
        masterPorts = new int[args.length];
        for (int i = 0; i < args.length; i++) {
            String[] parts = args[i].split(":");
            masterHosts[i] = parts[0];
            masterPorts[i] = Integer.parseInt(parts[1]);
        }
    }

    public static String getMasterAddress(int idx) {
        return "tcp:" + masterHosts[idx] + ":" + masterPorts[idx];
    }

    public static int getMasterPort(int idx) {
        return masterPorts[idx];
    }

    public static int getNumMasters() {
        return masterHosts.length;
    }
}
