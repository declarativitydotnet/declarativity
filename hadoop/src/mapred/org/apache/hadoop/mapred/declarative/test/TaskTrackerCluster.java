package org.apache.hadoop.mapred.declarative.test;

import java.io.File;
import java.io.IOException;
import java.io.OutputStream;
import java.util.HashSet;
import java.util.Set;
import java.util.concurrent.Executor;
import java.util.concurrent.Executors;

import org.apache.hadoop.mapred.JobConf;

public class TaskTrackerCluster extends Thread {
	
	private Executor executor;
	
	private Set<TaskTracker> trackers;
	
	private OutputStream utilityLog;
	private OutputStream callLog;
	
	public TaskTrackerCluster(JobConf conf, int size) throws IOException {
		this.executor = Executors.newCachedThreadPool();
		this.trackers = new HashSet<TaskTracker>();
		for (int i = 0; i < size; i++) {
			this.trackers.add(create(conf, "tracker" + i));
		}
		this.utilityLog = null;
		this.callLog = null;
	}
	
	public TaskTrackerCluster(JobConf conf, int size, OutputStream utilityLog, OutputStream callLog) throws IOException {
		this(conf, size);
		this.utilityLog = utilityLog;
		this.callLog = callLog;
	}
	
	public float slotUtility() {
		float utility = 0f;
		for (TaskTracker t : trackers) {
			utility += t.slotUtility();
		}
		return utility / (float) trackers.size();
	}
	
	public long heartbeatCallDuration() {
		long duration = 0L;
		for (TaskTracker t : trackers) {
			duration += t.heartbeatCallDuration();
		}
		return duration / (long) trackers.size();
	}
	
	public void run() {
		Long time = 0L;
		try {
			while (!isInterrupted()) {
				sleep(1000);
				Float su = slotUtility();
				Long  cd = heartbeatCallDuration();
				if (this.utilityLog != null && this.callLog != null) {
					this.utilityLog.write((time.toString() + ", " + su.toString() + "\n").getBytes());
					this.callLog.write((time.toString() + ", " + cd.toString() + "\n").getBytes());
				}
				else {
					System.err.println("TIMESTEP " + time++);
					System.err.println("SLOT UTILITY " + su);
					System.err.println("HEARTBEAT DURATION " + cd);
				}
			}
		} catch (InterruptedException e) {
		} catch (IOException e) {
			e.printStackTrace();
		}
	}
	
	@Override
	public void interrupt() {
		for (TaskTracker t : trackers) {
			t.interrupt();
		}
	}
	
	private TaskTracker create(JobConf conf, String name) throws IOException {
		TaskTracker tracker = new TaskTracker(conf, name, this.executor);
		tracker.start();
		return tracker;
	}
}
