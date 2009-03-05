package bfs;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.ArrayList;
import java.util.List;

public class Conf {
	public static final int DEFAULT_MASTER_PORT = 8001;
	public static final int DEFAULT_DN_DATA_PORT = 8002;
	public static final int DEFAULT_DN_CONTROL_PORT = 8003;

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
    private static final long listingTimeout = 20000;

    private static final int chunkSize = 256 * 1024;

    private static final long heartbeatRetention = 12000L;

    private static int replicationFactor = 3;

    private static String tapSink = null;
    //private static String tapSink = "tcp:localhost:5678";

    public static final String[] corpus = new String[] {
        "paxos/paxos_global.olg",
        "paxos/paxos_p1.olg",
        "paxos/paxos_p2.olg",
        "paxos/paxos_instance.olg",
        "bfs/bfs_global.olg",
        "bfs/heartbeats.olg",
        "bfs/chunks_global.olg",
        "bfs/chunks.olg",
        "bfs/bfs.olg",
        "bfs/placement.olg",
        "bfs/tap.olg"
    };

	private static String logSink = null;
	

	static {
        if (System.getenv("MASTERFILE") != null) {
        	File mastersFile = new File(System.getenv("MASTERFILE"));
        	if (mastersFile.exists()) {
                List<String> masterList = Conf.parseAddressFile(mastersFile);
                Conf.setMasters(masterList);
                System.out.println("Installed new master list: " + masterList);
        	}
        }

        if (System.getenv("SLAVEFILE") != null) {
        	File slavesFile = new File(System.getenv("SLAVEFILE"));
        	if (slavesFile.exists()) {
                List<String> slaveList = Conf.parseAddressFile(slavesFile);
                Conf.setDataNodes(slaveList);
                System.out.println("Installed new data node list: " + slaveList);
        	}
        }
	}

	public static String getLogSink() {
		return logSink;
	}
	public void setLogSink(String sink) {
		logSink = sink;
	}

	public static void setTap(String t) {
		tapSink = t;
	}
    public static int getChunkSize() {
        return chunkSize;
    }
    public static String getTapSink() {
        return tapSink;
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

    public static long getFileOpTimeout() {
        return fileOpTimeout;
    }

    public static long getListingTimeout() {
        return listingTimeout;
    }

    public static void setMasters(List<String> masters) {
    	masterNodes = new Host[masters.size()];
    	int i = 0;
    	for (String addr : masters) {
    		masterNodes[i++] = new Host(addr, DEFAULT_MASTER_PORT);
    	}
    }

    public static void setDataNodes(List<String> nodeList) {
    	dataNodes = new Host[nodeList.size()];
    	int i = 0;
    	for (String addr : nodeList) {
    		dataNodes[i++] = new Host(addr, DEFAULT_DN_CONTROL_PORT,
    				                  DEFAULT_DN_DATA_PORT);
    	}
    }

    public static void setNewMasterList(String... args) {
        masterNodes = new Host[args.length];
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

    public static List<String> parseAddressFile(File inFile) {
    	try {
			BufferedReader br = new BufferedReader(new FileReader(inFile));
			List<String> result = new ArrayList<String>();
			String line;
			while ((line = br.readLine()) != null) {
				String trimmedLine = line.trim();
				if (!trimmedLine.equals(""))
					result.add(trimmedLine);
			}
			return result;
		} catch (IOException e) {
			throw new RuntimeException(e);
		}
    }

    public static int findSelfIndex(boolean isMaster) {
    	Host[] hostAry;
    	if (isMaster)
    		hostAry = masterNodes;
    	else
    		hostAry = dataNodes;

    	try {
			InetAddress localHost = InetAddress.getLocalHost();
			InetAddress[] localAddrs = InetAddress.getAllByName(localHost.getHostName());
			for (InetAddress addr : localAddrs) {
				String hostName = addr.getHostName();
				String ip = addr.getHostAddress();

				for (int i = 0; i < hostAry.length; i++) {
					System.out.println("Comparing IP " + ip + " and host " +
							           hostName + " against " + hostAry[i].name);
					if (hostAry[i].name.equalsIgnoreCase(hostName) ||
					    hostAry[i].name.equals(ip))
						return i;
				}
			}
			// Failed to find our local address in the given list
			throw new RuntimeException("Cannot find local address!");
		} catch (UnknownHostException e) {
			throw new RuntimeException(e);
		}
    }

    public static String findLocalAddress(int port) {
    	try {
			InetAddress localHost = InetAddress.getLocalHost();
			return "tcp:" + localHost.getHostName() + ":" + port;
		} catch (UnknownHostException e) {
			throw new RuntimeException(e);
		}
    }
}
