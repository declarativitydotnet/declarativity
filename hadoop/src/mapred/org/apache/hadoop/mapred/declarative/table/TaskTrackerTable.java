package org.apache.hadoop.mapred.declarative.table;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Map;
import java.util.TreeMap;
import java.util.concurrent.LinkedBlockingQueue;

import org.apache.hadoop.mapred.JobConf;
import org.apache.hadoop.mapred.JobID;
import org.apache.hadoop.mapred.JobTracker;
import org.apache.hadoop.mapred.TaskTrackerStatus;
import org.apache.hadoop.mapred.declarative.Constants;
import org.apache.hadoop.mapred.declarative.Constants.TrackerState;
import org.apache.hadoop.mapred.declarative.master.JobTrackerImpl;
import org.apache.hadoop.net.DNSToSwitchMapping;
import org.apache.hadoop.net.NetworkTopology;
import org.apache.hadoop.net.Node;
import org.apache.hadoop.net.NodeBase;
import org.apache.hadoop.net.ScriptBasedMapping;
import org.apache.hadoop.util.ReflectionUtils;
import org.apache.hadoop.util.StringUtils;

import jol.core.Runtime;
import jol.types.basic.BasicTupleSet;
import jol.types.basic.Tuple;
import jol.types.exception.JolRuntimeException;
import jol.types.exception.UpdateException;
import jol.types.table.BasicTable;
import jol.types.table.Key;
import jol.types.table.ObjectTable;
import jol.types.table.TableName;

public class TaskTrackerTable extends BasicTable {
	
	private static class Resolver extends Thread {
		private Map<String, Node> hostnameToNodeMap;
		private NetworkTopology    clusterMap;
		private DNSToSwitchMapping dnsToSwitchMapping;

		private LinkedBlockingQueue<String> queue = new LinkedBlockingQueue <String>();

		public Resolver(Map<String, Node> hostnameToNodeMap, NetworkTopology clusterMap, JobConf conf) {
			super("DNS Resolver Thread");
			setDaemon(true);
			this.hostnameToNodeMap = hostnameToNodeMap;
			this.clusterMap = clusterMap;
			this.dnsToSwitchMapping =
				(DNSToSwitchMapping)ReflectionUtils.newInstance(
						conf.getClass("topology.node.switch.mapping.impl",
								ScriptBasedMapping.class,
								DNSToSwitchMapping.class),  conf);
		}

		public void resolve(String hostname) {
			while (!queue.add(hostname)) {
				JobTrackerImpl.LOG.warn("Couldn't add to the Resolution queue now. Will try again");
				try {
					Thread.sleep(2000);
				} catch (InterruptedException ie) {}
			}
		}

		@Override
		public void run() {
			while (!isInterrupted()) {
				try {
					List <String> dnHosts = new ArrayList<String>(queue.size());
					// Block if the queue is empty
					dnHosts.add(queue.take());
					queue.drainTo(dnHosts);
					List<String> rName = dnsToSwitchMapping.resolve(dnHosts);
					if (rName == null) {
						JobTrackerImpl.LOG.error("The resolve call returned null! Using " +
								NetworkTopology.DEFAULT_RACK + " for some hosts");
						rName = new ArrayList<String>(dnHosts.size());
						for (int i = 0; i < dnHosts.size(); i++) {
							rName.add(NetworkTopology.DEFAULT_RACK);
						}
					}

					int i = 0;
					for (String m : rName) {
						String host = dnHosts.get(i++);
						String networkLoc = NodeBase.normalize(m);
						addHostToNodeMapping(host, networkLoc);
					}
				} catch (InterruptedException ie) {
					JobTrackerImpl.LOG.warn(getName() + " exiting, got interrupted: " +
							StringUtils.stringifyException(ie));
					return;
				} catch (Throwable t) {
					JobTrackerImpl.LOG.error(getName() + " got an exception: " +
							StringUtils.stringifyException(t));
				}
			}
			JobTrackerImpl.LOG.warn(getName() + " exiting...");
		}

		private Node addHostToNodeMapping(String host, String networkLoc) {
			Node node;
			if ((node = clusterMap.getNode(networkLoc+"/"+host)) == null) {
				node = new NodeBase(host, networkLoc);
				clusterMap.add(node);
				hostnameToNodeMap.put(host, node);
			}
			return node;
		}
		  

	}
	
	/** The table name */
	public static final TableName TABLENAME = new TableName("hadoop", "taskTracker");
	
	/** The primary key */
	public static final Key PRIMARY_KEY = new Key(0,1);
	
	/** An enumeration of all clock table fields. */
	public enum Field{
		JTLOCATION,
		TTLOCATION,
		HOST, 
		HTTP_PORT, 
		STATE,
		FAILURES, 
		MAP_COUNT,
		REDUCE_COUNT,
		MAX_MAP, 
		MAX_REDUCE,
		TIMESTAMP
	};
	
	/** The table schema types. */
	public static final Class[] SCHEMA = {
		String.class,           // JT Location
		String.class,           // TT Location
		String.class,           // TT hostname
		Integer.class,          // Http port
		TrackerState.class,     // State
		Integer.class,          // Failures
		Integer.class,          // map tasks
		Integer.class,          // reduce tasks
		Integer.class,          // max map tasks
		Integer.class,          // max reduce tasks
		Long.class              // Timestamp
	};
	
	private Resolver resolver = null;
	
	private Map<String, Node> hostnameToNodeMap = null;
	
	private NetworkTopology clusterMap = null;
	
	public TaskTrackerTable(Runtime context) {
		super(context, TABLENAME, PRIMARY_KEY, SCHEMA);
	}

	public TaskTrackerTable(Runtime context, JobTrackerImpl tracker) {
		super(context, TABLENAME, PRIMARY_KEY, SCHEMA);
		this.hostnameToNodeMap = Collections.synchronizedMap(new TreeMap<String, Node>());
		this.clusterMap = new NetworkTopology();

		this.resolver = new Resolver(hostnameToNodeMap, clusterMap, tracker.conf());
		tracker.executor().execute(this.resolver);
	}
	
	@Override
	public boolean insert(Tuple t) throws UpdateException {
		TrackerState state = (TrackerState) t.value(Field.STATE.ordinal());
		if (state == Constants.TrackerState.INITIAL) {
			String hostname = (String) t.value(Field.HOST.ordinal());
			this.resolver.resolve(hostname);
		}
		
		return super.insert(t);
	}
	
	
}
