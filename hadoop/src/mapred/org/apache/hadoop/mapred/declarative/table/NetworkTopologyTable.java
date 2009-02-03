package org.apache.hadoop.mapred.declarative.table;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.LinkedBlockingQueue;

import org.apache.hadoop.mapred.JobConf;
import org.apache.hadoop.mapred.JobTracker;
import org.apache.hadoop.mapred.TaskTrackerStatus;
import org.apache.hadoop.mapred.declarative.JobTrackerImpl;
import org.apache.hadoop.net.DNSToSwitchMapping;
import org.apache.hadoop.net.NetworkTopology;
import org.apache.hadoop.net.Node;
import org.apache.hadoop.net.NodeBase;
import org.apache.hadoop.net.ScriptBasedMapping;
import org.apache.hadoop.util.ReflectionUtils;
import org.apache.hadoop.util.StringUtils;

import jol.core.Runtime;
import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.exception.UpdateException;
import jol.types.table.Key;
import jol.types.table.ObjectTable;
import jol.types.table.TableName;

public class NetworkTopologyTable extends ObjectTable {

	private class Resolver extends Thread {
		private jol.core.JolSystem context;
		private DNSToSwitchMapping dnsToSwitchMapping;

		private LinkedBlockingQueue<TaskTrackerStatus> queue =
			new LinkedBlockingQueue <TaskTrackerStatus>();

		public Resolver(jol.core.JolSystem context, JobConf conf) {
			super("DNS Resolver Thread");
			setDaemon(true);
			this.context = context;
			this.dnsToSwitchMapping =
				(DNSToSwitchMapping)ReflectionUtils.newInstance(
						conf.getClass("topology.node.switch.mapping.impl",
								ScriptBasedMapping.class,
								DNSToSwitchMapping.class),  conf);
		}

		public void resolve(TaskTrackerStatus t) {
			while (!queue.add(t)) {
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
					List <TaskTrackerStatus> statuses =
						new ArrayList<TaskTrackerStatus>(queue.size());
					// Block if the queue is empty
					statuses.add(queue.take());
					queue.drainTo(statuses);
					List<String> dnHosts = new ArrayList<String>(statuses.size());
					for (TaskTrackerStatus t : statuses) {
						dnHosts.add(t.getHost());
					}
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
						String host = statuses.get(i++).getHost();
						String networkLoc = NodeBase.normalize(m);
						register(host, networkLoc);
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

		private void register(String host, String networkLoc) throws UpdateException {
			TupleSet nodes = new TupleSet(NetworkTopologyTable.TABLENAME);
			Node     node  = new NodeBase(host, networkLoc);
			while (node != null) {
				nodes.add(NetworkTopologyTable.node(node));
				node = node.getParent();
			}
			this.context.schedule(JobTracker.PROGRAM,
					              NetworkTopologyTable.TABLENAME,
					              nodes, null);
		}
	}

	/** The table name */
	public static final TableName TABLENAME = new TableName(JobTracker.PROGRAM, "networkTopology");

	/** The primary key */
	public static final Key PRIMARY_KEY = new Key(0);

	/** An enumeration of all clock table fields. */
	public enum Field{NAME, LOCATION, PARENT, LEVEL};

	/** The table schema types. */
	public static final Class[] SCHEMA = {
		String.class,   // Name
		String.class,   // Location
		String.class,   // Parent name (FK)
		Integer.class   // Node level
	};

	private Resolver resolver;

	public NetworkTopologyTable(Runtime context, JobTrackerImpl tracker) {
		super(context, TABLENAME, PRIMARY_KEY, SCHEMA);
		this.resolver = new Resolver(context, tracker.conf());
		tracker.executor().execute(this.resolver);
	}

	/**
	 * Resolves the tracker network topology location, scheduling
	 * the resolution in the jol.core.Runtime context.
	 * NOTE: non-blocking call.
	 * @param tracker The tracker status.
	 */
	public void resolve(TaskTrackerStatus tracker) {
		this.resolver.resolve(tracker);
	}

	private static Tuple node(Node node) {
		String parent = node.getParent() == null ? null :
							node.getParent().getName();
		return new Tuple(node.getName(),
				         node.getNetworkLocation(),
				         parent, node.getLevel());
	}
}
