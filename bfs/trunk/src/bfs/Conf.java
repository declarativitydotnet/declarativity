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

    private static Host[][] masterNodes = new Host[][] {
    	{
    		new Host("localhost", 5505),
    		//        new Host("localhost", 5506),
    		//        new Host("localhost", 5507),
    	}, {
    		new Host("localhost", 6606),
    	}
    };

    private static Host[] dataNodes = new Host[] {
        new Host("localhost", 5600, 5700),
        new Host("localhost", 5601, 5701),
        new Host("localhost", 5602, 5702),
    };

    private static final long fileOpTimeout = 20000;
    private static final long listingTimeout = 20000;

    private static final int chunkSize = 256 * 1024;

    private static final long heartbeatRetention = 15000L;

    private static long propagationDelay = 15000L;

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
                List<List<String>> masterList = Conf.parseAddressFile(mastersFile);
                Conf.setMasters(masterList);
                System.out.println("Installed new master list: " + masterList);
        	}
        }

        if (System.getenv("SLAVEFILE") != null) {
        	File slavesFile = new File(System.getenv("SLAVEFILE"));
        	if (slavesFile.exists()) {
                List<List<String>> slaveList = Conf.parseAddressFile(slavesFile);
                // don't support partitioning of data nodes; just take first partition from file.
                Conf.setDataNodes(slaveList.get(0));
                System.out.println("Installed new data node list: " + slaveList);
        	}
        }
	}

    public static long getPropagationDelay() {
        return propagationDelay;
    }

    public static void setPropagationDelay(long delay) {
        propagationDelay = delay;
    }

	public static String getLogSink() {
		return logSink;
	}
	public static void setLogSink(String sink) {
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

    public static void setMasters(List<List<String>> partitions) {
    	int i = 0; int part = 0;
    	masterNodes = new Host[partitions.size()][];
    	for(List<String> masters : partitions) {
    		masterNodes[part] = new Host[masters.size()];
        	for (String addr : masters) {
	    		masterNodes[part][i++] = new Host(addr, DEFAULT_MASTER_PORT);
	    	}
	    	part++;
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

    public static void setNewMasterList(List<List<String>> partitions) {
        masterNodes = new Host[partitions.size()][];
        for(int part = 0; part < partitions.size(); part++) {
        	masterNodes[part] = new Host[partitions.get(part).size()];
        	for (int i = 0; i < partitions.get(part).size(); i++) {
                String[] parts = partitions.get(part).get(i).split(":");
                masterNodes[part][i] = new Host(parts[0], Integer.parseInt(parts[1]));
            }
        }
    }

    public static void setNewDataNodeList(int num) {
        dataNodes = new Host[num];
        for (int i = 0; i < num; i++) {
            dataNodes[i] = new Host("localhost", 5600+i, 5700+i);
        }
    }

    public static String getMasterAddress(int part, int idx) {
        Host h = masterNodes[part][idx];
        return "tcp:" + h.name + ":" + h.port;
    }

    public static int getMasterPort(int part, int idx) {
        return masterNodes[part][idx].port;
    }
    public static int getNumPartitions() {
    	return masterNodes.length;
    }
    public static int getNumMasters(int part) {
        return masterNodes[part].length;
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

    public static List<List<String>> parseAddressFile(File inFile) {
    	try {
			BufferedReader br = new BufferedReader(new FileReader(inFile));
			String line;
			List<List<String>> ret = new ArrayList<List<String>>();
			while((line = br.readLine()) != null) {
				List<String> result = new ArrayList<String>();
				ret.add(result);
				boolean done = false;
				do {
					String trimmedLine = line.trim();
					if(trimmedLine.equals("-")) {
						done = true;
					} else {
						if (!trimmedLine.equals(""))
							result.add(trimmedLine);
					}
				} while ((!done) && ((line = br.readLine()) != null));
			}
			return ret;
		} catch (IOException e) {
			throw new RuntimeException(e);
		}
    }

    public static int[] findSelfIndex(boolean isMaster) {
    	Host[][] hostAry;
    	if (isMaster)
    		hostAry = masterNodes;
    	else {
    		hostAry = new Host[1][];
    		hostAry[0] = dataNodes;
    	}
    	try {
			InetAddress localHost = InetAddress.getLocalHost();
			InetAddress[] localAddrs = InetAddress.getAllByName(localHost.getHostName());
			for (InetAddress addr : localAddrs) {
				String hostName = addr.getHostName();
				String ip = addr.getHostAddress();
				for (int i = 0; i < hostAry.length; i++) {
					for (int j = 0; j < hostAry[i].length; j++) {
						if (hostAry[i][j].name.equalsIgnoreCase(hostName) ||
						    hostAry[i][j].name.equals(ip))
							return new int[] {i, j};
					}
				}
			}
			// Failed to find our local address in the given list
			throw new RuntimeException("Cannot find local address!");
		} catch (UnknownHostException e) {
			throw new RuntimeException(e);
		}
    }

    public static String findSelfAddress(boolean isMaster) {
    	Host[][] hostAry;
    	if (isMaster)
    		hostAry = masterNodes;
    	else {
    		hostAry = new Host[1][];
    		hostAry[0] = dataNodes;
    	}
    	try {
			InetAddress localHost = InetAddress.getLocalHost();
			InetAddress[] localAddrs = InetAddress.getAllByName(localHost.getHostName());
			for (InetAddress addr : localAddrs) {
				String hostName = addr.getHostName();
				String ip = addr.getHostAddress();
				for (int i = 0; i < hostAry.length; i++) {
					for (int j = 0; j < hostAry[i].length; j++) {
						if (hostAry[i][j].name.equalsIgnoreCase(hostName) ||
						    hostAry[i][j].name.equals(ip))
							return "tcp:" + hostAry[i][j].name + ":" + hostAry[i][j].auxPort;
					}
				}
			}

			return null;
		} catch (UnknownHostException e) {
			return null;
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
	public static int getPartitionFromString(String truncatedMasterName) {
		int partition =  -1;
        for(int i = 0; i < Conf.getNumPartitions(); i++) {
        	for(int j = 0; j < Conf.getNumMasters(i); j++) {
        		System.out.println(Conf.getMasterAddress(i,j).toString() + " ?= " + truncatedMasterName);
        		if((Conf.getMasterAddress(i,j)).endsWith(truncatedMasterName)) {
        			partition = i; break;
        		}
        	}
        	if(partition != -1) { break; }
        }
        return partition;
    }
}
