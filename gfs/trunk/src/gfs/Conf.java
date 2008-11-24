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

    /* NB: The following three arrays must have the same length! */
    private static final String[] dataNodes = new String[] {
        "localhost",
        "localhost",
        "localhost"
    };
    private static final int[] dataNodeControlPorts = new int[] {
        5600,
        5601,
        5602
    };
    private static final int[] dataNodeDataPorts = new int[] {
        5601,
        5603,
        5603
    };

    static {
        assert(masterHosts.length == masterPorts.length);
        assert(dataNodes.length == dataNodeControlPorts.length);
        assert(dataNodes.length == dataNodeDataPorts.length);
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

    public static String getDataNodeAddress(int idx) {
        return "tcp:" + dataNodes[idx] + ":" + dataNodeControlPorts[idx];
    }

    public static int getDataNodeControlPort(int idx) {
        return dataNodeControlPorts[idx];
    }

    public static int getDataNodeDataPort(int idx) {
        return dataNodeDataPorts[idx];
    }

    public static int getNumDataNodes() {
        return dataNodes.length;
    }
}
