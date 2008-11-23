package gfs;

public class Conf {
    private static String selfAddr = null;

    /* NB: The following two arrays must have the same length! */
    private static final String[] masterHosts = new String[] {
        "localhost",
        "localhost",
        "localhost"
    };
    private static final int[] masterPorts = new int[] {
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
