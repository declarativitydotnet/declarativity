package bfs;

public class Conf {
    private static String selfAddr = null;

    private static class Host {
        public final String name;
        public final int port;
        public final int auxPort;

        Host(String name, int port, int auxPort) {
            this.name = name;
            this.port = port;
            this.auxPort = auxPort;
        }

        Host(String name, int port) {
            this(name, port, -1);
        }
    }

    private static Host[] masterNodes = new Host[] {
        new Host("localhost", 5505),
//        new Host("localhost", 5506),
//        new Host("localhost", 5507),
    };

    private static Host[] dataNodes = new Host[] {
        new Host("localhost", 5600, 5700),
        new Host("localhost", 5601, 5701),
        new Host("localhost", 5602, 5702),
    };

    private static final long fileOpTimeout = 20000;
    private static final long listingTimeout = 5000;

    private static final int chunkSize = 102400;

    private static final long heartbeatRetention = 4000L;

    private static int replicationFactor = 1;

    public static int getChunkSize() {
        return chunkSize;
    }
    public static long getHeartbeatRetention() {
        return heartbeatRetention;
    }

    public static int getRepFactor() {
        return replicationFactor;
    }

    public static void setRepFactor(int rf) {
        replicationFactor = rf;
    }

    /* NB: This must be called before installing "bfs.olg" */
    public static void setSelfAddress(String addr) {
        selfAddr = addr;
    }

    public static String getSelfAddress() {
        // XXX: null check disabled because of EC2 for now
        //if (selfAddress == null)
        //    throw new IllegalStateException();
        return selfAddr;
    }

    public static long getFileOpTimeout() {
        return fileOpTimeout;
    }

    public static long getListingTimeout() {
        return listingTimeout;
    }

    public static void setNewMasterList(String... args) {
        masterNodes = new Host[args.length];
        System.out.println("yo yo\n");
        for (int i = 0; i < args.length; i++) {
            String[] parts = args[i].split(":");
            masterNodes[i] = new Host(parts[0], Integer.parseInt(parts[1]));
        }
    }

    public static void setNewDataNodeList(int num) {
        dataNodes = new Host[num];
        for (int i = 0; i < num; i++) {
            dataNodes[i] = new Host("localhost", 5600+i, 5700+i);
        }
    }

    public static String getMasterAddress(int idx) {
        Host h = masterNodes[idx];
        return "tcp:" + h.name + ":" + h.port;
    }

    public static int getMasterPort(int idx) {
        return masterNodes[idx].port;
    }

    public static int getNumMasters() {
        return masterNodes.length;
    }

    public static String getDataNodeAddress(int idx) {
        Host dn = dataNodes[idx];
        return "tcp:" + dn.name + ":" + dn.port;
    }

    public static int getDataNodeControlPort(int idx) {
        return dataNodes[idx].port;
    }

    public static int getDataNodeDataPort(int idx) {
        return dataNodes[idx].auxPort;
    }

    public static int findDataNodeDataPort(String host, int controlPort) {
        for (Host h : dataNodes) {
            if (h.name.equals(host) && h.port == controlPort)
                return h.auxPort;
        }

        throw new IllegalArgumentException("No such data node: " +
                                           host + ":" + controlPort);
    }

    public static int getNumDataNodes() {
        return dataNodes.length;
    }
}
